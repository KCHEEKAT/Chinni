// Dynamically allocating major number and statically creating device files.
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
 
MODULE_LICENSE("GPL");

// Let the system allocate the major number for the driver.
dev_t dev = 0;

static int __init drv_char_init(void)
{
        printk(KERN_INFO "Hello Kernel\n");

        //Allocating Major number
        if((alloc_chrdev_region(&dev, 0, 1, "cg_char_dev")) <0){
                printk(KERN_ERR "Can not allocate major number for device.\n");
                return -1;
        }
        printk(KERN_INFO "Major Num = %d Minor Num = %d \n",MAJOR(dev), MINOR(dev));

        printk(KERN_INFO "Driver initialized successfully.\n");
        return 0;
}

static void __exit drv_char_exit(void)
{
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Bye-bye Kernel\n");
}
 
module_init(drv_char_init);
module_exit(drv_char_exit);
