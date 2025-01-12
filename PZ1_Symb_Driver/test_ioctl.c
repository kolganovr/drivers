#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define IOCTL_RESET_BUFFER _IOW('k', 1, int)

int main() {
    int fd = open("/dev/pz1_symb_drv", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    int new_size = 512;
    if (ioctl(fd, IOCTL_RESET_BUFFER, &new_size) < 0) {
        fprintf(stderr, "ioctl failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    printf("Buffer reset to size %d\n", new_size);
    close(fd);
    return 0;
}