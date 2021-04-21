#ifndef _PEER_H_
#define _PEER_H_

#include "stun/nat_test.h"
#include "bucket.h"

// Peer active heartbeat, we should send heartbeat to mantain the hole per hb_time seconds by peer.
struct peer_active_hb
{
      int sockfd;
      struct sockaddr_in dst_addr;
      int hb_time; // 30s

};

struct peer
{
      struct nat_type nt; 
      char peer_id [21], mstp_id[21];
      int master_peer_port, source_port;
      char vlan_local_ipv4[16], vlan_remote_ipv4[16], master_peer_ipv4[16], source_ipv4[16];

      struct peer_active_hb hb;
      int sockfd;
      int find_peer_timeid;

      struct bucket peer;
};

static int create_mstp_conn_sock();

static int registry_peer(struct peer *handler);

static void* find_remote_peer(const void *args);

int init_peer(struct peer *handler);

int peer_loop(struct peer *handler); 

#endif // !_PEER_H_