
#include "master_peer.h"
#include "timer.h"
#include "pmp.h"
#include "udp.h"

#include <string.h>


static struct master_peer *_mstp = NULL;

static void 
on_read(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, 
const struct sockaddr* addr, unsigned flags)
{

      char source_id[21], target_id[21], b[BUFSIZ];
      struct bucket_item items[256], item;
      u_int16_t port;
      int len = 0, size = 0;


      if (nread > sizeof(struct PMP_Header))
      {
            struct PMP_Header *head = (struct PMP_Header *)(buf->base);
            if (head->code == REQ) // Handle request packet  
            {
                  switch (head->type)
                  {
                  case PING:
                        if (PMP_ping_req_unpack(source_id, target_id, buf->base, nread) == ERROR)
                        {
                              fprintf(stderr, "[ERROR] Malformation PMP ping request packet received\n");
                              return;
                        }
                        // if (strcmp(_mstp->node_id, target_id) != 0)
                        //       goto error_node_id;
                        
                        fprintf(stdout, "[INFO] Received master peer ping from %s\n", source_id);

                        len = PMP_ping_rsp_pkt(target_id, source_id, b);
                        break;
                  case DISC:
                        if (PMP_discovery_req_unpack(&port, source_id, target_id, buf->base, nread) == ERROR)
                        {
                              fprintf(stderr, "[ERROR] Malformation PMP discovery request packet received\n");
                              return;
                        }
                        fprintf(stdout, "[INFO] Received master peer discovery from %s\n", source_id);
                        size = get_top_bucket_items(&_mstp->master_peer_bkt, items, 10);
                        len = PMP_discovery_rsp_pkt(_mstp->node_id, items, size, b);
                        break;
                  case G_PS:
                        if (PMP_get_peers_req_unpack(source_id, target_id, buf->base, nread) == ERROR)
                        {
                              fprintf(stderr, "[ERROR] Malformation PMP get peers request packet received\n");
                              return; 
                        }

                        fprintf(stdout, "[INFO] Received master peer get peers from %s\n", source_id);
                        size = get_top_bucket_items(&_mstp->peer_bkt, items, 10);
                        len = PMP_get_peers_rsp_pkt(source_id, items, size, b);
                        break;
                  case R_P:
                        if (PMP_peer_registry_req_unpack(source_id, &(_mstp->peer_bkt), buf->base, nread) == ERROR)
                        {
                              fprintf(stderr, "[ERROR] Malformation PMP get peers request packet received\n");
                              return;
                        }

                        fprintf(stdout, "[INFO] Received peer registry from %s\n", source_id);
                        len = PMP_peer_registry_rsp_pkt(_mstp->node_id, source_id, b);
                        break;
                  case F_P:
                        fprintf(stdout, "[INFO] Peer bucket size: %d\n", bucket_size(&_mstp->peer_bkt));
                        if ((len = PMP_find_peer_req_unpack(source_id, target_id, &(_mstp->peer_bkt), &item, buf->base, nread)) == ERROR)
                        {
                              fprintf(stderr, "[ERROR] Malformation PMP find peer request packet received\n");
                              return;
                        }
                        else if (len == FAILED)
                        {
                              fprintf(stderr, "[ERROR] Not find the remote peer\n");
                              return;
                        }
                        fprintf(stdout, "[INFO] Received find peer\n");
                        len = PMP_find_peer_rsp_pkt(_mstp->node_id, source_id, item, b);
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

      if (len > 0)
      {
            uv_buf_t bufs = uv_buf_init(b, len);
            int ret;
            if ((ret = uv_udp_send(&req, handle, &bufs, 1, addr, NULL)) < 0)
                  fprintf(stderr, "[ERROR] %s\n", uv_strerror(ret));
      }


      // fprintf(stdout, "[ERROR] Incompatible node id %s(received) -> %s(yours)\n", target_id, _mstp->node_id);


}


void* 
ping_peer(void *args)
{
      if (args == NULL) return NULL;
      
      struct bucket *b = (struct bucket *)args;
      struct bucket_item *item = get_next_bucket_item(b);
      char buf[BUFSIZ], source_id[21], target_id[21];
      struct udp_handler uh;

      if (item == NULL) return (NULL);

      fprintf(stdout, "[INFO] Send PMP ping request packet from %s to %s\n", _mstp->node_id, item->node_id);

      int len = PMP_ping_req_pkt(_mstp->node_id, item->node_id, buf);
      send_udp_pkt(&uh, item->ipv4, item->port, 1, buf, len);
      len = recv_udp_pkt(&uh, buf);

      if (len > 0)
      {
            if (PMP_ping_rsp_unpack(source_id, target_id, buf, len) == OK)
            {
                  if (strcmp(source_id, item->node_id) == 0 && strcmp(target_id, _mstp->node_id) == 0)
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
      return NULL;
}

static void* 
discovery_proc(void *bucket)
{
      if (bucket == NULL) return NULL;

      struct bucket *b = (struct bucket *)bucket;     
      struct bucket_item *item = get_front_bucket(b);
      struct udp_handler handler;
      char buf[1024], source_id[21];
      int len = PMP_discovery_req_pkt(item->port, _mstp->node_id, item->node_id, buf);

      fprintf(stdout, "[INFO] Send PMP discovery request packet from %s to %s\n", _mstp->node_id, item->node_id);

      send_udp_pkt(&handler, item->ipv4, item->port, 1, buf, len);

      len = recv_udp_pkt(&handler, buf);

      if (len > 0)
      {
            if (PMP_discovery_rsp_unpack(_mstp->node_id, b, buf, len) == OK)
                  fprintf(stdout, "[INFO] PMP discovery reponse packet receive success\n");
            else fprintf(stdout, "[INFO] Malformation PMP packet received\n");

      }else fprintf(stdout, "[INFO] Remote master peer %s is dead\n", item->node_id);

      return NULL;
}

static void* 
get_peers(void *bucket)
{
      if (bucket == NULL) return NULL;

      struct bucket *b = (struct bucket *)bucket;
      struct bucket_item *item = get_front_bucket(b);
      struct udp_handler uh;
      char buf[BUFSIZ], source_id[20];

      if (item == NULL) return (NULL);
      int len = PMP_get_peers_req_pkt(_mstp->node_id, item->node_id, buf);
      send_udp_pkt(&uh, item->ipv4, item->port, 1, buf, len);
      len = recv_udp_pkt(&uh, buf);

      if (len > 0)
      {
            if (PMP_get_peers_rsp_unpack(source_id, &(_mstp->peer_bkt), buf, len) == OK)
                  fprintf(stdout, "[INFO] PMP get peers reponse packet receive success\n");
            else fprintf(stdout, "[ERROR] Malformation PMP get peers response packet received\n");
      }
      else
            fprintf(stdout, "[INFO] Remote master peer %s is dead\n", item->node_id);

      return NULL;
}
static void 
shutdown_master_peer()
{
      if (_mstp != NULL)
      {
            clear_timeout(_mstp->discovery_timeid);
            clear_timeout(_mstp->ping_timeid);
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

      // _mstp->discovery_timeid = set_timeout(10, discovery_proc, &mstp->master_peer_bkt);
      // _mstp->ping_timeid      = set_timeout(10, ping_peer, &mstp->master_peer_bkt);
      mstp->get_peers_timeid = set_timeout(10, get_peers, &mstp->master_peer_bkt);

      strncpy(mstp->ipv4, ipv4, strlen(ipv4));
      mstp->port = port;

      _mstp = mstp;

      return (OK);
}


int 
master_peer_loop(struct master_peer *mstp)
{
      struct sockaddr_in addr;
      int err;

      if (mstp == NULL) return (ERROR);
      
      uv_loop_init(&loop);
      uv_udp_init(&loop, &server);
      if ((err = uv_ip4_addr(mstp->ipv4, mstp->port, &addr)) != 0)
      {
            fprintf(stderr, "[ERROR] %s", uv_strerror(err));
      }
      if ((err = uv_udp_bind(&server, (const struct sockaddr *)&addr, 0)) != 0)
      {
            fprintf(stderr, "[ERROR] %s", uv_strerror(err));
            
      }
      uv_udp_recv_start(&server, alloc_buffer, on_read);
      fprintf(stdout, "[INFO] Master peer server runs as %s:%d\n", mstp->ipv4, mstp->port);

      return uv_run(&loop, UV_RUN_DEFAULT);

}