
#include "../device.h"
#include "../nose.h"

#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <unistd.h>


#include <sys/ioctl.h> // ioctl
#include <sys/kern_control.h> // struct socketaddr_ctl
#include <net/if_utun.h> // UTUN_CONTROL_NAME

#include <fcntl.h>


int 
utun_open(char *device_name) {
      int fd;
      char real_name[20];
      memset(real_name, 0, 20);
      sprintf(real_name, "/dev/%s", device_name);
      fprintf(stdout, "[INFO] Open tun device: %s\n", real_name);
      if ((fd = open(real_name, O_RDWR | O_NONBLOCK)) <= 0){
            perror("[ERROR] Open a tun device failed");
            return (FAILED);
      }
      return fd;
}


int utun_read(int fd, char *buffer)
{
      return read(fd, buffer, 1518);
}

int utun_write(int fd, char *buffer, int len)
{
      return write(fd, buffer, len);
}