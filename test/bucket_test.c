
#include <stdio.h>

#include "../src/bucket.h"
#include "../src/utils.h"


int main()
{
      struct bucket bkt;
      struct bucket_item items[2] = {
            {"123123", "127.0.0.1", 998},
            {"123124", "127.0.0.1", 996}
      };
      gen_random_node_id(items[0].node_id);
      gen_random_node_id(items[1].node_id);
      init_bucket(&bkt, items, 2);

      printf("%d\n", bkt.top);

      pop_front_bucket(&bkt);

      struct bucket_item item = get_front_bucket(&bkt);
      printf("node_id: %s, host: %s:%d\n", item.node_id, item.ipv4, item.port);

      printf("%d\n", bkt.top);
      destory_bucket(&bkt);
      return 0;
}