
#include "../src/master_peer.h"
#include "../src/conf/conf-reader.h"

int main(int argc, char ** argv)
{

      char ipv4[16] = {0}, tmp[6] = {0};
      int port = 9998;

      struct conf_reader creader;
      if (argc < 2)
      {
            printf("./master_peer [conf_path]\n");
            return 0;
      }
      if (read_conf(&creader, argv[1], "r") != OK)
      {
            printf("Read conf error\n");
            return 0;
      }

      struct master_peer mstp;
      struct bucket_item items[1] = {{0}};
      memset(&mstp, 0, sizeof(struct master_peer));

      get_value(&creader, "ipv4", ipv4);
      get_value(&creader, "port", tmp);
      port = atoi(tmp);
      memset(tmp, 0, sizeof(tmp));
      get_value(&creader, "node_id", mstp.node_id);
      get_value(&creader, "master_peer_ipv4", items[0].ipv4);
      get_value(&creader, "master_peer_port", tmp);
      items[0].port = atoi(tmp);
      get_value(&creader, "mstp_id", items[0].node_id);
      init_master_peer(&mstp, ipv4, port, items, 1);

      
      return master_peer_loop(&mstp);
}