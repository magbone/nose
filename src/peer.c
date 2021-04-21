
#include "peer.h"
#include "nose.h"
#include "utils.h"
#include "pmp.h"
#include "udp.h"
#include "timer.h"
#include <errno.h>
#include <string.h>

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
      struct peer_active_hb hb;
      int sockfd = create_mstp_conn_sock();
      int len = 0, addr_len = sizeof(struct sockaddr_in);
      if (sockfd <= 0) return (ERROR);

      fprintf(stdout, "[INFO] Registry peer %s to the master peer %s[%s:%d]\n", handler->peer_id,
                  handler->mstp_id, handler->master_peer_ipv4, handler->master_peer_port);

      hb.sockfd = sockfd;
      hb.dst_addr.sin_family = AF_INET;
      hb.dst_addr.sin_addr.s_addr = inet_addr(handler->master_peer_ipv4);
      hb.dst_addr.sin_port = htons(handler->master_peer_port);

      handler->hb = hb;

      char buf[BUFSIZ];
      int try_times = 0;
      int size = PMP_peer_registry_req_pkt(handler->mstp_id, handler->peer_id, handler->vlan_local_ipv4, handler->nt, buf);

      if (size < 0) return (ERROR);

      do {

            if ((len = sendto(sockfd, buf, size, 0, (struct sockaddr *)&(handler->hb.dst_addr), addr_len)) < 0)
            {
                  fprintf(stderr,"[ERROR] (Sendto)len: %d, err_code: %d, err_msg: %s\n", len, errno, strerror(errno));
                  return (len);
            }
            len = recvfrom(sockfd, buf, BUFSIZ, 0, (struct sockaddr *)&(handler->hb.dst_addr), (socklen_t *)& addr_len);
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
find_remote_peer(const void *args)
{
      struct peer *pr = (struct peer *)args;
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
            clear_timeout(pr->find_peer_timeid);
      }
      
      return (NULL);
}

int 
init_peer(struct peer *handler)
{
      if (handler == NULL) return (ERROR);
      init_bucket(&(handler->peer), NULL, 0);

      if ((handler->sockfd = get_nat_type(handler->source_ipv4, handler->source_port, NULL, 0, &(handler->nt))) < OK)
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

      for(;;);
      return (OK);
}