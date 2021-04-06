
#include "../src/pmp.h"


int main()
{
      char buf[1024];
      // int len = PMP_discovery_req_pkt(9998, "aaaaaaaaaaaaaaaaaaaa", "bbbbbbbbbbbbbbbbbbbb", buf);
      // for (int i = 0; i < len; i++)
      //       printf("%02x\n", buf[i]);
      // printf("%d\n", len);

      struct bucket_item *item = (struct bucket_item *)malloc(sizeof(struct bucket_item));
      memcpy(item->node_id, "bbbbbbbbbbbbbbbbbbbb", 20);
      item->port = htons(6552);
      memcpy(item->ipv4, "192.168.1.1", strlen("192.168.1.1"));
      
      PMP_options_t *opt = (PMP_options_t *)item;

      char *gk = (char *)opt;

      for (int i = 0; i < sizeof(PMP_options_t); i++)
      {
            printf("%02x\n", *(gk + i));
      }
      
      
      return 0;
}