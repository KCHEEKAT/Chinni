// Dynamically allocating major number and dynamically creating device files.
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
 
MODULE_LICENSE("GPL");

// Let the system allocate the major number for this device type/driver.
dev_t dev = 0;
static struct class *cg_dev_class;
 
static int __init drv_char_init(void)
{
        printk(KERN_INFO "Hello Kernel\n");

        //Allocating Major number
        if((alloc_chrdev_region(&dev, 0, 1, "cg_char_dev")) <0){
                printk(KERN_ERR "Can not allocate major number for device.\n");
                return -1;
        }
        printk(KERN_INFO "Major Num = %d Minor Num = %d \n",MAJOR(dev), MINOR(dev));
 
        //Creating device class
        if((cg_dev_class = class_create(THIS_MODULE,"cg_char_dev_class")) == NULL){
            printk(KERN_ERR "Can not create the device class for device.\n");
            goto class_error;
        }
 
        //Creating device
        if((device_create(cg_dev_class,NULL,dev,NULL,"cg_char_device")) == NULL){
            printk(KERN_ERR "Can not create the device.\n");
            goto device_error;
        }

        printk(KERN_INFO "Initialization successful.\n");
        return 0;
 
device_error:
        class_destroy(cg_dev_class);
class_error:
        unregister_chrdev_region(dev,1);
        return -1;
}
 
static void __exit drv_char_exit(void)
{
        device_destroy(cg_dev_class,dev);
        class_destroy(cg_dev_class);
        unregister_chrdev_region(dev, 1);

        printk(KERN_INFO "Bye-bye Kernel\n");
        return ;
}
 
module_init(drv_char_init);
module_exit(drv_char_exit);
