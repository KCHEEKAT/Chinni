// How to run the module.
//$ sudo insmod km_timer.ko
//$ sudo rmmod km_timer

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/timer.h>	// For Kernel Timer 
#include <linux/delay.h>	// For mdelay() 

MODULE_LICENSE("GPL");

static struct timer_list tm;

static void my_timer_func(struct timer_list *tm1)
{
    int i;
    for(i=0; i < 10; i++) 
    {
        printk(KERN_INFO "my_timer_func(): i = %d\n", i);
        mdelay(1000); // Use mdelay(), udelay() & ndelay() in Kernel Timer func.

        /* Do not use sleep() functions inside a Kernel Timer function as
           it runs in software interrupt context and leads to unpredictable
           behaviour like System Hang/Panic etc. 
        //msleep(1000);
        */
    }

    return ;
}

static int __init start_module(void)
{
    printk(KERN_INFO "Hello Kernel\n");

    // Create (initialize) Kernel Timer dynamically.
    // Run the Kernel Timer after 5 seconds (=5*HZ).
    tm.expires = jiffies + 5*HZ;
    timer_setup(&tm, my_timer_func, 0);

    // Start the Kernel Timer.
    add_timer(&tm);

    return 0;
}

static void __exit end_module(void)
{
    if(timer_pending(&tm)) // Executing or executed.
        printk(KERN_INFO "Kernel Timer pending\n");
    else
        printk(KERN_INFO "Kernel Timer not pending\n");
          
    // Stop/delete the Kernel Timer.
    del_timer(&tm);

    printk(KERN_INFO "Bye-bye Kernel\n");

    return ;
}

module_init(start_module);
module_exit(end_module);

