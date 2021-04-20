
#include <stdio.h>

#include "../src/bucket.h"
#include "../src/utils.h"


int main()
{
      struct bucket bkt;
      struct bucket_item items[4] = {
            {.node_id="123123", .ipv4="127.0.0.1", .port=998},
            {.node_id="123124", .ipv4="127.0.0.1", .port=996},
            {.node_id="123123", .ipv4="127.0.0.1", .port=997},
            {.node_id="123125", .ipv4="127.0.0.1", .port=995}
      };
      init_bucket(&bkt, items, 4);

      printf("%d\n", bucket_size(&bkt));

      // pop_front_bucket(&bkt);

      // struct bucket_item *item = get_front_bucket(&bkt);
      // printf("node_id: %s, host: %s:%d\n", item->node_id, item->ipv4, item->port);


      
      printf("%d\n", bucket_size(&bkt));

      destory_bucket(&bkt);
      return 0;
}