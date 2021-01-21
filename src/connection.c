

#include "nose.h"
#include "connection.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

/*
* Make a connection to remote server
*/

int 
make_socket(char *dst_addr, int port)
{
      int fd;
      struct sockaddr_in sr; 
      if ((fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
      {
            perror("[ERROR] Socket create failed");
            return (FAILED);
      }

      memset(&sr, 0, sizeof(struct sockaddr_in));

      sr.sin_family = AF_INET;
      sr.sin_port = htons(port);
      sr.sin_addr.s_addr = inet_addr(dst_addr);

      
      if (connect(fd, (struct sockaddr *)(&sr), sizeof(struct sockaddr)) == -1){
            perror("[Error] Connect to remote server failed");
            return (FAILED);
      }
      return fd;
}

static int 
make_raw_socket(char *dst_addr, int protocol)
{
      int fd;
      // int on = 1;
      if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_GRE)) <= 0)
      {
            perror("[Error] Create a new socket failed");
            return (ERROR);
      }
      // int len = sizeof(on);
      // if ((fd = setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &on, len)) <= 0)
      // {
      //       perror("[Error] Turn on self-defining IP datagram failed");
      //       return (ERROR);
      // }

      

      return (fd);
      
}

static void 
do_PPTP_connection(int connection_fd, struct vpn_peer *peer)
{
      int state = S_SCCRQ;
      char buffer[BUFSIZ];
      int len;


      while (1)
      {
            switch (state)
            {
            case S_SCCRQ:
                  // Send the Start-Control-Connection-Request
                  printf("Send the Start-Control-Connection-Request\n");
                  len = fill_SCCRq_packet(buffer, BUFSIZ, NULL, NULL);
                  state = send_packet(buffer, connection_fd, len);
                  
                  if (state == OK) state = S_SCCRP; // Waiting for Start-Control-Connection-Reply 
                  else state = ERROR_STATE;
                  break;
            
            case S_SCCRP:
                  //TODO Check the packet if it is valid.
                  len = read_buffer(buffer, connection_fd, BUFSIZ);
                  state = S_OCRQ; //--> Outgoing Control Request
                  break;
            case S_OCRQ:
                  printf("Send the Outgoing-Control-Request\n");
                  len = fill_OCRq_packet(buffer, BUFSIZ, NULL, NULL);
                  state = send_packet(buffer, connection_fd, len);
                  if (state == OK) state = S_OCRP; // --> Outgoing Control Reply
                  else state = ERROR_STATE;
                  break;
            case S_OCRP:
                  len = read_buffer(buffer, connection_fd, BUFSIZ);
                  state = clip_OCRp_packet(buffer, peer);
                  if (state == OK) state = S_SLI; // --> Set link info
                  else state = ERROR_STATE;
                  break;
            case S_SLI:
                  len = fill_SLI_packet(buffer, BUFSIZ, peer->peer_id);
                  state = send_packet(buffer, connection_fd, len);
                  if (state == OK) state = LIVING_LOOP;
                  break;
            default:
                  break;
            }

            if(state == ERROR_STATE) {
                  printf("Error occured\n");
                  break; 
            }

            //TODO Go on to check the living of tunneling
            if(state == LIVING_LOOP) break;
      }
      
}

static void 
do_PPP_connection(int fd, struct vpn_peer *peer)
{
      //Filling the enhanced GRE header
      struct EGRE_header *egre_header = filling_EGRE_header(sizeof(struct PPP) + sizeof(struct PPP_LCP) + 5, peer->peer_id);
      //Filling the PPP content 
      struct PPP *ppp = filling_PPP(LCP);  

      struct PPP_LCP *ppp_lcp = filling_PPP_LCP(CONF_ACK, 0x04, sizeof(struct PPP_LCP) + 5);

      char buffer[BUFSIZ];
      memset(buffer, 0, BUFSIZ); 
      memcpy(buffer, (char *)egre_header, sizeof(struct EGRE_header));
      memcpy(buffer + sizeof(struct EGRE_header), 
            (char *)ppp, sizeof(struct PPP));
      memcpy(buffer + sizeof(struct EGRE_header) + sizeof(struct PPP),
            (char* )ppp_lcp, sizeof(struct PPP_LCP));

      for (int i = 0; i < sizeof(struct EGRE_header) + sizeof(struct PPP) + sizeof(struct PPP_LCP); i++)
      {
            printf("%02x ", buffer[i]);
      }
      printf("\n");
      char ppp_chars[sizeof(struct PPP)];
      memcpy(ppp_chars, (char *)ppp, sizeof(struct PPP));
      for(int i = 0; i < sizeof(struct PPP); i++)
      {
            printf("%02x ", ppp_chars[i]);
      }
      printf("\n");

      char options[] = {0x03, 0x05, 0xc2, 0x23, 0x05};
      memcpy(buffer + sizeof(struct EGRE_header) + sizeof(struct PPP) + sizeof(struct PPP_LCP),
            options, 5);

      struct sockaddr_in send;
      send.sin_family = AF_INET;
      send.sin_port = 0;
      send.sin_addr.s_addr = inet_addr("172.16.15.129");
      
      if (sendto(fd, buffer,  
                                                + sizeof(struct EGRE_header)
                                                + sizeof(struct PPP)
                                                + sizeof(struct PPP_LCP) + 5, 0, (struct sockaddr *)&send, sizeof(send)) < 0)
      {
            perror("[ERROR] Send the PPP packet failed");
      }
}
static int 
fill_SCCRq_packet(char * buffer, int len, char *host_name, char *vendor_name)
{
      memset(buffer, 0, len);
      struct PPTP_SCCRq * sccrq = (struct PPTP_SCCRq *)malloc(sizeof(struct PPTP_SCCRq));

      if (sccrq == NULL) return (FAILED);

      
      sccrq->length = htons(sizeof(struct PPTP_SCCRq));
      sccrq->message_type = htons(1);
      sccrq->magic_cookie = htonl(MAGIC_COOKIE);
      sccrq->ctrl_message_type = htons(SCCRQ);
      sccrq->reserved0 = 0;
      sccrq->protocol_version = htons(PPTP_VER);
      sccrq->reserved1 = 0;
      sccrq->framing_cap = htonl(1);
      sccrq->bearer_cap = htonl(1);
      sccrq->max_channels = 0;
      sccrq->firmware_revision = 0;
      
      memset(sccrq->host_name, 0, 64);
      memset(sccrq->vender_name, 0, 64);

      if (host_name) strcpy(sccrq->host_name, host_name);
      if (vendor_name) strcpy(sccrq->vender_name, vendor_name);

      
      memcpy(buffer, (char *)sccrq, sizeof(struct PPTP_SCCRq));
      
      free(sccrq);
      return sizeof(struct PPTP_SCCRq);
}

static int 
fill_OCRq_packet(char * buffer, int len, char *phone_number, char *subaddr){
      memset(buffer, 0, len);

      struct PPTP_OCRq * ocrq = (struct PPTP_OCRq *)malloc(sizeof(struct PPTP_OCRq));

      if (ocrq == NULL) return (FAILED);

      ocrq->length = htons(sizeof(struct PPTP_OCRq));
      ocrq->message_type = htons(1);
      ocrq->magic_cookie = htonl(MAGIC_COOKIE);
      ocrq->ctrl_message_type = htons(OCRQ);
      ocrq->call_serial_number = htons(2);
      ocrq->reserved0 = 0;
      ocrq->call_id = htons(CALL_ID);
      ocrq->call_serial_number = htons(2);
      ocrq->min_BPS = htonl(300);
      ocrq->max_BPS = htonl(100000000);
      ocrq->bearer_type = htons(3);
      ocrq->framing_type = htons(3);
      ocrq->packet_recv = htons(64);
      ocrq->packet_prcessing_delay = 0;
      ocrq->phone_number_len = 0;
      ocrq->reserved1 = 0;

      if (phone_number){
            ocrq->phone_number_len = htons(strlen(phone_number));
            strcpy(ocrq->phone_number, phone_number);
      }

      if (subaddr) strcpy(ocrq->subaddr, subaddr);

      memcpy(buffer, (char *)ocrq, sizeof(struct PPTP_OCRq));

      free(ocrq);

      return sizeof(struct PPTP_OCRq);
}

static int 
clip_OCRp_packet(char *buffer, struct vpn_peer *peer)
{
      struct PPTP_OCRp *ocrp = (struct PPTP_OCRp *)buffer;

      // for(int i = 0; i < sizeof(struct PPTP_OCRp); i++)
      //       printf("%02x ", buffer[i]);

      // printf("\n");

      // if (htons(ocrp->message_type) != 1 && htons(ocrp->ctrl_message_type) != OCRP) return (ERROR);
      
      // if (htons(ocrp->peer_call_id) != peer->call_id) return (ERROR);
      
      peer->peer_id = htons(ocrp->call_id);
      printf("%d %d\n", peer->call_id, peer->peer_id);
      // printf("%x %x %x \n", htons(ocrp->call_id), 
      //       htons(ocrp->peer_call_id), 
      //       htons(ocrp->length));
      return (OK);
}

static int 
fill_SLI_packet(char *buffer, int len, u_int16_t peer_call_id)
{
      memset(buffer, 0, len);
      struct PPTP_SLI *sli = (struct PPTP_SLI *)malloc(sizeof(struct PPTP_SLI));

      if (sli == NULL) return (FAILED);

      sli->length = htons(sizeof(struct PPTP_SLI));
      sli->message_type = htons(1);
      sli->magic_cookie = htonl(MAGIC_COOKIE);
      sli->ctrl_message_type = htons(SLI);
      sli->peer_call_id = htons(peer_call_id);
      sli->send_accm = 0xFFFFFFFF;
      sli->receive_accm = 0XFFFFFFFF;

      memcpy(buffer, (char *)sli, sizeof(struct PPTP_SLI));

      return sizeof(struct PPTP_SLI);
}
static int 
send_packet(char *buffer, int sock_fd, int len)
{
      if (send(sock_fd, buffer, len, 0) == -1){
            perror("[Error] Send the packet failed");
            return (FAILED);
      }
      return (OK);
}

static int
read_buffer(char *buffer, int sock_fd, int len)
{
      if(recv(sock_fd, buffer, len, 0) == -1){
            perror("[ERROR] Receive the packet failed");
            return (FAILED);
      }

      return (OK);
}
int 
mainloop(int device_fd, int connection_fd)
{
      
      struct vpn_peer *peer = (struct vpn_peer *)malloc(sizeof(struct vpn_peer));
      if (peer == NULL) return (FAILED);
      printf("Open to establish PPTP for two peers...\n");
      do_PPTP_connection(connection_fd, peer);
      printf("The PPTP connection established\n");

      printf("Open to establish PPP connection\n");
      
      int ppp_fd = make_raw_socket("172.16.15.129", 47);
      if (ppp_fd <= 0) return (FAILED);
      struct sockaddr_in recv;
      recv.sin_addr.s_addr = htonl(INADDR_ANY);
      recv.sin_port = 0;
      recv.sin_family = AF_INET;

      char buffer[BUFSIZ];
      int ret;
      int addr_len =  sizeof(recv);
      if((ret = recvfrom(ppp_fd, buffer, BUFSIZ, 0, (struct sockaddr *)&recv, (socklen_t *)&addr_len)) < 0)
      {
            perror("[ERROR] Receive data failed");
            return (ERROR);
      }
      // printf("Send the ppp packet for Configure Request\n");
      // do_PPP_connection(ppp_fd, peer);
      // printf("The PPP connection established\n");
      printf("ret: %d\n", ret); 
      for(;;);
      return (OK);
}