
#ifndef _DEVICE_H_
#define _DEVICE_H_

int utun_open(char *device_name);

int utun_close(int fd);

int utun_read(int fd, char *buffer);

int utun_write(int fd, char *buffer, int len);

#endif // !_DEVICE_H_