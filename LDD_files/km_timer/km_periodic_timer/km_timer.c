// Periodic Kernel Timer
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
static int i = 0;
static int timer_interval = 5000; //in units of milliseconds (5 Seconds) 

// This function will be called periodically.
static void my_timer_func(struct timer_list *tm1)
{
    printk(KERN_INFO "my_timer_func(): i = %d\n", i);
    i++;

    /* Modify the Timer expiration time value by 5 seconds and re-start the
       Timer with increased expiration time value. This will make sure to call
       this Timer function periodically untill stopped/deleted in end_module(). 
    */
    mod_timer(&tm, jiffies + msecs_to_jiffies(timer_interval));

    return ;
}

static int __init start_module(void)
{
    printk(KERN_INFO "Hello Kernel\n");

    // Create (initialize) Kernel Timer dynamically.
    // Run the Kernel Timer after 5000 ms (5 seconds).
    tm.expires = jiffies + msecs_to_jiffies(timer_interval);
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

