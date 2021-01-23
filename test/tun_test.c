
#include "../src/device.h"
#include "../src/nose.h"
#include "../src/net_config.h"
#include <errno.h>

#define MTU 1518

int main(void)
{
    int fd;

    if ((fd = utun_open("tun4")) < 0) return (FAILED);

    printf("Setting ip configure\n");
    set_ip_configure("tun4", "10.1.0.10", "10.1.0.20");

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