
#include "master_peer.h"
#include "timer.h"

#include <string.h>

static void 
on_read(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, 
const struct sockaddr* addr, unsigned flags)
{
      printf("Recv: %ld\n", nread);
}

static void registry_ping_peer(struct bucket_item *item)
{
      item->time_id = set_timeout(30, ping_peer, item);
}

static void unregistry_ping_peer(struct bucket_item *item)
{
      clear_timeout(item->time_id);
}

static void* ping_peer(void *item)
{
      if (item == NULL) return NULL;

      struct bucket_item *b = (struct bucket_item *)item;
      
      printf("node_id: %s, host: %s:%d\n", b->node_id, b->ipv4, b->port);
      return NULL;
}

int 
init_master_peer(struct master_peer *mstp, char *ipv4, int port, struct bucket_item *items, int item_size)
{
      if (mstp == NULL) return (ERROR);

      init_bucket(&mstp->master_peer_bkt, items, item_size);
      init_bucket(&mstp->peer_bkt, NULL, 0);
      
      for (int i = 0; i < item_size; i++)
            registry_ping_peer(items + i);
      
      strncpy(mstp->ipv4, ipv4, strlen(ipv4));
      mstp->port = port;

      return (OK);
}


int 
master_peer_loop(struct master_peer *mstp)
{
      struct sockaddr_in addr;
      int error;

      if (mstp == NULL) return (ERROR);
      
      uv_loop_init(&loop);
      uv_udp_init(&loop, &server);
      uv_ip4_addr(mstp->ipv4, mstp->port, &addr);
      uv_udp_bind(&server, (const struct sockaddr *)&addr, 0);
      uv_udp_recv_start(&server, alloc_buffer, on_read);


      return uv_run(&loop, UV_RUN_DEFAULT);

}