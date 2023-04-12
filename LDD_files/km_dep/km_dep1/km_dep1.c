// How to run the module.
//$ sudo insmod km_dep1.ko
//$ sudo insmod km_dep2.ko

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

int km_dep1_a=10;
EXPORT_SYMBOL(km_dep1_a);

int km_dep1_b=20;
EXPORT_SYMBOL_GPL(km_dep1_b);

//static int mk_dep1_c=30;

int km_dep1_add(int x, int y)
{
    printk(KERN_INFO "Module1: km_dep1_add()\n");
    return x+y;
}
EXPORT_SYMBOL(km_dep1_add);

static int __init start_module(void)
{
    printk(KERN_INFO "Module1: Hello Kernel\n");
    return 0;
}
static void __exit end_module(void)
{
    printk(KERN_INFO "Module1: Bye-bye Kernel\n");
    return ;
}
module_init(start_module);
module_exit(end_module);

