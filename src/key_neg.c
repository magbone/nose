

#include "protocols.h"
#include "key_neg.h"

int 
key_neg_pkt(int id, u_int8_t *pub_key, size_t key_len, char* buffer)
{
      struct vpn_proto_header* header = (struct vpn_proto_header *)malloc(sizeof(struct vpn_proto_header));

      if (header == NULL)
      {
            fprintf(stderr, "[ERROR] Malloc error\n");
            return (ERROR);
      }
      header->code = 0;
      header->type = KEY_NEG;
      header->id = htons(id);
      header->length = htons(sizeof(struct vpn_proto_header) + key_len);
      header->reversed = 0;

      memcpy(buffer, (char*)header, sizeof(struct vpn_proto_header));
      memcpy(buffer + sizeof(struct vpn_proto_header), (char *)pub_key, key_len);
      free(header);
      return (sizeof(struct vpn_proto_header) + key_len);
}

int key_neg_pkt_validate(char *buffer, u_int8_t *pub_key, size_t key_len)
{
      if (pub_key == NULL) return (ERROR);

      struct vpn_proto_header* header = (struct vpn_proto_header *)buffer;

      if (header->type != KEY_NEG) return (ERROR);

      memcpy(pub_key, buffer + sizeof(struct vpn_proto_header), key_len);

      return (OK);
}