

#include "auth.h"
#include "utils.h"

#include <arpa/inet.h>

int 
auth_request_pkt(int id, int challenge, char* s_key, char* buffer)
{
      if (buffer == NULL) return (FAILED);

      struct vpn_proto_header* header = (struct vpn_proto_header *)malloc(sizeof(struct vpn_proto_header));
      if (header == NULL)
      {
            fprintf(stderr, "[ERROR] Malloc error\n");
            return (ERROR);
      } 

      header->code = AUTH_REQ;
      header->type = AUTH;
      header->id = htonl(id);
      header->length = htons(AUTH_REQ_LEN);
      header->reversed = 0;
      
      memcpy(buffer, (char *) header, sizeof(struct vpn_proto_header));

      struct auth_req_pkt* pkt = (struct auth_req_pkt *)malloc(sizeof(struct auth_req_pkt));
      if (pkt == NULL)
      {
            fprintf(stderr, "[ERROR] Malloc error\n");
            return (ERROR);
      }

      char ser_chall_id[256];
      memset(ser_chall_id, 0, 256);
      // SHA256(secret||challege||ID)
      int ret;
      if ((ret = combine_ser_chall_id(s_key, challenge, id, ser_chall_id)) <= 0)
            return (ERROR);
      char hash[65];
      gen_sha256(ser_chall_id, ret, hash);
      pkt->challenge = htons(challenge);
      pkt->hash_method = HASH256;
      pkt->reserved = 0;
      memcpy(pkt->hash, hash, 64);

      free(header);
      free(pkt);
      return AUTH_REQ_LEN;
}

int 
auth_respose_pkt(int id, int challenge, int message_code, char* buffer)
{
      if (buffer == NULL) return (FAILED);

      struct vpn_proto_header* header = (struct vpn_proto_header *)malloc(sizeof(struct vpn_proto_header));
      if (header == NULL)
      {
            fprintf(stderr, "[ERROR] Malloc error\n");
            return (ERROR);
      } 

      header->code = AUTH_RSP;
      header->type = AUTH;
      header->id = htonl(id);
      header->length = htons(AUTH_REQ_LEN);
      header->reversed = 0;
      
      memcpy(buffer, (char *) header, sizeof(struct vpn_proto_header));

      struct auth_rsp_pkt* pkt = (struct auth_rsp_pkt *)malloc(sizeof(struct auth_req_pkt));

      if (pkt == NULL) 
      {
            fprintf(stderr, "[ERROR] Malloc error\n");
            return (ERROR);
      }

      pkt->challenge = htons(challenge);
      pkt->message_code = message_code;
      pkt->reserved = 0;

      memcpy(buffer + sizeof(struct vpn_proto_header), (char *)pkt, sizeof(struct auth_rsp_pkt));

      free(header);
      free(pkt);
      return (AUTH_RSP_LEN);
}

int 
auth_request_validate(char* buffer, size_t size, struct config conf)
{
      if (size < AUTH_REQ_LEN)
            return (FAILED);

      struct vpn_proto_header* header = (struct vpn_proto_header *)buffer;
      if (header->type != AUTH || header->code != AUTH_REQ)
            return (FAILED);
      
      struct auth_req_pkt* pkt = (struct auth_req_pkt *)(buffer + sizeof(struct vpn_proto_header)); 
      char ser_chall_id[256];
      memset(ser_chall_id, 0, 256);
      // SHA256(secret||challege||ID)
      int ret;
      if ((ret = combine_ser_chall_id(conf.key, pkt->challenge, header->id, ser_chall_id)) <= 0)
            return (ERROR);
      char hash[65];
      gen_sha256(ser_chall_id, ret, hash);

      if(strcmp(hash, (char *)pkt->hash) != 0)
            return (ERROR);
      
      conf.id = header->id;

      return (OK);
}