// System ioctl.h must be included before including this header file in driver and applications. 
// IOCTL commands
/*
//General format of IOCTL commands:
//#define IOCTL_COMMAND_NAME _IOX(MAGIC_NUMBER, COMMAND_NUMBER, DATA_TYPE)
// Where, 
// _IO: For ioctl command with no parameter
// _IOW: For ioctl command with write only parameter
// _IOR: For ioctl command with read only parameter
// _IOWR: For ioctl command with write & read parameter
*/

#define DRV_CHAR_IOCTL_MAGIC_NUMBER    'a' //Choose any 8-bit number (less than 256)
#define DRV_CHAR_IOCTL_FILL_BUF_WITH_ZERO _IO(DRV_CHAR_IOCTL_MAGIC_NUMBER, 0)        
#define DRV_CHAR_IOCTL_SET_BUF_SIZE   _IOW(DRV_CHAR_IOCTL_MAGIC_NUMBER, 1, unsigned int)      
#define DRV_CHAR_IOCTL_GET_BUF_SIZE   _IOR(DRV_CHAR_IOCTL_MAGIC_NUMBER, 2, unsigned int *)       
#define DRV_CHAR_IOCTL_INDEX_TO_VALUE    _IOWR(DRV_CHAR_IOCTL_MAGIC_NUMBER, 3, unsigned int *)
#define DRV_CHAR_IOCTL_MAX_CMDS		DRV_CHAR_IOCTL_INDEX_TO_VALUE	

/*
// Although the following IOCTL commands might work,
// but the safety and security of the system is compromised.
// Its usage might lead to unpredictable system behaviour.
#define DRV_CHAR_IOCTL_FILL_BUF_WITH_ZERO     0
#define DRV_CHAR_IOCTL_SET_BUF_SIZE           1
#define DRV_CHAR_IOCTL_GET_BUF_SIZE           2
#define DRV_CHAR_IOCTL_INDEX_TO_VALUE         3 
#define DRV_CHAR_IOCTL_MAX_CMDS		DRV_CHAR_IOCTL_INDEX_TO_VALUE	
*/

