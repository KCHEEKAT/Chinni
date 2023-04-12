// How to run the module.
//$ sudo insmod km_workq.ko
//$ sudo rmmod km_workq

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/workqueue.h>	// For Work Queue 
#include <linux/delay.h>	// For msleep() 

MODULE_LICENSE("GPL");

struct work_struct ws;

static void my_work_func(struct work_struct *pws)
{
        int i=0;
        for (i = 0; i < 10; i++)
        {
            printk(KERN_INFO "my_work_func(): i = %d\n", i);
            msleep(1000);
	}
        printk(KERN_INFO "my_work_func(): work finished\n");

        return ;
}

static int __init start_module(void)
{
    printk(KERN_INFO "Hello Kernel\n");

    // Creating (initializing) Work item dynamically.
    INIT_WORK(&ws, my_work_func);
    schedule_work(&ws);

    return 0;
}

static void __exit end_module(void)
{

    if (work_pending(&ws)) // Work has not started executing.
        printk(KERN_INFO "Work is pending\n");
    else
        printk(KERN_INFO "Work is executing or executed\n");

    // Enable either cancel_work_sync() or flush_work(), not both together.
    // Cancel the work item if not started executing, else wait till completion.
    cancel_work_sync(&ws);

    //Flush the work items from kernel global Work Queue. Waits till completion.
    //flush_work(&ws);

    printk(KERN_INFO "Bye-bye Kernel\n");

    return ;
}

module_init(start_module);
module_exit(end_module);

