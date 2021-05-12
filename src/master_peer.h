#ifndef _MASTER_PEER_H_
#define _MASTER_PEER_H_

#include "nose.h"
#include "bucket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <event.h>

struct master_peer
{
      struct bucket master_peer_bkt, peer_bkt;
      char ipv4[16];
      int port;
      char node_id[21];
};


static void udp_recv_cb(int sock, short which, void *arg);
static void ping_peer(int sock, short which, void *arg);
static void discovery_proc(int sock, short which, void *arg);
static void get_peers(int sock, short which, void *arg);


int init_master_peer(struct master_peer *mstp, char *ipv4, int port, struct bucket_item *items, int item_size);
int master_peer_loop(struct master_peer *mstp);


#endif // !_MASTER_PEER_H_