// How to run the module.
//$ sudo insmod km_workq.ko
//$ sudo rmmod km_workq

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/workqueue.h>	// For Work Queue 
#include <linux/delay.h>	// For msleep() 

MODULE_LICENSE("GPL");

static void my_work_func(struct work_struct *pws)
{
        int i;
        for (i = 0; i < 10; i++)
        {
            printk(KERN_INFO "my_work_func(): i = %d\n", i);
            msleep(1000);
	}
        printk(KERN_INFO "my_work_func(): work finished\n");

        return ;
}

// Creating (initializing) Work item statically.
DECLARE_WORK(ws, my_work_func);

// The above line is equivalent to the following declaration and initialization.
// struct work_struct ws = {..., my_work_func, ...};

static int __init start_module(void)
{
    printk(KERN_INFO "Hello Kernel\n");

    schedule_work(&ws);

    return 0;
}

static void __exit end_module(void)
{
    printk(KERN_INFO "Bye-bye Kernel\n");

    return ;
}

module_init(start_module);
module_exit(end_module);

