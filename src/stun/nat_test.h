#ifndef _NAT_TEST_H_
#define _NAT_TEST_H_

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "nat_types.h"



#define BIND_REQ              0x0001
#define BIND_RSP              0x0101
#define BIND_ERROR_RSP        0x0111
#define SHARED_SEC_REQ        0x0002
#define SHARED_SEC_RSP        0x0102
#define SHARED_SEC_ERR_RSP    0x0112

struct _STUN_message_header
{
      uint16_t stun_message_type;
      uint16_t message_length;
      uint8_t transaction_id[16];
};

struct _STUN_attribute
{
      uint16_t type;
      uint16_t length;
      // Value (Variable)
};

struct _STUN_attribute_value
{
      uint16_t pro_family;
      uint16_t port;
      uint32_t ipv4;
};

struct rsp_values
{
      char external_ip[16];
      uint16_t external_port;
      char source_ip[16];
      uint16_t source_port;
      char changed_ip[16];
      uint16_t changed_port;
};

static const char *stun_server[] = {
      "stun.ekiga.net",
      "stun.ideasip.com",
      "stun.voiparound.com",
      "stun.voipbuster.com",
      "stun.voipstunt.com",
      "stun.callwithus.com"
};

#define DEFAULT_STUN_PORT     3478
#define DEFAULT_SRC_IP        "0.0.0.0"
#define DEFAULT_SRC_PORT      54320

#define MAPPED_ADDR     0x0001
#define RESP_ADDR       0x0002
#define CHANGE_REQ      0x0003
#define SRC_ADDR        0x0004
#define CHANGE_ADDR     0x0005
#define USERNAME        0x0006
#define PASSWORD        0x0007
#define MSG_INT         0x0008
#define ERR_CODE        0x0009
#define UNKNOWN_ATTR    0x000a
#define REFL_FROM       0x000b
#define XOR_ONLY        0x0021
#define XOR_MAPPED_ADDR 0x8020
#define SERVER_NAME     0x8022
#define SEC_ADDR        0x8050

#define FLAG_CHANGE_IP  0x0004
#define FLAG_CHANGE_PORT 0x0002

static int _create_nat_test_sock(struct sockaddr_in *addr);

static int stun_rsp_unpack(char *buf, int size, struct rsp_values *rvals);

static int test1(int sockfd, struct sockaddr_in *dst_addr,
      struct sockaddr_in *src_addr, char *trans_id,
      struct rsp_values *rvals);

static int test2(int sockfd, struct sockaddr_in *dst_addr,
      struct sockaddr_in *src_addr, char *trans_id,
      struct rsp_values *rvals);

static int test3(int sockfd, struct sockaddr_in *dst_addr,
      struct sockaddr_in *src_addr, char *trans_id,
      struct rsp_values *rvals);

static int do_test(const char *source_addr, const int source_port, 
      const char *stun_server_addr, const int stun_server_port,
      struct nat_type *type);

int get_nat_type(char *source_addr, int source_port, 
      char *stun_server_addr, int stun_server_port,
      struct nat_type *type);



#endif // !_NAT_TEST_H_