// How to run the module.
//$ sudo insmod km_dep1.ko
//$ sudo insmod km_dep2.ko

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

extern int km_dep1_a=10;

extern int km_dep1_b=20;

int km_dep1_add(int, int);

static int __init start_module(void)
{
    int result;
    printk(KERN_INFO "Module2: Hello Kernel\n");
    printk(KERN_INFO "Module2: km_dep1_a=%d\n", km_dep1_a);
    printk(KERN_INFO "Module2: km_dep1_b=%d\n", km_dep1_b);
    result = km_dep1_add(100, 1000);
    printk(KERN_INFO "Module2: Addition Result = %d\n", result);
    return 0;
}
static void __exit end_module(void)
{
    printk(KERN_INFO "Module2: Bye-bye Kernel\n");
    return ;
}
module_init(start_module);
module_exit(end_module);

