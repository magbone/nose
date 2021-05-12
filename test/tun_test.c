

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <unistd.h>
#include <event.h>
#include <string.h> // strlcpy

#include <sys/ioctl.h> // ioctl
#include <sys/kern_control.h> // struct socketaddr_ctl
#include <net/if_utun.h> // UTUN_CONTROL_NAME

#include <fcntl.h>

#define MTU 16384

static uint64_t packet_len = 0;
static uint64_t time_start   = 0;
static struct timeval tv;
static void tun_cb(const int sock, short int which, void *arg)
{
    char buffer[MTU];
     int ret = read(sock, buffer, MTU);
        if (ret == -1)
            fprintf(stderr, "Error code: %d\n", errno);
        
    packet_len += ret;

    gettimeofday(&tv, NULL);

    if (time_start == 0)
    time_start = tv.tv_sec;
    else
    {
        uint64_t time_sub = tv.tv_sec - time_start;

        printf("Bindwidth: %lfBps\n", (double)packet_len / (double)time_sub);
    }
}   


int utun_open() {
    struct ctl_info ctlInfo;
    strlcpy(ctlInfo.ctl_name, UTUN_CONTROL_NAME, sizeof(ctlInfo.ctl_name));

    int fd;
    fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    if (fd < 0) {
        perror("socket");
        return fd;
    }

    struct sockaddr_ctl sc;

    if (ioctl(fd, CTLIOCGINFO, &ctlInfo) == -1) {
        close(fd);
        perror("ioctl");
        return -1;
    }
    printf("ctl_info: {ctl_id: %ud, ctl_name: %s}\n",
           ctlInfo.ctl_id, ctlInfo.ctl_name);

    sc.sc_id = ctlInfo.ctl_id;
    sc.sc_len = sizeof(sc);
    sc.sc_family = AF_SYSTEM;
    sc.ss_sysaddr = AF_SYS_CONTROL;
    sc.sc_unit = 10;


    printf("Connecting...\n");
    if (connect(fd, (struct sockaddr *)&sc, sizeof(sc)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }
    printf("Connected\n");
    // set_nonblock (fd);
    fcntl (fd, F_SETFL, O_NONBLOCK);

    // set_cloexec (fd);
    return fd;
}

int main() {
    struct event tun_event;
    int fd = utun_open();
    printf("%d\n", fd);
    system("ifconfig utun9 10.1.0.2 10.1.0.3");

    event_init();

    event_set(&tun_event, fd, EV_READ|EV_PERSIST, tun_cb, NULL);
    event_add(&tun_event, 0);

    event_dispatch();
    close(fd);
    return 0;
}