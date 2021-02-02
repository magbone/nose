
#ifndef _AUTH_H_
#define _AUTH_H_

#include "nose.h"
#include "config.h"
#include "protocols.h"



// Definition of hash methods
#define HASH256 0

struct auth_req_pkt
{
      u_int16_t challenge;
      u_int8_t hash_method; // 0 is hash256 and other is reserved.
      u_int8_t reserved;
      u_int8_t hash[64];
};

#define FAILURE 0 
#define SUCCESS 1

struct auth_rsp_pkt
{
      u_int16_t challenge;
      u_int8_t message_code;
      u_int8_t reserved;
};


#define AUTH_REQ_LEN (sizeof(struct vpn_proto_header) + sizeof(struct auth_req_pkt))
#define AUTH_RSP_LEN (sizeof(struct vpn_proto_header) + sizeof(struct auth_req_pkt))

int auth_request_pkt(int id, int challenge, char* s_key, char* buffer);

int auth_respose_pkt(int id, int challenge, int message_code, char *buffer);

int auth_request_validate(char* buffer, size_t size, struct config conf);

#endif // !_AUTH_H_