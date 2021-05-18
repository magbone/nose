
#include "pmp.h"
#include "udp.h"
#include "master_peer.h"

#include <string.h>
#include <signal.h>
#include <unistd.h>

static struct event_base *ebase = NULL;
static struct event ping_event, discovery_event, get_peers_event, udp_event;
static struct timeval ten_tv;
static int sock;

static void 
udp_recv_cb(int sock, short which, void *arg)
{

      char source_id[21], target_id[21], b[BUFSIZ];
      struct bucket_item items[256], item;
      struct sockaddr_in raddr;
      uint16_t port;
      int rlen = 0, wlen = 0, size = 0;
      struct master_peer *mstp = (struct master_peer *)arg;
      socklen_t raddr_len = sizeof(struct sockaddr_in);

      if (( rlen = recvfrom( sock, b, BUFSIZ, 0, (struct sockaddr *)&raddr, &raddr_len ) ) < 0)
      {
            fprintf(stderr, "[ERROR] (recvfrom) err_code: %d, err_msg: %s\n",
                        errno, strerror(errno));
      }

      if (rlen > sizeof(struct PMP_Header))
      {
            struct PMP_Header *head = (struct PMP_Header *)b;
            if (head->code == REQ) // Handle request packet  
            {
                  switch (head->type)
                  {
                  case PING:
                        if (PMP_ping_req_unpack(source_id, target_id, b, rlen) == ERROR)
                        {
                              fprintf(stderr, "[ERROR] Malformation PMP ping request packet received\n");
                              return;
                        }
                        // if (strcmp(_mstp->node_id, target_id) != 0)
                        //       goto error_node_id;
                        
                        fprintf(stdout, "[INFO] Received master peer ping from %s\n", source_id);

                        wlen = PMP_ping_rsp_pkt(target_id, source_id, b);
                        break;
                  case DISC:
                        if (PMP_discovery_req_unpack(&port, source_id, target_id, b, rlen) == ERROR)
                        {
                              fprintf(stderr, "[ERROR] Malformation PMP discovery request packet received\n");
                              return;
                        }
                        fprintf(stdout, "[INFO] Received master peer discovery from %s\n", source_id);
                        size = get_top_bucket_items(&mstp->master_peer_bkt, items, 10);
                        wlen = PMP_discovery_rsp_pkt(mstp->node_id, items, size, b);
                        break;
                  case G_PS:
                        if (PMP_get_peers_req_unpack(source_id, target_id, b, rlen) == ERROR)
                        {
                              fprintf(stderr, "[ERROR] Malformation PMP get peers request packet received\n");
                              return; 
                        }

                        fprintf(stdout, "[INFO] Received master peer get peers from %s\n", source_id);
                        size = get_top_bucket_items(&mstp->peer_bkt, items, 10);
                        wlen = PMP_get_peers_rsp_pkt(source_id, items, size, b);
                        break;
                  case R_P:
                        if (PMP_peer_registry_req_unpack(source_id, &(mstp->peer_bkt), b, rlen) == ERROR)
                        {
                              fprintf(stderr, "[ERROR] Malformation PMP get peers request packet received\n");
                              return;
                        }

                        fprintf(stdout, "[INFO] Received peer registry from %s\n", source_id);
                        wlen = PMP_peer_registry_rsp_pkt(mstp->node_id, source_id, b);
                        break;
                  case F_P:
                        fprintf(stdout, "[INFO] Peer bucket size: %d\n", bucket_size(&mstp->peer_bkt));
                        if ((wlen = PMP_find_peer_req_unpack(source_id, target_id, &(mstp->peer_bkt), &item, b, rlen)) == ERROR)
                        {
                              fprintf(stderr, "[ERROR] Malformation PMP find peer request packet received\n");
                              return;
                        }
                        else if (wlen == FAILED)
                        {
                              fprintf(stderr, "[ERROR] Not find the remote peer\n");
                              return;
                        }
                        fprintf(stdout, "[INFO] Received find peer\n");
                        wlen = PMP_find_peer_rsp_pkt(mstp->node_id, source_id, item, b);
                        break;
                  default:
                        break;
                  }


            }
            else 
            {
                  // Other such as response packet is ignored.
            }
      }
      else {
            fprintf(stderr, "[ERROR] Malformation PMP request packet received\n");
            return;
      }

      if (wlen > 0)
      {
            if (sendto(sock, b, wlen, 0, (struct sockaddr *)&raddr, sizeof(raddr)) < 0)
                  fprintf(stderr, "[ERROR] (sendto) err_code: %d, err_msg: %s\n", errno, strerror(errno));
      }
      else 
            fprintf( stderr, "[ERROR] Processing packet error: %d\n", wlen );
      // fprintf(stdout, "[ERROR] Incompatible node id %s(received) -> %s(yours)\n", target_id, _mstp->node_id);


}

static void
ping_peer(int sock, short which, void *arg)
{
      if (arg == NULL) 
      {
            fprintf(stderr, "[ERROR] Null pointer argument passed to ping_peer function\n");
            goto next;
      }
      
      struct master_peer *mstp = (struct master_peer *)arg;
      struct bucket_item *item = get_next_bucket_item(&mstp->master_peer_bkt);
      char buf[BUFSIZ], source_id[21], target_id[21];
      struct udp_handler uh;

      if (item == NULL) goto next;

      fprintf(stdout, "[INFO] Send PMP ping request packet from %s to %s\n", mstp->node_id, item->node_id);

      int len = PMP_ping_req_pkt(mstp->node_id, item->node_id, buf);
      send_udp_pkt(&uh, item->ipv4, item->port, 1, buf, len);
      len = recv_udp_pkt(&uh, buf);

      if (len > 0)
      {
            if (PMP_ping_rsp_unpack(source_id, target_id, buf, len) == OK)
            {
                  if (strcmp(source_id, item->node_id) == 0 && strcmp(target_id, mstp->node_id) == 0)
                        fprintf(stdout, "[INFO] Node %s is alive\n", source_id);
                  else fprintf(stdout, "[INFO] Invalid PMP packet received\n");
            }     
            else fprintf(stdout, "[INFO] Malformation PMP packet received\n");
      }
      else 
      {
            fprintf(stdout, "[INFO] Remote master peer %s is dead\n", item->node_id);
            // TODO remove the item to the bottom of bucket.
      }
      
      next:
            event_add(&ping_event, &ten_tv);
}

static void
discovery_proc(int sock, short which, void *arg)
{
      if (arg == NULL) 
      {
            fprintf(stderr, "[ERROR] Null pointer argument passed to discovery_proc function\n");
            goto next;
      }

      struct master_peer *mstp = (struct master_peer *) arg;     
      struct bucket_item *item = get_front_bucket(&mstp->master_peer_bkt);
      struct udp_handler handler;
      char buf[1024], source_id[21];
      int len = PMP_discovery_req_pkt(item->port, mstp->node_id, item->node_id, buf);

      fprintf(stdout, "[INFO] Send PMP discovery request packet from %s to %s\n", mstp->node_id, item->node_id);

      send_udp_pkt(&handler, item->ipv4, item->port, 1, buf, len);

      len = recv_udp_pkt(&handler, buf);

      if (len > 0)
      {
            if (PMP_discovery_rsp_unpack(mstp->node_id, &mstp->master_peer_bkt, buf, len) == OK)
                  fprintf(stdout, "[INFO] PMP discovery reponse packet receive success\n");
            else fprintf(stdout, "[INFO] Malformation PMP packet received\n");

      }else fprintf(stdout, "[INFO] Remote master peer %s is dead\n", item->node_id);

      next:
            event_add(&discovery_event, &ten_tv);
}

static void 
get_peers(int sock, short which, void *arg)
{
      if (arg == NULL)
      {
            fprintf(stderr, "[ERROR] Null pointer argument passed to discovery_proc function\n");
            goto next;
      }

      struct master_peer *mstp = (struct master_peer *)arg;
      struct bucket_item *item = get_front_bucket(&mstp->master_peer_bkt);
      struct udp_handler uh;
      char buf[BUFSIZ], source_id[20];

      if (item == NULL) goto next;
      int len = PMP_get_peers_req_pkt(mstp->node_id, item->node_id, buf);
      send_udp_pkt(&uh, item->ipv4, item->port, 1, buf, len);
      len = recv_udp_pkt(&uh, buf);

      if (len > 0)
      {
            if (PMP_get_peers_rsp_unpack(source_id, &(mstp->peer_bkt), buf, len) == OK)
                  fprintf(stdout, "[INFO] PMP get peers reponse packet receive success\n");
            else fprintf(stdout, "[ERROR] Malformation PMP get peers response packet received\n");
      }
      else
            fprintf(stdout, "[INFO] Remote master peer %s is dead\n", item->node_id);

      next:
            event_add(&get_peers_event, &ten_tv);
}

static void 
cancel_handle( int sig )
{
      if ( sig == SIGINT )
      {
            if ( sock > 0)
                  close( sock );
            event_base_loopbreak( ebase );
            printf("\nQuit\n");
            exit(0);
      }
}

int 
init_master_peer(struct master_peer *mstp, char *ipv4, int port, struct bucket_item *items, int item_size)
{
      if (mstp == NULL) return (ERROR);

      init_bucket(&(mstp->master_peer_bkt), items, item_size);
      init_bucket(&(mstp->peer_bkt), NULL, 0);
      
      for (int i = 0; i < item_size; i++)
            push_front_bucket(&mstp->master_peer_bkt, *(items + i));

      memset(&ten_tv, 0, sizeof( struct timeval));
      ten_tv.tv_sec  = 5; // 
      ten_tv.tv_usec = 0;

      strncpy(mstp->ipv4, ipv4, strlen(ipv4));
      mstp->port = port;

      return (OK);
}


int 
master_peer_loop(struct master_peer *mstp)
{
      struct sockaddr_in addr;
      int err;

      if (mstp == NULL) return (ERROR);
      
      memset(&addr, 0, sizeof(struct sockaddr_in));
      addr.sin_addr.s_addr = inet_addr(mstp->ipv4);
      addr.sin_family = AF_INET;
      addr.sin_port = htons(mstp->port);
      
      sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

      if (sock <= 0) return (ERROR);

      if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
      {
            fprintf(stderr, "[ERROR] Socket binds address failed. err_code: %d, err_msg: %s\n", 
                        errno, strerror(errno));
            return (ERROR);
      }

      ebase = event_base_new();

      event_assign(&udp_event, ebase, sock, EV_READ | EV_PERSIST, udp_recv_cb, mstp);
      event_add(&udp_event, 0);

      event_assign(&ping_event, ebase, -1, 0, ping_peer, mstp);
      event_add(&ping_event, &ten_tv);

      event_assign(&discovery_event, ebase, -1, 0, discovery_proc, mstp);
      event_add(&discovery_event, &ten_tv);

      event_assign(&get_peers_event, ebase, -1, 0, get_peers, mstp);
      event_add(&get_peers_event, &ten_tv);


      fprintf(stdout, "[INFO] Master peer server runs as %s:%d\n", mstp->ipv4, mstp->port);

      signal( SIGINT, cancel_handle );

      return (event_base_loop(ebase, 0));
}