// Skeleton char driver - File operations in char device driver.
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");

// Let the system allocate the major number for this device type/driver.
dev_t dev = 0;
static struct class *cg_dev_class;
static struct cdev cg_cdev;

// char driver file operations handler function prototypes
static int      drv_char_open(struct inode *inode, struct file *file);
static int      drv_char_close(struct inode *inode, struct file *file);
static ssize_t  drv_char_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  drv_char_write(struct file *filp, const char *buf, size_t len, loff_t * off);

//char driver file operations initializations
static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = drv_char_read,
    .write      = drv_char_write,
    .open       = drv_char_open,
    .release    = drv_char_close,
};

//This function is called when application opens the device file.
static int drv_char_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "drv_char_open() Called.\n");
        return 0;
}

//This function is called when application closes the device file.
static int drv_char_close(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "drv_char_close() Called.\n");
        return 0;
}

//This function is called when application reads from the device file.
static ssize_t drv_char_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "drv_char_read() Called.\n");
        return 0;
}

//This function is called when application writes into the device file.
static ssize_t drv_char_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "drv_char_write() Called.\n");
        return len;
}

static int __init drv_char_init(void)
{
        printk(KERN_INFO "Hello Kernel\n");

        //Allocating Major number
        if((alloc_chrdev_region(&dev, 0, 1, "cg_char_dev")) <0){
                printk(KERN_ERR "Can not allocate major number for device.\n");
                return -1;
        }
        printk("Major Num = %d Minor Num = %d \n",MAJOR(dev), MINOR(dev));

        //Initialize cdev & register the file operations handler functions. 
        cdev_init(&cg_cdev,&fops);

        //Adding a character device to the system
        if((cdev_add(&cg_cdev,dev,1)) < 0){
            printk(KERN_ERR "Can not add the device.\n");
            goto cdev_error;
        }

        //Creating device class
        if((cg_dev_class = class_create(THIS_MODULE,"cg_char_dev_class")) == NULL){
            printk(KERN_ERR "Can not create the device class for device.\n");
            goto class_error;
        }

        //Creating device
        if((device_create(cg_dev_class,NULL,dev,NULL,"cg_char_dev")) == NULL){
            printk(KERN_ERR "Can not create the device.\n");
            goto device_error;
        }

        printk(KERN_INFO "Driver initialized successfully.\n");
        return 0;

device_error:
        class_destroy(cg_dev_class);
class_error:
        cdev_del(&cg_cdev);
cdev_error:
        unregister_chrdev_region(dev,1);
        return -1;
}

static void __exit drv_char_exit(void)
{
        device_destroy(cg_dev_class,dev);
        class_destroy(cg_dev_class);
        cdev_del(&cg_cdev);
        unregister_chrdev_region(dev, 1);

        printk("Driver removed.\n");
        return ;
}

module_init(drv_char_init);
module_exit(drv_char_exit);

