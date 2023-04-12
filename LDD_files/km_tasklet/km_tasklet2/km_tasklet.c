// How to run the module.
//$ sudo insmod km_tasklet.ko
//$ sudo rmmod km_tasklet

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/interrupt.h>	// For Tasklet 
#include <linux/delay.h>	// For mdelay() 

MODULE_LICENSE("GPL");

static struct tasklet_struct tl; 

static void my_tasklet_func(unsigned long data)
{
    int i, count;
    count = data;
    for (i = 0; i < count; i++)
    {
        printk(KERN_INFO "my_tasklet_func(): i = %d\n", i);

        mdelay(1000);	// Use mdelay(), udelay(), ndelay() in Tasklets.
    }
    printk(KERN_INFO "my_tasklet_func(): Tasklet finished\n");

    return ;
}

static int __init start_module(void)
{
    printk(KERN_INFO "Hello Kernel\n");

    // Create (initialize) Tasklet dynamically.
    tasklet_init(&tl, my_tasklet_func, 10);

    // Schedule the Tasklet.
    tasklet_schedule(&tl);

    return 0;
}

static void __exit end_module(void)
{
    // Kill the Tasklet.
    tasklet_kill(&tl);

    printk(KERN_INFO "Bye-bye Kernel\n");

    return ;
}

module_init(start_module);
module_exit(end_module);

