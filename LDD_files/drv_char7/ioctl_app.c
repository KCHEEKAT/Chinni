#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "drv_char_ioctl.h"

int main()
{
        int fd;
        int ret;
        unsigned int index_value; // For array data access 
        unsigned int driver_buf_size; // For setting driver buffer size
        unsigned int cur_driver_buf_size; // For getting driver buffer size

	printf("Opening the device file.\n");
        fd = open("/dev/cg_char_dev", O_RDWR);
        if(fd < 0) {
                printf("Failed to open the device file. errno=%d\n", errno);
                return -1;
        }
        sleep(2); 

        // Fill up zeros in the driver buffer.
	// This IOCTL command does not require any argument.
        ret = ioctl(fd, DRV_CHAR_IOCTL_FILL_BUF_WITH_ZERO, 0);
        if (ret < 0)
	    printf("ioctl(DRV_CHAR_IOCTL_FILL_BUF_WITH_ZERO) failed, errno=%d\n", errno);

        // Change the size of driver buffer 
        printf("Enter the new size of driver buffer: \n");
        scanf("%u",&driver_buf_size);
        printf("Changing the size of driver buffer\n");
        ret = ioctl(fd, DRV_CHAR_IOCTL_SET_BUF_SIZE, driver_buf_size); 
        if (ret < 0)
	    printf("ioctl(DRV_CHAR_IOCTL_SET_BUF_SIZE) failed, errno=%d\n", errno);
        sleep(2); 

        // Get the current size of driver buffer 
        printf("Getting the current size of driver buffer\n");
        ret = ioctl(fd, DRV_CHAR_IOCTL_GET_BUF_SIZE, &cur_driver_buf_size); 
        if (ret < 0)
	    printf("ioctl(DRV_CHAR_IOCTL_GET_BUF_SIZE) failed, errno=%d\n", errno);
        printf("Current size of driver buffer: %u\n", cur_driver_buf_size);

        // Fetch the value corresponding to an index from the driver.
        printf("Enter the index: \n");
        scanf("%u",&index_value);
        printf("Fetching the value corresponding to an index from the driver.\n");
        ret = ioctl(fd, DRV_CHAR_IOCTL_INDEX_TO_VALUE, &index_value);
        if (ret < 0)
	    printf("ioctl(DRV_CHAR_IOCTL_INDEX_TO_VALUE) failed, errno=%d\n", errno);
        printf("Value: %u\n", index_value);
        sleep(2); 

        // Close the device file.
	printf("Closing from the device file.\n");
        close(fd);
}
