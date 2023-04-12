#include <linux/module.h>     /* all modules should include */
#include <linux/kernel.h>     /* for KERN_INFO */
#include <linux/init.h>       /* for the macros */
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("cheekatla kaveri device driver");
MODULE_DESCRIPTION("Basic Kernel Module");
MODULE_VERSION("0.1");
  
static int __init hello_start(void)
{
    printk(KERN_INFO "Loading kernel module...\n");
    printk(KERN_INFO "Hello Kernel\n");
    return 0;
}
  
static void __exit hello_end(void)
{
    printk(KERN_INFO "Goodbye Kernel\n");
    printk(KERN_INFO "Unloading kernel module...\n");
}
  
module_init(hello_start);
module_exit(hello_end);
