

#include "../device.h"
#include "../nose.h"


#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int utun_open(char *device_name)
{
      struct ifreq ifr;
      int fd, err, id = 0;

      char *dev_path = "/dev/net/tun", _dev_name[10] = {0};
      fprintf(stdout, "[INFO] Opening a tun device...\n");
      if ((fd = open(dev_path, O_RDWR)) < 0)
      {
            
            perror("[ERROR] Opening /dev/net/tun");
            return fd;
      }
      memset(&ifr, 0, sizeof(ifr));

      ifr.ifr_flags = IFF_TUN | IFF_NO_PI | IFF_TUN_EXCL;

      if (device_name == NULL || !(*device_name)) return (FAILED);
     
            
      do
      {
            printf("%d\n", id);
            memset(_dev_name, 0, 10);
            sprintf(_dev_name, "tun%d", id++);
            strncpy(ifr.ifr_name, _dev_name, IFNAMSIZ);

            if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0)
                  fprintf(stderr, "[ERROR] ioctl(TUNSETIFF) err_code: %d, err_msg: %s\n", errno, strerror(errno));
                  

            if (ioctl(fd, TUNSETPERSIST, 1) >= 0)
            {
                  strcpy(device_name, ifr.ifr_name);
                  break;
                  
            }      
            
            fprintf(stderr, "[ERROR] ioctl(TUNSETPERSIST) err_code: %d, err_msg: %s\n", errno, strerror(errno));
      }
      while(id <= 255);

      if (id == 256)
      {
            fprintf(stderr, "[ERROR] The tun device allocation failed");
            return (ERROR);
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

int utun_close(int fd)
{
      struct ifreq ifr;
      ifr.ifr_flags = IFF_TUN 
            #ifdef IFF_TUN_EXCL
            | IFF_TUN_EXCL
            #endif 
            ;
      int ret = ERROR;
      if (ioctl(fd, TUNSETIFF, ifr))
      {
            fprintf(stderr, "[ERROR] utun_close(ioctl) TUNSETIFF err_code: %d, err_msg %s\n", 
                        errno, strerror(errno));
            goto out;
      }

      if (ioctl(fd, TUNSETPERSIST, 0))
      {
            fprintf(stderr, "[ERROR] utun_close(ioctl) TUNSETPERSIST err_code: %d, err_msg %s\n", 
                        errno, strerror(errno));
            goto out;
      }
      ret = (OK);
      out:
            close(fd);
            return (ret);
}