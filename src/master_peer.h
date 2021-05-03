#ifndef _MASTER_PEER_H_
#define _MASTER_PEER_H_

#include "nose.h"
#include "bucket.h"

#include <uv.h>

struct master_peer
{
      struct bucket master_peer_bkt, peer_bkt;
      char ipv4[16];
      int port;
      char node_id[21];

      int discovery_timeid;
      int ping_timeid;
      int get_peers_timeid;
};

typedef struct {
      uv_write_t req;
      uv_buf_t buf;
}write_req_t;

static uv_loop_t loop;
static uv_udp_t server;
static uv_udp_send_t req;

void free_write_req(uv_write_t *req);
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);

static void on_read(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);

// void registry_ping_peer(struct bucket_item *item);
// void unregistry_ping_peer(struct bucket_item *item);

void* ping_peer(void *item);
static void* discovery_proc(void *bucket);
static void* get_peers(void *bucket);

static void shutdown_master_peer();

int init_master_peer(struct master_peer *mstp, char *ipv4, int port, struct bucket_item *items, int item_size);
int master_peer_loop(struct master_peer *mstp);


#endif // !_MASTER_PEER_H_