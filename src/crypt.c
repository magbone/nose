

#include "crypt.h"
#include "protocols.h"


int app_data_pkt(int id, char *payload, size_t payload_len, char *buffer)
{
      if (payload == NULL || payload_len <= 0)return (ERROR);

      struct vpn_proto_header* header = (struct vpn_proto_header *)malloc(sizeof(struct vpn_proto_header));

      if (header == NULL)
      {
            fprintf(stderr, "[ERROR] Malloc error\n");
            return (ERROR);
      }
      header->code = 0;
      header->type = APP_DATA;
      header->id = htons(id);
      header->length = htons(sizeof(struct vpn_proto_header) + payload_len);
      header->reversed = 0;

      memcpy(buffer, (char*)header, sizeof(struct vpn_proto_header));
      memcpy(buffer + sizeof(struct vpn_proto_header), payload, payload_len);

      free(header);
      return (sizeof(struct vpn_proto_header) + payload_len);     
}
