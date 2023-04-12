#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

unsigned char rbuf[32];
unsigned char wbuf[32];

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

        printf("Enter the text to write into driver/device :");
        scanf("%s", wbuf);

        // Write the text into driver/device.
	printf("Writing into the device file.\n");
        ret = write(fd, wbuf, strlen(wbuf)+1);
        if(ret < 0) {
                printf("Failed to write into the driver/device.\n");
                return -1;
        }
        sleep(4);

        // Read the text driver/device.
        //ret = read(fd, rbuf, sizeof(rbuf));
	printf("Reading from the device file.\n");
        ret = read(fd, rbuf, 16);
        if(ret < 0) {
                printf("Failed to read from the driver/device.\n");
                return -1;
        }
        sleep(4);

        // print the text read from driver/device.
        printf("Text read from the driver/device:");
        printf("%s\n", rbuf);

        // Close the device file.
	printf("Closing from the device file.\n");
        close(fd);
}
