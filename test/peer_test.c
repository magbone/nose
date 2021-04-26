

#include "../src/peer.h"
#include "../src/conf/conf-reader.h"
#include "../src/nose.h"


int main(int argc, char **argv)
{
      if (argc < 2) 
      {
            printf("./peer_test [conf_path]\n");
            return 0;
      }
      struct peer pr;
      struct conf_reader cread;
      memset(&pr, 0, sizeof(struct peer));

      if (read_conf(&cread, argv[1], "r") != OK)
      {
            printf("Read failed\n");
            return 0;
      }
      char value[10] = {0};
      get_value(&cread, "source_ipv4", pr.source_ipv4);
      get_value(&cread, "master_peer_ipv4", pr.master_peer_ipv4);
      get_value(&cread, "mstp_id", pr.mstp_id);
      get_value(&cread, "vlan_local_ipv4", pr.vlan_local_ipv4);
      get_value(&cread, "vlan_remote_ipv4", pr.vlan_remote_ipv4);
      get_value(&cread, "stun_server_ipv4", pr.stun_server_ipv4);
      get_value(&cread, "source_port", value);
      pr.source_port = atoi(value);
      memset(value, 0, 10);
      get_value(&cread, "master_peer_port", value);
      pr.master_peer_port = atoi(value);

      get_value(&cread, "key", pr.key);


      printf("source_ipv4: %s\t"
            "souce_port: %d\t"
            "stun_server_ipv4: %s\t"
            "stun_server_port: %d\t"
            "master_peer_ipv4: %s\t"
            "mstp_id: %s\n",
            pr.source_ipv4,
            pr.source_port,
            pr.stun_server_ipv4,
            pr.stun_server_port,
            pr.master_peer_ipv4,
            pr.mstp_id);
      
      if (init_peer(&pr) <= 0) return 0;
      if (peer_loop(&pr) == ERROR)
            printf("Shutdown by error\n");
      return 0;
}