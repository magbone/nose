
#include "peer.h"
#include "nose.h"
#include "utils.h"
#include "pmp.h"
#include "pcp.h"
#include "udp.h"
#include "timer.h"
#include "device.h"
#include "net_config.h"
#include "crypt.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

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
      int size = PMP_peer_registry_req_pkt(handler->mstp_id, handler->peer_id, handler->vlan_local_ipv4, handler->nt, buf);

      if (size < 0) return (ERROR);

      do {

            if ((len = sendto(sockfd, buf, size, 0, (struct sockaddr *)&master_server_addr, addr_len)) < 0)
            {
                  fprintf(stderr,"[ERROR] (Sendto)len: %d, err_code: %d, err_msg: %s\n", len, errno, strerror(errno));
                  return (len);
            }
            len = recvfrom(sockfd, buf, BUFSIZ, 0, (struct sockaddr *)&master_server_addr, (socklen_t *)& addr_len);
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

static void* 
find_remote_peer(void *arg)
{
      struct peer *pr = (struct peer *)arg;
      char buf[BUFSIZ], peer_id[21] = {0};
      struct udp_handler uh;

      int len = PMP_find_peer_req_pkt(pr->peer_id ,pr->mstp_id, pr->vlan_remote_ipv4, buf);

      if (len < 0) return (NULL);

      if (send_udp_pkt(&uh, pr->master_peer_ipv4, pr->master_peer_port, 1, buf, len) < 0)
      {
            fprintf(stderr, "[ERROR] (sendto) Peer finds remote peer failed. err_code: %d, err_msg: %s\n", errno, strerror(errno));
            return (NULL);
      }

      if ((len = recv_udp_pkt(&uh, buf)) < 0)
      {
            fprintf(stderr, "[ERROR] (recv) Master peer %s is dead\n", pr->mstp_id);
            return (NULL);
      }

      if (PMP_find_peer_rsp_unpack(peer_id, &(pr->peer), buf, len) != OK)
      {
            fprintf(stderr, "[ERROR] Malformation PMP find peer packet\n");
            return (NULL);
      }

      struct bucket_item *remote_item = get_front_bucket(&(pr->peer));

      if (remote_item != NULL)
      {
            fprintf(stdout, "[INFO] Remote peer %s:%d vlan ip: %s\n", remote_item->ipv4,
                        remote_item->port, remote_item->vlan_ipv4);
            pr->remote_peer_addr.sin_family = AF_INET;
            pr->remote_peer_addr.sin_port = htons(remote_item->port);
            pr->remote_peer_addr.sin_addr.s_addr = inet_addr(remote_item->ipv4);
            rk_sema_post(&(pr->found));
            clear_timeout(pr->find_peer_timeid);
            
      }
      
      return (NULL);
}

static void* 
recv_remote_peer_thread(void *arg)
{
      if (arg == NULL) return (NULL);
      struct peer *pr = (struct peer *)arg;
      struct sockaddr_in src_addr;
      int size = sizeof(struct sockaddr_in);
      for(;;)
      {
            char buf[BUFSIZ * 2];
            memset(buf, 0, BUFSIZ * 2);
            int len = recvfrom(pr->sockfd, buf, BUFSIZ * 2, 0, 
            (struct sockaddr *)(&src_addr), (socklen_t *)&size);

            if (len > 0)
            {
                  if (src_addr.sin_addr.s_addr == pr->remote_peer_addr.sin_addr.s_addr &&
                        src_addr.sin_port == pr->remote_peer_addr.sin_port)
                  fprintf(stdout, "[INFO] Received packet from remote peer(len:%d)\n", len);
                  recved_pkt_unpack(buf, len, pr);
            }
      }
      return (NULL);
}

static void* 
udp_holing_hello(void *arg)
{
      if (arg == NULL) return (NULL);
      struct peer *pr = (struct peer *)arg;

      if (pr->helloed)
      {
            clear_timeout(pr->holing_hello_timeid);
            return (NULL);
      }
      char buf[BUFSIZ];
      int len = PCP_hello_syn(buf);
      if (len == ERROR) return (NULL);
      if (sendto(pr->sockfd, buf, len, 0, 
            (const struct sockaddr *)&(pr->remote_peer_addr), sizeof(struct sockaddr_in)) < 0)
      {
            fprintf(stderr, "[ERROR] (sendto) PCP hello syn packet send failed. err_code:%d, err_msg %s\n", errno, strerror(errno));
            return (NULL);
      }
      
      fprintf(stdout, "[INFO] PCP hello syn packet sent\n");
      return (NULL);

}

static void* 
peer_heartbeat(void *arg)
{
      if (arg == NULL) return (NULL);

      struct peer *pr = (struct peer *)arg;
      char buf[BUFSIZ];
      int len = PCP_hb_syn(buf);
      if (len == ERROR) return (NULL);

      if (sendto_remote_peer(buf, len, pr) < 0)
            fprintf(stderr, "[ERROR] (sendto) PCP heartbeat syn packet send failed. err_code:%d, err_msg %s\n", errno, strerror(errno));
      fprintf(stdout, "[INFO] PCP heartbeat syn packet sent\n");
      return (NULL);
}

static void* 
recv_tun_device_thread(void *arg)
{
      if (arg == NULL) return (NULL);

      struct peer *pr = (struct peer *)arg;
      char buf[BUFSIZ * 2], bbuf[BUFSIZ * 2];
      int len, crypted_len;
      for (;;)
      {
            if ((len = utun_read(pr->tun_fd, buf)) < 0)
            {
                  fprintf(stderr, "[ERROR] Read data from tun device failed, err_code: %d, err_msg: %s\n", errno, strerror(errno));
                  continue;
            }
            fprintf(stdout, "[INFO] Read data from tun device, %d bytes\n", len);
            #ifdef DEBUG
                  fprintf(stdout, "[DEBUG] (utun)Display data before encrypted(%d):\n", len);
                  show_buf_as_hex(buf, len);
            #endif // DEBUG

            #ifdef DEBUG
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

            len = PCP_payload_pkt(bbuf, buf, crypted_len, len);

            if (sendto_remote_peer(buf, len, pr) < 0)
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
      char _buf[BUFSIZ * 2], plaintext[BUFSIZ * 2];
      int len;
      uint16_t text_len;
      switch (pcp->flags)
      {
            case F_HELLO_SYN:
                  fprintf(stdout, "[INFO] Received PCP hello syn packet from other peer\n");
                  len = PCP_hello_ack(_buf);
                  if (len == ERROR) return;
                  if (sendto_remote_peer(_buf, len, pr) != OK)
                  {
                        fprintf(stderr, "[ERROR] (sendto) PCP hello ack packet send failed. err_code:%d, err_msg %s\n", errno, strerror(errno));
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
                  clear_timeout(pr->holing_hello_timeid);
                  // TODO Start to VPN connection.
                  // Initial tun device, assigned with IP address...
                  if (init_tun_device(pr) != OK) 
                  {
                        fprintf(stderr, "[ERROR] Inital tun device failed\n");
                        return;
                  }
                  if (pthread_create(&(pr->tun_thread), NULL, recv_tun_device_thread, pr) < 0)
                        return;
                  pr->heartbeat_timeid = set_timeout(30, peer_heartbeat, pr);
                  pr->helloed = 1;
                  break;
            case F_HB_SYN:
                  len = PCP_hb_ack(_buf);
                  if (len == ERROR) return;
                  if (sendto_remote_peer(_buf, len, pr) != OK)
                  {
                        fprintf(stderr, "[ERROR] (sendto) PCP heartbeat ack packet send failed. err_code:%d, err_msg %s\n", errno, strerror(errno));
                        return;
                  }
                  fprintf(stdout, "[INFO] Received PCP heartbeat syn packet from other peer\n");
                  break;
            case F_HB_ACK:
                  // TODO

                  fprintf(stdout, "[INFO] Received PCP heartbeat ack packet from other peer\n");
                  break;
            case F_PAYLOAD:
                  // TODO
                  len = htons(pcp->len);
                  if (size < len + sizeof(struct PCP))
                  {
                        fprintf(stderr, "[ERROR] Malformation PCP payload received\n");
                        return;
                  }
                  memcpy(_buf, buf + sizeof(struct PCP) + sizeof(uint16_t), len - sizeof(uint16_t));
                  text_len = htons(*(uint16_t *)(buf + sizeof(struct PCP)));
                  #ifdef DEBUG
                  fprintf(stdout, "[DEBUG] (p2p) Plain text length: %d bytes\n", text_len);
                  #endif // DEBUG
                  #ifdef DEBUG
                  fprintf(stdout, "[DEBUG] (p2p) Display data before decrypted(%d):\n", len - sizeof(uint16_t));
                  show_buf_as_hex(_buf, len - sizeof(uint16_t));
                  #endif // DEBUG
                  // Decrypt
                  #ifdef DEBUG
                  fprintf(stdout, "[WARN] Primary key %s\n", pr->key);
                  show_buf_as_hex(pr->key, 32);
                  #endif
                  if (ERROR == decrypt_by_aes_256((const char *)_buf , len - sizeof(uint16_t), plaintext, pr->key))
                  {
                        fprintf(stderr, "[INFO] Decrypt data error\n");
                        return;
                  }
                  #ifdef DEBUG
                  fprintf(stdout, "[DEBUG] (p2p) Display data after decrypted(%d):\n", len - sizeof(uint16_t));
                  show_buf_as_hex(plaintext, len - sizeof(uint16_t));
                  #endif // DEBUG
                  memcpy(_buf, plaintext, text_len);
                  
                  fprintf(stdout, "[INFO] Received PCP payload packet from other peer\n");
                  if (utun_write(pr->tun_fd, _buf, text_len) < 0)
                  {
                        fprintf(stderr, "[ERROR] Write data into tun device failed. err_code:%d err_msg:%s\n", errno, strerror(errno));
                        return;
                  }
                  fprintf(stdout, "[INFO] Write data to tun device, %d bytes\n", text_len);
            default:
                  break;
            }
}

static int 
init_tun_device(struct peer *pr)
{
      char dev_name[20] = "tun0";
      int fd;
      #if defined(_UNIX) || defined(__APPLE__)
      if ((fd = utun_open(dev_name)) < 0) return (FAILED);
      #endif
      fprintf(stdout, "[INFO] Setting ip configure\n");
      set_ip_configure(dev_name, pr->vlan_local_ipv4, pr->vlan_remote_ipv4);
            
      #if defined(__linux)
      if ((fd = utun_open(dev_name)) < 0) return (FAILED);
      #endif
      pr->tun_fd = fd;
      return (OK);
}

static int 
sendto_remote_peer(char *buf, int size, struct peer *pr)
{
      if (buf == NULL || size <= 0 || pr == NULL) return (ERROR);
      if (sendto(pr->sockfd, buf, size, 0, 
            (const struct sockaddr *)&(pr->remote_peer_addr), sizeof(struct sockaddr_in)) < 0)
            return (ERROR);
      return (OK);
}



int 
init_peer(struct peer *handler)
{
      if (handler == NULL) return (ERROR);
      init_bucket(&(handler->peer), NULL, 0);
      handler->helloed = 0;
      rk_sema_init(&(handler->found), 0);
      if ((handler->sockfd = get_nat_type(handler->source_ipv4, handler->source_port, 
                  handler->stun_server_ipv4, handler->stun_server_port, &(handler->nt))) < OK)
            return (ERROR);
      printf("%s %d %d\n", handler->nt.ipv4, handler->nt.port, handler->nt.nat_type);
      gen_random_node_id(handler->peer_id);

      return (OK);
}

int 
peer_loop(struct peer *handler)
{
      if (handler == NULL) return (ERROR);
      if (registry_peer(handler) == ERROR) return (ERROR);

      handler->find_peer_timeid = set_timeout(10, find_remote_peer, handler);

      rk_sema_wait(&(handler->found));
      

      handler->holing_hello_timeid = set_timeout(2, udp_holing_hello, handler);
      if (pthread_create(&(handler->recv_thread), NULL, recv_remote_peer_thread, handler) < 0)
            return (ERROR);
      pthread_join(handler->recv_thread, NULL);
      return (OK);
}