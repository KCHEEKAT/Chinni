//NOTE: Kernel modules do not take float, double and char as parameter.
// NOTE: Use long for float and double and single character string for char. 
// How to run the module.
//$ sudo insmod km_param_all.ko c='z' sh=2000 l=20000 a=-1,-2,-3 s="Howareyou?"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

static char *c="x";
module_param(c, charp, 0);

static short sh=1000;
module_param(sh, short, 0);

static long l=10000;
module_param(l, long, 0);

static int a[5]={1,2,3,4,5};
static int array_count=5;
module_param_array(a, int, &array_count, 0);

static char *s="";
module_param(s, charp, 0);

static int __init start_module(void)
{
int i;
printk(KERN_INFO "Hello Kernel\n");

printk(KERN_INFO "c=%s\n", c);
printk(KERN_INFO "sh=%d\n", sh);
printk(KERN_INFO "l=%ld\n", l);

printk(KERN_INFO "Array Count=%d\n", array_count);
for (i=0; i < sizeof(a)/sizeof(int); i++) 
{
printk(KERN_INFO "a[%d]=%d", i, a[i]);
}

printk(KERN_INFO "s=%s\n", s);

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
