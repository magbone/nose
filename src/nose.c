
#include "nose.h"
#include "device.h"
#include "net_config.h"
#include "utils.h"
#include "client.h"
#include "server.h"

enum WORK_MODE{CLIENT, SERVER};

int work_mode = CLIENT;

int
check_mode_argus(struct config conf)
{
      if (work_mode == CLIENT)
            return conf.server_host != NULL && conf.local_host != NULL && \
                  conf.remote_host != NULL && conf.server_port > 0;
      else return conf.server_host != NULL && conf.server_port > 0;
} 
void 
print_usage()
{
      printf("nose is a very simple VPN implement over p2p\n\n"
             "Usage: nose [server|client] \n"
             "  -l IP address assigned to the utun interface of local machine\n"
             "  -r IP address assigned to the other peer of p2p\n"
             "  -sh IP address of public server for forwarding the traffic\n"
             "  -sp Port number given to the public server\n\n\n"
             "Examples:\n"
             "In client:\n"
             "nose client -l 10.1.0.10 -r 10.1.0.20 -sh x.x.x.x -sp 9090\n"
             "In server:\n"
             "nose server -sh x.x.x.x -sp 9090\n\n");
}

int
main(int argc, char *argv[])
{
      if (argc <= 1)
      {
            print_usage();
            return 0;
      }

      struct config conf;
      memset(&conf, 0, sizeof(struct config));

      for(int i = 1; i < argc; i++)
      {
            if (strcmp(argv[i], "server") == 0)
                  work_mode = SERVER;
            else if (strcmp(argv[i], "-l") == 0)
            {
                  char *address = ++i < argc ? argv[i] : NULL;
                  if (!address || !is_valid_ip_address(address))
                  {
                        printf("Invalid argument: %s\n", address);
                        return (FAILED);
                  }
                  conf.local_host = address;
            }
            else if (strcmp(argv[i], "-r") == 0)
            {
                  char *address = ++i < argc ? argv[i] : NULL;
                  if (!address || !is_valid_ip_address(address))
                  {
                        printf("Invalid argument: %s\n", address);
                        return (FAILED);
                  }
                  conf.remote_host = address;
            }
            else if (strcmp(argv[i], "-sh") == 0)
            {
                  char *address = ++i < argc ? argv[i] : NULL;
                  if (!address ||!is_valid_ip_address(address))
                  {
                        printf("Invalid argument: %s\n", address);
                        return (FAILED);
                  }
                  conf.server_host = address;
            }
            else if (strcmp(argv[i], "-sp") == 0)
            {
                  char *port_str = ++i < argc ? argv[i] : NULL;
                  int port;
                  if (!port_str || !(port = atoi(port_str))){
                        printf("Invalid argument: %s\n", port_str);
                        return (FAILED);
                  }
                  conf.server_port = port;
            }
      }

      // Check the work mode and its conrresponding arguments.
      if (!check_mode_argus(conf))
      {
            printf("Lack some arguments\n");
            return (FAILED);
      }

      char dev_name[20] = "tun0";

      if (work_mode == CLIENT)
      {
            int fd;
            #if defined(_UNIX) || defined(__APPLE__)
            if ((fd = utun_open(dev_name)) < 0) return (FAILED);
            #endif
            printf("Setting ip configure\n");

            #if defined(__linux)
            set_ip_configure(dev_name, conf.local_host, conf.remote_host);
            #elif defined(_UNIX) || defined(__APPLE__)
            set_ip_configure(dev_name, conf.local_host, conf.remote_host);
            #endif 
            
            #if defined(__linux)
            if ((fd = utun_open(dev_name)) < 0) return (FAILED);
            #endif

            conf.utun_fd = fd;

            return client_loop(conf);
      }
      else{
            fprintf(stdout, "[INFO] Run as server mode\n");
            return server_loop(conf);
      }
      return 0;
}