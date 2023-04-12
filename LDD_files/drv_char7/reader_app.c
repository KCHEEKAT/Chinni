#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

char rbuf[32];

int main()
{
        int fd;
        int ret;

	printf("Opening the device file.\n");
        fd = open("/dev/cg_char_dev", O_RDWR);
        if(fd < 0) {
                printf("Failed to open the device file. errno=%d\n", errno);
                return -1;
        }
        sleep(2);

        // Read the text driver/device.
	printf("Reading from the device file.\n");
        ret = read(fd, rbuf, 16);
        if(ret < 0) {
                printf("Failed to read from the driver/device.\n");
                return -1;
        }
        // print the text read from driver/device.
        printf("Text read from the driver/device:");
        printf("%s\n", rbuf);
        sleep(2);

        // Close the device file.
	printf("Closing from the device file.\n");
        close(fd);
}
