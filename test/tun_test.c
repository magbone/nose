#include <stdio.h>
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <unistd.h>

#include <string.h> // strlcpy

#include <sys/ioctl.h> // ioctl
#include <sys/kern_control.h> // struct socketaddr_ctl
#include <net/if_utun.h> // UTUN_CONTROL_NAME

#include <fcntl.h>

int assgin_ip(){

    return 0;
}

int utun_open() {
    int fd;
    char device_name[15];
    memset(device_name, 0, 15);
    for(int i = 4; i <= 255; i++){ // Retry to open a new tun devices up to 255 times
        sprintf(device_name, "/dev/tun%d", i);
        printf("device name: %s\n", device_name);
        if ((fd = open(device_name, O_RDWR | O_NONBLOCK)) >= 1)
            break;
    }
    // struct ctl_info ctlInfo;
    // strlcpy(ctlInfo.ctl_name, UTUN_CONTROL_NAME, sizeof(ctlInfo.ctl_name));

    // int fd;
    // fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    // if (fd < 0) {
    //     perror("socket");
    //     return fd;
    // }

    // struct sockaddr_ctl sc;

    // if (ioctl(fd, CTLIOCGINFO, &ctlInfo) == -1) {
    //     close(fd);
    //     perror("ioctl");
    //     return -1;
    // }
    // printf("ctl_info: {ctl_id: %ud, ctl_name: %s}\n",
    //        ctlInfo.ctl_id, ctlInfo.ctl_name);

    // sc.sc_id = ctlInfo.ctl_id;
    // sc.sc_len = sizeof(sc);
    // sc.sc_family = AF_SYSTEM;
    // sc.ss_sysaddr = AF_SYS_CONTROL;
    // sc.sc_unit = 10;

    // if (connect(fd, (struct sockaddr *)&sc, sizeof(sc)) < 0) {
    //     perror("connect");
    //     close(fd);
    //     return -1;
    // }

    // set_nonblock (fd);
    
    // set_cloexec (fd);


    return fd;
}

#define BUFFERSIZE 1500
int main() {
    int fd = utun_open();
    printf("%d\n", fd);
    if (fd < 0) return -1;
    while (1){
        unsigned char buffer[BUFFERSIZE];
        memset(buffer, 0, BUFFERSIZE);
        int i, len;

        len = read(fd, buffer, BUFFERSIZE);

        if (len < 0) continue;
        printf("len: %d\n", len);
        for(i = 0; i < len; i++){
            printf("%02x ", buffer[i]);
        }
        printf("\n");
    }
    close(fd);
    return 0;
}