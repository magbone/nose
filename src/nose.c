
#include"nose.h"
#include"device.h"
#include"net_config.h"
#include"connection.h"

int
main(int argc, char ** argv)
{
      int fd;

      if ((fd = utun_open("tun4")) < 0) return (FAILED);

      printf("Setting ip configure\n");
      set_ip_configure("tun4", "10.1.0.10", "10.1.0.20");

      int sock_fd;
      if ((sock_fd = make_socket("172.16.15.129", 1723)) < 0) return (FAILED);

      // Doing main loop
      mainloop(fd, sock_fd);

      return 0;
}