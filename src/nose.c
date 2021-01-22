
#include"nose.h"
#include"device.h"
#include"net_config.h"
#include"config.h"

int
main(int argc, char ** argv)
{
      int fd;

      if ((fd = utun_open("tun4")) < 0) return (FAILED);

      printf("Setting ip configure\n");
      set_ip_configure("tun4", "10.1.0.10", "10.1.0.20");

      struct config conf;
      conf.server_host = NULL;
      conf.server_port = 9090;
      conf.device_fd = fd;

      return 0;
}