// How to run the module.
//$ sudo insmod km_param_int.ko i=100

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

static int i=10;
module_param(i, int, 0);
// Following line is Optional
MODULE_PARM_DESC(i, "Integer parameter");
static int __init start_module(void)
{
printk(KERN_INFO "Hello Kernel\n");
printk(KERN_INFO "i=%d\n", i);
return 0;
}
static void __exit end_module(void)
{
printk(KERN_INFO "Bye-bye Kernel\n");
return ;
}
module_init(start_module);
module_exit(end_module);

