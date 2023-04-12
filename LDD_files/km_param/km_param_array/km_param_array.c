// How to run the module.
//$ sudo insmod km_param_array.ko a=-1,-2,-3

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

static int a[5]={1,2,3,4,5};
static int array_count=5;
module_param_array(a, int, &array_count, 0);
MODULE_PARM_DESC(a, "Integer Array");

static int __init start_module(void)
{
int i;
printk(KERN_INFO "Hello Kernel\n");
printk(KERN_INFO "Array Count=%d\n", array_count);

for (i=0; i < sizeof(a)/sizeof(int); i++) 
{
printk(KERN_INFO "a[%d]=%d", i, a[i]);
}
printk(KERN_INFO "\n");

return 0;
}
static void __exit end_module(void)
{
printk(KERN_INFO "Bye-bye Kernel\n");
return ;
}
module_init(start_module);
module_exit(end_module);
