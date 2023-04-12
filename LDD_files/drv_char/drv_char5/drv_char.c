// First working virtual char device driver
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //For kmalloc()
#include<linux/uaccess.h>              //For copy_to/from_user()

MODULE_LICENSE("GPL");
 
#define BUF_SIZE     16 
 
// Let the system allocate the major number for this device type/driver.
dev_t dev = 0;
static struct class *cg_dev_class;
static struct cdev cg_cdev;
unsigned char *drv_buf;
//Below static buffer can be used instead of dynamic memory buffer.
//unsigned char drv_buf[BUF_SIZE];

//char driver file operations handler function prototypes
static int      drv_char_open(struct inode *inode, struct file *file);
static int      drv_char_close(struct inode *inode, struct file *file);
static ssize_t  drv_char_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  drv_char_write(struct file *filp, const char *buf, size_t len, loff_t * off);

//char driver file operations initializations 
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = drv_char_read,
        .write          = drv_char_write,
        .open           = drv_char_open,
        .release        = drv_char_close,
};
 
//This function is called when application opens the device file.
static int drv_char_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "drv_char_open() called.\n");
        return 0;
}

//This function is called when application closes the device file.
static int drv_char_close(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "drv_char_close() called.\n");
        return 0;
}

//This function is called when application reads from the device file.
static ssize_t drv_char_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "drv_char_read() called.\n");

        if (len > BUF_SIZE)
        {
            printk(KERN_WARNING "Trying to read more data than buffer size. Trucating read data to buffer size.\n");
            len = BUF_SIZE;
        }

        //Copy the data from the kernel space buffer to the user-space buffer.
        if( copy_to_user(user_buf, drv_buf, len) )
        {
                printk(KERN_ERR "Failed to copy data from kernel space to user space.\n");
		return -EIO;
        }

        printk(KERN_INFO "Data reading successful.\n");
        return len;
}

//This function is called when application writes into the device file.
static ssize_t drv_char_write(struct file *filp, const char __user *user_buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "drv_char_write() called.\n");

        if (len > BUF_SIZE)
        {
            printk(KERN_ERR "Trying to write more data than buffer size.\n");
            return -ENOMEM;
        }

        //Copy the data from user-space buffer to kernel space buffer. 
        if( copy_from_user(drv_buf, user_buf, len) )
        {
                printk(KERN_ERR "Failed to copy data from user space to kernel space.\n");
		return -EIO;
        }

        printk(KERN_INFO "Data writing successful.\n");
        return len;
}

static int __init cg_driver_init(void)
{
        printk(KERN_INFO "Hello Kernel\n");

        //Allocating Major number
        if((alloc_chrdev_region(&dev, 0, 1, "cg_Dev")) <0){
                printk(KERN_ERR "Can not allocate major number for device.\n");
                return -1;
        }
        printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        //Initializing cdev and registering file operations handler functions. 
        cdev_init(&cg_cdev,&fops);
 
        //Adding a character device to the system.
        if((cdev_add(&cg_cdev,dev,1)) < 0){
            printk(KERN_ERR "Can not add the device to the system\n");
	    goto cdev_error;
        }
 
        //Creating device class
        if((cg_dev_class = class_create(THIS_MODULE,"cg_class")) == NULL){
            printk(KERN_ERR "Can not create the the device class for device.\n");
	    goto class_error;
        }
 
        //Creating device
        if((device_create(cg_dev_class,NULL,dev,NULL,"cg_char_dev")) == NULL){
            printk(KERN_ERR "Can not create the device.\n");
            goto device_error;
        }
        
        //Allocating memory buffer
        if((drv_buf = kmalloc(BUF_SIZE , GFP_KERNEL)) == 0){
            printk(KERN_ERR "Can not allocate memory.\n");
            goto kmalloc_error;
        }
        
        memset(drv_buf, 0, BUF_SIZE);
        
        printk(KERN_INFO "Driver initialized successfully.\n");
        return 0;
 
kmalloc_error:
        device_destroy(cg_dev_class,dev);
device_error:
        class_destroy(cg_dev_class);
class_error:
        cdev_del(&cg_cdev);
cdev_error:
        unregister_chrdev_region(dev,1);
        return -1;
}

static void __exit cg_driver_exit(void)
{
        kfree(drv_buf);
        device_destroy(cg_dev_class,dev);
        class_destroy(cg_dev_class);
        cdev_del(&cg_cdev);
        unregister_chrdev_region(dev, 1);

        printk(KERN_INFO "Driver removed\n");
        return ;
}
 
module_init(cg_driver_init);
module_exit(cg_driver_exit);