

#include "../device.h"
#include "../nose.h"


#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>

int utun_open(char *device_name)
{
      struct ifreq ifr;
      int fd, err;

      char *dev_path = "/dev/net/tun";

      fprintf(stdout, "[INFO] Opening a tun device...\n");
      if ((fd = open(dev_path, O_RDWR)) < 0)
      {
            
            perror("[ERROR] Opening /dev/net/tun");
            return fd;
      }

      memset(&ifr, 0, sizeof(ifr));

      ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

      if (device_name == NULL || !(*device_name)) return (FAILED);
      int id = 1;
      do{
            
            strncpy(ifr.ifr_name, device_name, IFNAMSIZ);

            if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) > 0)
                  break;
            
            sprintf(device_name, "tun%d", id);
            memset(ifr.ifr_name, 0, IFNAMSIZ);
      }while (id <= 255);
      
      if (id > 255) 
      {
            fprintf(stdout, "[ERROR] Device allocation is out of range\n");
            close(fd);
            return (FAILED);
      }
      strcpy(device_name, ifr.ifr_name);
      
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
