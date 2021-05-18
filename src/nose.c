
#include "nose.h"
#include "peer.h"
#include "master_peer.h"
#include "conf/conf-reader.h"

#define FILE_PATH_LEN 256
#define VALUE_LEN     10

enum WORK_MODE{MASTER_PEER, PEER};

int work_mode = MASTER_PEER;

void 
print_usage()
{
      printf("nose is a very simple VPN implementation based on p2p\n\n"
             "Usage: nose [master-peer|peer] \n"
             "    -c the configure file path\n\n\n"
             "Examples:\n"
             "On master peer:\n"
             "nose master-peer -c master_peer.conf\n"
             "On peer:\n"
             "nose peer -c peer.conf\n\n");
}

int
main(int argc, char *argv[])
{
      if (argc <= 1)
      {
            print_usage();
            return (0);
      }

      char conf_path[FILE_PATH_LEN] = {0};


      for(int i = 1; i < argc; i++)
      {
            if (strcmp(argv[i], "peer") == 0)
                  work_mode = PEER;
            else if (strcmp(argv[i], "-c") == 0)
            {
                  char *path = ++i < argc ? argv[i] : NULL;
                  if (NULL == path || !*path)
                  {
                        print_usage();
                        return (0);
                  }
                  strncpy(conf_path, path, strlen(path));
            }
      }    
      
      if (!*conf_path)
      {
            print_usage();
            return (0);
      }

      struct conf_reader cread;
      if (read_conf(&cread, conf_path, "r") != OK)
      {
            fprintf(stderr, "Configure file read filed\n");
            return (0);
      }

      if (work_mode == PEER)
      {
            struct peer pr;
            char value[VALUE_LEN] = {0};
            memset(&pr, 0, sizeof(pr));
            
            get_value(&cread, "source_ipv4", pr.source_ipv4);
            get_value(&cread, "master_peer_ipv4", pr.master_peer_ipv4);
            get_value(&cread, "mstp_id", pr.mstp_id);
            get_value(&cread, "vlan_local_ipv4", pr.vlan_local_ipv4);
            get_value(&cread, "vlan_remote_ipv4", pr.vlan_remote_ipv4);
            get_value(&cread, "stun_server_ipv4", pr.stun_server_ipv4);
            get_value(&cread, "source_port", value);
            pr.source_port = atoi(value);
            memset(value, 0, VALUE_LEN);
            get_value(&cread, "master_peer_port", value);
            pr.master_peer_port = atoi(value);

            get_value(&cread, "key", pr.key);

            fprintf(stdout, "source_ipv4: %s\t"
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
      
            if (init_peer(&pr) <= 0) 
            {
                  fprintf(stderr, "Peer initialized failed\n");
                  return 0;
            }
            return peer_loop(&pr);
      }
      else
      {
            char ipv4[16] = {0}, tmp[6] = {0};
            int port = 9998;

            struct master_peer mstp;
            struct bucket_item items[1] = {{0}};
            memset(&mstp, 0, sizeof(struct master_peer));

            get_value(&cread, "ipv4", ipv4);
            get_value(&cread, "port", tmp);
            port = atoi(tmp);
            memset(tmp, 0, sizeof(tmp));
            get_value(&cread, "node_id", mstp.node_id);
            get_value(&cread, "master_peer_ipv4", items[0].ipv4);
            get_value(&cread, "master_peer_port", tmp);
            items[0].port = atoi(tmp);
            get_value(&cread, "mstp_id", items[0].node_id);
            init_master_peer(&mstp, ipv4, port, items, 1);
            return (master_peer_loop(&mstp));
      }
      
      return 0;
}