
#include "peer.h"
#include "nose.h"
#include "utils.h"
#include "pmp.h"
#include "pcp.h"
#include "udp.h"
#include "device.h"
#include "net_config.h"
#include "crypt.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <event.h>
#include <zlib.h>
#include <signal.h>

static struct event udp_event, hb_event, fp_event, hello_event;
static struct event_base *ebase;
static struct timeval fp_tv, hello_tv, hb_tv;
static int udp_fd, tun_fd;

#ifdef DEBUG
static inline void show_buf_as_hex(char *buf, int size)
{
      for (int i = 0; i < size; i++)
      {
            printf("%02x", buf[i] & 0xff);

            if ((i + 1) % 8 == 0) printf("\n");
            else printf(" ");
      }

      if (size % 8 != 0) printf("\n");
}
#endif // DEBUG

static int 
create_mstp_conn_sock()
{

      int sockfd, ret;
      
      if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) return (sockfd);
      struct timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;

      if ((ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) < 0)
            return (ret);

      return (sockfd);
}

static int 
registry_peer(struct peer *handler)
{
      
      int sockfd = create_mstp_conn_sock();
      int len = 0, addr_len = sizeof(struct sockaddr_in);
      struct sockaddr_in master_server_addr;
      if (sockfd <= 0) return (ERROR);

      fprintf(stdout, "[INFO] Registry peer %s to the master peer %s[%s:%d]\n", handler->peer_id,
                  handler->mstp_id, handler->master_peer_ipv4, handler->master_peer_port);
      
      master_server_addr.sin_family = AF_INET;
      master_server_addr.sin_port   = htons(handler->master_peer_port);
      master_server_addr.sin_addr.s_addr = inet_addr(handler->master_peer_ipv4);

      char buf[BUFSIZ];
      int try_times = 0;
      int size = PMP_peer_registry_req_pkt( handler->mstp_id, handler->peer_id, 
                  handler->vlan_local_ipv4, handler->nt, buf );

      if (size < 0) return (ERROR);

      do {

            if ( ( len = sendto( sockfd, buf, size, 0,
                        ( struct sockaddr * )&master_server_addr, addr_len ) ) < 0 )
            {
                  fprintf( stderr,"[ERROR] (Sendto)len: %d, err_code: %d, err_msg: %s\n", 
                              len, errno, strerror( errno ) );
                  return (len);
            }
            len = recvfrom( sockfd, buf, BUFSIZ, 0, 
                        ( struct sockaddr * )&master_server_addr, ( socklen_t * )& addr_len );
            try_times++;
      }
      while (len < 0 && try_times <= 3); 

      if (len < 0 && try_times > 3)
      {
            fprintf(stderr, "[INFO] Registry peer failed after trying of three times\n");
            return (ERROR);
      }            

      fprintf(stdout, "[INFO] Registry peer success\n");
      return (OK);
}

static void
find_remote_peer(int sock, short which, void *arg)
{
      struct peer *pr = (struct peer *)arg;
      char buf[BUFSIZ], peer_id[21] = {0}, s_mstp_id[21] = {0};
      char vlan_remote_ipv4[16] = {0}, s_mstp_ipv4[16] = {0};

      struct udp_handler uh;
      int s_mstp_port = pr->master_peer_port, try_times = 6;
      int len = PMP_find_peer_req_pkt(pr->peer_id ,pr->mstp_id, pr->vlan_remote_ipv4, buf);

      if (len < 0) goto next;

      if (send_udp_pkt(&uh, pr->master_peer_ipv4, pr->master_peer_port, 1, buf, len) < 0)
      {
            fprintf( stderr, "[ERROR] (sendto) Peer finds remote peer failed."
                  " err_code: %d, err_msg: %s\n", errno, strerror( errno ) );
            goto next;
      }

      if ((len = recv_udp_pkt(&uh, buf)) < 0)
      {
            fprintf(stderr, "[ERROR] (recv) Master peer %s is dead\n", pr->mstp_id);
            close(uh.sockfd);
            goto next;
      }

      if ( PMP_find_peer_rsp_unpack( peer_id, &pr->peer, buf, len ) != OK )
      {
            fprintf(stderr, "[ERROR] Malformation PMP find peer packet\n");
            close(uh.sockfd);
            goto next;
      }

      struct bucket_item *remote_item = get_front_bucket( &pr->peer );

      if (remote_item != NULL)
      {
            fprintf(stdout, "[INFO] Remote peer %s:%d vlan ip: %s\n", remote_item->ipv4,
                        remote_item->port, remote_item->vlan_ipv4);
            pr->remote_peer_addr.sin_family = AF_INET;
            pr->remote_peer_addr.sin_port = htons(remote_item->port);
            pr->remote_peer_addr.sin_addr.s_addr = inet_addr(remote_item->ipv4);
            // Start to send PCP_hello_syn
            
            hello_tv.tv_sec  = 1;
            hello_tv.tv_usec = 0;

            event_assign(&hello_event, ebase, -1, 0, udp_holing_hello, pr);
            event_add(&hello_event, &hello_tv);
            return;
      }
      
      next: 
            event_add(&fp_event, &fp_tv);
}

static void 
udp_recv_cb(const int sock, short int which, void *arg)
{
      struct peer *pr = (struct peer *)arg;
      struct sockaddr_in src_addr;
      int size = sizeof(struct sockaddr_in);
      char buf[BUFSIZ * 11];
            
      memset(buf, 0, BUFSIZ * 11);      
      int len = recvfrom(sock, buf, BUFSIZ * 11, 0, 
            (struct sockaddr *)(&src_addr), (socklen_t *)&size);

      if (len > 0)
      {
            if (src_addr.sin_addr.s_addr == pr->remote_peer_addr.sin_addr.s_addr &&
                        src_addr.sin_port == pr->remote_peer_addr.sin_port)
            {
                  #ifdef DEBUG
                        fprintf(stdout, "[INFO] Received packet from remote peer(len:%d)\n", len);
                  #endif // DEBUG

                  recved_pkt_unpack(buf, len, pr);
            }
      }
      
}



static void 
udp_holing_hello(int sock, short which, void *arg)
{
      if (arg == NULL) goto next;
      struct peer *pr = (struct peer *)arg;

      if (pr->helloed)
            return;

      char buf[BUFSIZ];
      int len = PCP_hello_syn(buf);
      if (len == ERROR) goto next;
      if ( sendto( udp_fd, buf, len, 0, 
            ( const struct sockaddr * )&pr->remote_peer_addr, 
             sizeof( struct sockaddr_in ) ) < 0 )
      {
            fprintf( stderr, "[ERROR] (sendto) PCP hello syn packet send failed. "
                              "err_code:%d, err_msg %s\n", errno, strerror( errno ) );
            goto next;
      }
      
      fprintf(stdout, "[INFO] PCP hello syn packet sent\n");
      
      next:
            event_add(&hello_event, &hello_tv);

}

static void 
peer_heartbeat(int sock, short which, void *arg)
{
      if (arg == NULL) goto next;

      struct peer *pr = (struct peer *)arg;
      if (!pr->syn_counts)
      {
            // Accumulated there syn packets hasn't received ack, the connection may be broken.
            // We should try to resend hello packet to establish connection.
            fprintf(stderr, "[ERROR] It seems that the P2P connection is broken\n");
            pr->helloed    = 0;
            pr->syn_counts = 3;
            utun_close( tun_fd );
            fprintf(stdout, "[INFO] Tun device is closed\n");
            event_add(&hello_event, &hello_tv);
            return;
      }
      char buf[BUFSIZ];
      int len = PCP_hb_syn(buf);
      if (len == ERROR) goto next;

      if (sendto_remote_peer(buf, len, pr) < 0)
            fprintf( stderr, "[ERROR] (sendto) PCP heartbeat syn packet send failed. " 
                        "err_code:%d, err_msg %s\n", errno, strerror( errno ) );
      fprintf(stdout, "[INFO] PCP heartbeat syn packet sent\n");
      // The syn_count will minus 1 after sent PCP heartbeat syn packet
      pr->syn_counts--;
      next:
            event_add(&hb_event, &hb_tv);

}

static void* 
recv_tun_device_thread(void *arg)
{
      if (arg == NULL) return (NULL);

      struct peer *pr = (struct peer *)arg;
      char buf[BUFSIZ * 11], bbuf[BUFSIZ * 11];
      int len, crypted_len;
      uLongf compressed_len;
      for (;;)
      {
            if ( ( len = utun_read( tun_fd, buf ) ) <= 0 )
            {
                  fprintf( stderr, "[ERROR] Read data from tun device failed,"
                              " err_code: %d, err_msg: %s\n", 
                              errno, strerror( errno ) );
                  continue;
            }
            crypted_len = len;

            #ifdef DEBUG
            fprintf(stdout, "[DEBUG] Read data from tun device, %d bytes\n", len);
            fprintf(stdout, "[DEBUG] (utun)Display data before encrypted(%d):\n", len);
            show_buf_as_hex(buf, len); 
            fprintf(stdout, "[WARN] Primary key: %s\n", pr->key);
            show_buf_as_hex(pr->key, 32);
            #endif

            if ((crypted_len = encrypt_by_aes_256(buf, len, bbuf, pr->key)) == ERROR)
            {
                  fprintf(stderr, "[ERROR] Encrypted data error\n");
                  continue;
                  
            }
            #ifdef DEBUG
                  fprintf(stdout, "[DEBUG] (utun)Display data after encrypted(%d):\n", crypted_len);
                  show_buf_as_hex(bbuf, crypted_len);
            #endif // DEBUG

            // Compress data
            compressed_len = crypted_len * 4;
            if ( Z_OK != compress( ( Bytef * )buf, ( uLongf * )&compressed_len, 
                  ( const Bytef * )bbuf, crypted_len ) )
            {
                  fprintf(stderr, "[ERROR] Data compressed failed\n");
                  return (NULL);
            }

            #ifdef DEBUG
            fprintf(stdout, "[DEBUG] Display data after compressed(%d)\n", compressed_len);
            show_buf_as_hex(buf, compressed_len);
            #endif // DEBUG
            
            len = PCP_payload_pkt(buf, bbuf, compressed_len, len);

            if (sendto_remote_peer(bbuf, len, pr) < 0)
                  fprintf(stderr, "[ERROR] Send payload to remote failed\n");
            
      }
      
}

static void 
recved_pkt_unpack(char *buf, int size, struct peer *pr)
{
      if (size < sizeof(struct PCP))
      {
            fprintf(stderr, "[ERROR] Malformation PCP packet received\n");
            return;
      }

      struct PCP *pcp = (struct PCP*)buf;
      char _buf[BUFSIZ * 11], plaintext[BUFSIZ * 11];
      int len;
      uLongf uncompressed_len;
      uv_buf_t ubuf;
      switch (pcp->flags)
      {
            case F_HELLO_SYN:
                  fprintf(stdout, "[INFO] Received PCP hello syn packet from other peer\n");
                  len = PCP_hello_ack(_buf);
                  if (len == ERROR) return;
                  if (sendto_remote_peer(_buf, len, pr) != OK)
                  {
                        fprintf( stderr, "[ERROR] (sendto) PCP hello ack packet send failed."
                              " err_code:%d, err_msg %s\n", errno, strerror( errno ) );
                        return;
                  }
                  fprintf(stdout, "[INFO] PCP hello ack packet sent\n");
                  break;
            case F_HELLO_ACK:
                  if (pr->helloed)
                  {
                        fprintf(stdout, "[INFO] Previously received PCP hello ack packet, drop it\n");
                        return;
                  }
                  fprintf(stdout, "[INFO] Received PCP hello ack packet from other peer\n");
                  
      
                  // Initialize tun device, assigned with IP address...
                  fprintf(stdout, "[INFO] Open tun device...\n");
                  if (init_tun_device(pr) != OK) 
                  {
                        fprintf(stderr, "[ERROR] Inital tun device failed\n");
                        return;
                  }
                  if ( pthread_create( &pr->tun_thread, NULL, recv_tun_device_thread, pr ) < 0 )
                        return;

                  pr->helloed = 1;
                  hb_tv.tv_sec  = 30;
                  hb_tv.tv_usec = 0;
                  event_assign(&hb_event, ebase, -1, 0, peer_heartbeat, pr);
                  event_add(&hb_event, &fp_tv);

                  // TODO Start to establish VPN connection.
                  // Send authenication request packet to the remote peer
                  // Such API as following denfinition
                  // vpn_init( vpn_handler, vpn_ );
                  break;
            case F_HB_SYN:
                  len = PCP_hb_ack(_buf);
                  if (len == ERROR) return;
                  if (sendto_remote_peer(_buf, len, pr) != OK)
                  {
                        fprintf( stderr, "[ERROR] (sendto) PCP heartbeat ack packet send failed."
                              " err_code:%d, err_msg %s\n", errno, strerror( errno ) );
                        return;
                  }
                  fprintf(stdout, "[INFO] Received PCP heartbeat syn packet from other peer\n");
                  break;
            case F_HB_ACK:
                  // TODO
                  pr->syn_counts++;
                  fprintf(stdout, "[INFO] Received PCP heartbeat ack packet from other peer\n");
                  break;
            case F_PAYLOAD:
                  // TODO 
                  // For receiving the vpn payload, unpack and handle the packet.
                  len = htons(pcp->len);
                  if (size < len + sizeof(struct PCP))
                  {
                        fprintf(stderr, "[ERROR] Malformation PCP payload received\n");
                        return;
                  }
                  memcpy(_buf, buf + sizeof(struct PCP) + sizeof(uint16_t), len - sizeof(uint16_t));
                  const uint16_t text_len = htons(*(uint16_t *)(buf + sizeof(struct PCP)));

                  #ifdef DEBUG
                  fprintf(stdout, "[DEBUG] (p2p) Plain text length: %d bytes\n", text_len);
                  fprintf(stdout, "[DEBUG] (p2p) Display data before decrypted(%d):\n", len - sizeof(uint16_t));
                  show_buf_as_hex(_buf, len - sizeof(uint16_t));

                  fprintf(stdout, "[WARN] Primary key %s\n", pr->key);
                  show_buf_as_hex(pr->key, 32);
                  #endif

                  uncompressed_len = (len - sizeof(uint16_t)) * 4;
                  // Decompress data
                  if ( Z_OK != uncompress( ( Bytef * )buf, ( uLongf * )&uncompressed_len,
                         ( const Bytef * )_buf, len - sizeof( uint16_t ) ) )
                  {
                        fprintf(stderr, "[ERROR] Data uncompressed failed\n");
                        return;
                  }

                  #ifdef DEBUG
                  fprintf(stdout, "[DEBUG] Display data after decompressed(%d):\n", uncompressed_len);
                  show_buf_as_hex(buf, uncompressed_len);
                  #endif // DEBUG
                  
                  // TODO VPN
                  // API defined like this:
                  // vpn_

                  // Decrypt
                  if ( ERROR == decrypt_by_aes_256( ( const char *)buf , 
                              uncompressed_len, plaintext, pr->key ) )
                  {
                        fprintf(stderr, "[INFO] Decrypt data error\n");
                        return;
                  }

                  #ifdef DEBUG
                  fprintf(stdout, "[DEBUG] (p2p) Display data after decrypted(%d):\n",  uncompressed_len);
                  show_buf_as_hex(plaintext, uncompressed_len);
                  fprintf(stdout, "[DEBUG] Received PCP payload packet from other peer\n");
                  #endif // DEBUG

                  if ( utun_write( tun_fd, plaintext, text_len ) <= 0 )
                  {
                        fprintf( stderr, "[ERROR] Write data into tun device failed." 
                                    "err_code:%d err_msg:%s\n", errno, strerror( errno ) );
                        return;
                  }

                  #ifdef DEBUG
                  fprintf(stdout, "[INFO] Write data to tun device, %d bytes\n", text_len);
                  #endif
                  
            default:
                  break;
            }
}

static int 
init_tun_device(struct peer *pr)
{
      char dev_name[20] = "tun0";
      
      if ( ( tun_fd = utun_open( dev_name ) ) <= 0 ) return ( FAILED );

      fprintf(stdout, "[INFO] Setting ip configure\n");
      set_ip_configure(dev_name, pr->vlan_local_ipv4, pr->vlan_remote_ipv4);

      return (OK);
}

static int 
sendto_remote_peer(char *buf, int size, struct peer *pr)
{
      if (buf == NULL || size <= 0 || pr == NULL) return (ERROR);
      if ( sendto( udp_fd, buf, size, 0, 
            ( const struct sockaddr * )&pr->remote_peer_addr, sizeof( struct sockaddr_in ) ) < 0 )
            return (ERROR);
      return (OK);
}

static void
cancel_handle( int sig )
{
      if ( sig == SIGINT)
      {
            if ( tun_fd > 0 )
                  utun_close( tun_fd );
            if ( udp_fd > 0 )
                  close( udp_fd );
            printf("\nQuit\n");
            exit(0);
      }
}

int 
init_peer(struct peer *handler)
{
      if (handler == NULL) return (ERROR);
      init_bucket(&(handler->peer), NULL, 0);
      
      handler->helloed    = 0;
      handler->syn_counts = 3;
      if ( ( udp_fd = get_nat_type( handler->source_ipv4, handler->source_port, 
                  handler->stun_server_ipv4, handler->stun_server_port, &handler->nt ) ) < OK )
      {
            fprintf(
                  stderr, "[ERROR] An error occured when get nat type\n"
            );
            return (ERROR);
      }
      // UDP Block and Sym. UDP Firewall
      if (handler->nt.nat_type <= 1) 
      {
            fprintf(stderr, "[ERROR]  Incapable of UDP connectivity.\n");
            return (ERROR);
      }
      fprintf( stdout, "[INFO] External IP: %s External Port: %d NAT Type: %d\n",
             handler->nt.ipv4, handler->nt.port, handler->nt.nat_type );
      gen_random_node_id(handler->peer_id);


      return (OK);
}

int 
peer_loop(struct peer *handler)
{
      if (handler == NULL) return (ERROR);
      if (registry_peer(handler) == ERROR) return (ERROR);

      ebase = event_base_new();

      fp_tv.tv_sec = 5;
      fp_tv.tv_usec = 0;

      event_assign(&fp_event, ebase, -1, 0, find_remote_peer, handler);
      event_add(&fp_event, &fp_tv);
      
      event_assign( &udp_event, ebase, udp_fd, EV_READ | EV_PERSIST, udp_recv_cb, handler );
      event_add(&udp_event, 0);
      
      signal( SIGINT, cancel_handle );

      return (event_base_loop(ebase, 0));
}