// Statically allocating major number and statically creating device files.
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");

//Creating the dev with selected major and minor number.
dev_t dev = MKDEV(254, 0);

static int __init drv_char_init(void)
{
    printk(KERN_INFO "Hello Kernel\n");

    register_chrdev_region(dev, 1, "cg_char_dev");
    printk(KERN_INFO "Major Num = %d Minor Num = %d \n",MAJOR(dev), MINOR(dev));

    return 0;
}

static void __exit drv_char_exit(void)
{
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Bye-bye Kernel\n");
    return ;
}
 
module_init(drv_char_init);
module_exit(drv_char_exit);
