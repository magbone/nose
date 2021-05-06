#ifndef _PEER_H_
#define _PEER_H_

#include "stun/nat_test.h"
#include "bucket.h"
#include "sem.h"

// Peer active heartbeat, we should send heartbeat to mantain the hole per hb_time seconds by peer.
struct peer_active_hb
{
      int hb_time; // 30s
      int hb_try_times; // 3 times

};

struct peer
{
      struct nat_type nt; 
      char peer_id [21], mstp_id[21];
      int master_peer_port, source_port, stun_server_port;
      char vlan_local_ipv4[16], vlan_remote_ipv4[16], master_peer_ipv4[16], source_ipv4[16], stun_server_ipv4[16];

      struct peer_active_hb hb;
      struct sockaddr_in remote_peer_addr;
      struct bucket peer;

      int sockfd;
      int find_peer_timeid;
      struct rk_sema found;
      int holing_hello_timeid;
      int heartbeat_timeid;
      int helloed;
      char key[34]; // The key for encrypting and decrypting the data
      pthread_t recv_thread, tun_thread;

      int tun_fd;
};

static int create_mstp_conn_sock();

static int registry_peer(struct peer *handler);

static void* find_remote_peer(void *arg);
static void* recv_remote_peer_thread(void *arg);
static void* recv_tun_device_thread(void *arg);
static void* udp_holing_hello(void *arg);
static void* peer_heartbeat(void *arg);

static void udp_recv_cb(const int sock, short int which, void *arg);
static void recved_pkt_unpack(char *buf, int size, struct peer *pr);
static int sendto_remote_peer(char *buf, int size, struct peer *pr);

static int init_tun_device(struct peer *pr);
int init_peer(struct peer *handler);

int peer_loop(struct peer *handler); 

#endif // !_PEER_H_