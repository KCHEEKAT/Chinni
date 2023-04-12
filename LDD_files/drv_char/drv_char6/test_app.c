#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

// IOCTL commands
/*
//General format of IOCTL commands:
//#define IOCTL_COMMAND_NAME _IOX(MAGIC_NUMBER, COMMAND_NUMBER, DATA_TYPE)
// Where,
// _IO: ioctl command with no parameter
// _IOW: ioctl command with write only parameter 
// _IOR: ioctl command with read only parameter 
// _IOWR: ioctl command with write & read parameter 
*/
//The following lines related to ioctl must be same on both device driver & app.
#define MAGIC_NUMBER    'a'	// 8-bit magic number (any selected number)
#define COMMAND_NUMBER1	1
#define COMMAND_NUMBER2 2
#define PARAM_WR_CMD _IOW(MAGIC_NUMBER, COMMAND_NUMBER1, long int *)
#define PARAM_RD_CMD _IOR(MAGIC_NUMBER, COMMAND_NUMBER2, long int *)

int main()
{
        int fd;
        int ret;
        int wparamvalue, rparamvalue;

	printf("Opening the device file.\n");
        fd = open("/dev/cg_char_dev", O_RDWR);
        if(fd < 0) {
                printf("Failed to open the device file. errno=%d\n", errno);
                return -1;
        }

        // Invoking ioctl() to write a value.
	printf("Enter the Value to send\n");
        scanf("%d",&wparamvalue);
        printf("Writing Value to Driver\n");
        ioctl(fd, PARAM_WR_CMD, (long int *) &wparamvalue); 
        sleep(4); 

        // Invoking ioctl() to read a value.
        printf("Reading Value from Driver\n");
        ioctl(fd, PARAM_RD_CMD, (long int *) &rparamvalue);
        printf("Value is %d\n", rparamvalue);
        sleep(4); 

        // Close the device file.
	printf("Closing from the device file.\n");
        close(fd);
}
