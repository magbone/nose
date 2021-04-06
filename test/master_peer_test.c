#include "../src/master_peer.h"


int main()
{
      char ipv4[] = "127.0.0.1";
      int port = 9987;
      struct master_peer mstp;
      struct bucket_item item[] = {
            {
                  .node_id = "hellworld",
                  .ipv4 = "127.0.0.1",
                  .port = 9996
            }
      };
      
      init_master_peer(&mstp, ipv4, port, item, 1);
      return master_peer_loop(&mstp);
}