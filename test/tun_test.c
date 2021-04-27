
#include "../src/device.h"
#include "../src/nose.h"
#include "../src/net_config.h"
#include <errno.h>

#define MTU 1518

int main(void)
{
    int fd;
    char *local_host = "172.16.0.10", *remote_host = "172.16.0.20";
    char device_name[10] = {"tun0"};
    // #if defined(_UNIX) || defined(__APPLE__)
    // if ((fd = utun_open(device_name)) < 0) return (FAILED);
    // #endif
    // printf("Setting ip configure\n");

    // #if defined(__linux)
    // set_ip_configure(device_name, local_host, remote_host);
    // #elif defined(_UNIX) || defined(__APPLE__)
    // set_ip_configure(device_name, local_host, remote_host);
    // #endif 
    // #if defined(__linux)
    if ((fd = utun_open(device_name)) < 0) return (FAILED);
    // #endif
    set_ip_configure(device_name, local_host, remote_host);
    char buffer[MTU];
    memset(buffer, 0, MTU);
    for(;;)
    {
        int ret = utun_read(fd, buffer);
        if (ret == -1)
            fprintf(stderr, "Error code: %d\n", errno);
        
        printf("len: %d\n", ret);
        for (int i = 0; i < ret; i++)
            printf("%02x ", buffer[i]);
        printf("\n");
    }
    return 1;

}