

#include "../src/peer.h"

int main()
{
      struct peer pr = {
            .source_ipv4 = "10.66.83.203",
            .source_port = 54320,
            .master_peer_ipv4 = "127.0.0.1",
            .master_peer_port = 9998,
            .mstp_id = "aaaaaaaaaaaaaaaaaaaa",
            .vlan_local_ipv4 = "172.0.10.2"
      };
      
      if (init_peer(&pr) <= 0) return 0;
      peer_loop(&pr);
      return 0;
}