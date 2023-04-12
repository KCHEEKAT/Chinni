// How to run the module.
//$ sudo insmod km_param_string.ko s="Howareyou?"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

static char *s="";
module_param(s, charp, 0);
MODULE_PARM_DESC(s, "Character String");

static int __init start_module(void)
{
printk(KERN_INFO "Hello Kernel\n");
printk(KERN_INFO "s=%s\n", s);
return 0;
}
static void __exit end_module(void)
{
printk(KERN_INFO "Bye-bye Kernel\n");
return ;
}
module_init(start_module);
module_exit(end_module);
