// How to run the module.
//$ sudo insmod km_tasklet.ko
//$ sudo rmmod km_tasklet

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/interrupt.h>	// For Tasklet 
#include <linux/delay.h>	// For mdelay() 

MODULE_LICENSE("GPL");

static void my_tasklet_func(unsigned long data)
{
    int i, count;
    count = data;
    for (i = 0; i < count; i++)
    {
        printk(KERN_INFO "my_tasklet_func(): i = %d\n", i);

        /* Do not use sleep() functions inside a Tasklet function.
           Sleeping inside a Tasklet results in unpredictable behaviour
           and kernel panic as Tasklet runs in Software interrupt context.
        // msleep(1000);
        */

        /* Use mdelay(), udelay(), ndelay() in Tasklets as processor loops
           inside these delay() functions, which is the desired behaviour in
           Tasklet as it runs in Software interrupt contex.
         */
        mdelay(1000);	// Use mdelay(), udelay(), ndelay() in Tasklets.
    }
    printk(KERN_INFO "my_tasklet_func(): Tasklet finished\n");

    return ;
}

// Create (initialize) Tasklet statically.
DECLARE_TASKLET(tl, my_tasklet_func, 10);

// The above line is equivalent to the following declaration and initialization.
// struct tasklet_struct tl = {..., my_tasklet_func, data, ...};

static int __init start_module(void)
{
    printk(KERN_INFO "Hello Kernel\n");

    // Schedule the Tasklet.
    tasklet_schedule(&tl);

    return 0;
}

static void __exit end_module(void)
{
   /*  However, we are not able to kill the tasklet during its execution by
       removing the module as the shell gets pre-empted and blocked by Tasklet
       running at higher priority. Other way of calling the tasklet_kill() 
       function should be used for killing the Tasklet during its execution.
       The Tasklet can be killed from hardware interrupt service routine which
       runs at higher priority than Tasklets. 
    */ 
    // Kill the Tasklet.
    tasklet_kill(&tl);

    printk(KERN_INFO "Bye-bye Kernel\n");

    return ;
}

module_init(start_module);
module_exit(end_module);

