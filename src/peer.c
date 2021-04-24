
#include "peer.h"
#include "nose.h"
#include "utils.h"
#include "pmp.h"
#include "pcp.h"
#include "udp.h"
#include "timer.h"
#include "device.h"
#include "net_config.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

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
      int len;
      for (;;)
      {
            if ((len = utun_read(pr->tun_fd, buf)) < 0)
            {
                  fprintf(stderr, "[ERROR] Read data from tun device failed, err_code: %d, err_msg: %s\n", errno, strerror(errno));
                  continue;
            }
            fprintf(stdout, "[INFO] Read data from tun device, %d bytes\n", len);
            len = PCP_payload_pkt(buf, bbuf, len);

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
      char _buf[BUFSIZ * 2];
      int len;
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
                  memcpy(_buf, buf + sizeof(struct PCP), len);
                  fprintf(stdout, "[INFO] Received PCP payload packet from other peer\n");
                  if (utun_write(pr->tun_fd, _buf, len) < 0)
                  {
                        fprintf(stderr, "[ERROR] Write data into tun device failed. err_code:%d err_msg:%s\n", errno, strerror(errno));
                        return;
                  }
                  fprintf(stdout, "[INFO] Write data to tun device, %d bytes\n", len);
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