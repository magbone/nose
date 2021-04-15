#ifndef _PMP_H_
#define _PMP_H_

#include "nose.h"
#include "bucket.h"

// Peer Manage Protocol

#define DISC 0 // Discovery
#define PING 1 // Ping
#define G_PS 2 // Get peers

#define REQ 0 // Request
#define RSP 1 // Response


struct PMP_Header
{
      u_int8_t type;
      u_int8_t code;
};

typedef struct PMP_discovery_req
{
      struct PMP_Header header;
      u_int16_t port;
      u_int8_t target_id[20];
      u_int8_t source_id[20];
}PMP_discovery_req_t;

typedef struct PMP_discovery_rsp
{
      struct PMP_Header header;
      u_int16_t count;
      u_int8_t target_id[20];
      //.....
}PMP_discovery_rsp_t;

typedef struct PMP_options
{
      u_int16_t port;
      u_int16_t reserve;
      u_int32_t ipv4;
      u_int8_t node_id[20];
}PMP_options_t;

typedef struct PMP_ping_req
{
      struct PMP_Header header;
      u_int16_t reserve;
      u_int8_t target_id[20];
      u_int8_t source_id[20];
}PMP_ping_req_t;

typedef PMP_ping_req_t PMP_ping_rsp_t;
typedef PMP_ping_req_t PMP_get_peers_req_t;

typedef struct PMP_get_peers_rsp
{
      struct PMP_Header header;
      u_int16_t count;   
      u_int8_t target_id[20];
}PMP_get_peers_rsp_t;


typedef struct PMP_get_peers_options
{
      u_int8_t nat_type;
      u_int8_t reserve;
      u_int16_t port;
      u_int32_t ipv4;
      u_int8_t node_id[20]; 
}PMP_get_peers_options_t;


int PMP_discovery_req_pkt(u_int16_t port, char *source_id, char *target_id, char *buf);

int PMP_discovery_req_unpack(u_int16_t *port, char *source_id, char *target_id, char *buf, int size);

int PMP_discovery_rsp_pkt(char *target_id, struct bucket_item *items, int size, char *buf);

int PMP_discovery_rsp_unpack(char *source_id, struct bucket *b, char *buf, int size);

int PMP_ping_req_pkt(char *source_id, char *target_id, char *buf);

int PMP_ping_req_unpack(char *source_id, char *target_id, char *buf, int size);

int PMP_ping_rsp_pkt(char *source_id, char *target_id, char *buf);

#define PMP_ping_rsp_unpack(source_id, target_id, buf, size) \
      PMP_ping_req_unpack(source_id, target_id, buf, size)

int PMP_get_peers_req_pkt(char *source_id, char *target_id, char *buf);

#define PMP_get_peeers_req_unpack(source_id, target_id, buf, size) \
      PMP_ping_req_unpack(source_id, target_id, buf, size)

int PMP_get_peers_rsp_pkt(char *target_id, struct bucket_item *items, int size, char *buf);

int PMP_get_peers_rsp_unpack(char *source_id, struct bucket *b, char *buf, int size);





#endif //! _PMP_H_
