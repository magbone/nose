#include "../src/master_peer.h"


int main(int argc, char ** argv)
{

      char ipv4[] = "127.0.0.1";
      int port = 9998;
      struct master_peer mstp = {
            .node_id = "aaaaaaaaaaaaaaaaaaaa"
      };
      struct bucket_item item[] = {
            {
                  .node_id = "bbbbbbbbbbbbbbbbbbbb",
                  .ipv4 = "127.0.0.1",
                  .port = 9998
            }
      };
      
      struct bucket_item peers_items[2] = {
            {
                  .node_id = "cccccccccccccccccccc",
                  .ipv4 = "127.0.0.1",
                  .port = 12345,
                  .nat_type = 1,
                  .vlan_ipv4 = "172.0.10.88"
            },
            {
                  .node_id = "dddddddddddddddddddd",
                  .ipv4 = "127.0.0.1",
                  .port = 12346,
                  .nat_type = 2,
                  .vlan_ipv4 = "172.0.10.99"
            }
      };

      init_master_peer(&mstp, ipv4, port, item, 1);

      for (int i = 0; i < 2; i++)
            push_front_bucket(&(mstp.peer_bkt), *(peers_items + i));
      
      return master_peer_loop(&mstp);
}