// How to run the module.
//$ sudo insmod km_kthread.ko
//$ sudo rmmod km_kthread

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kthread.h>	// For kthread
#include <linux/sched.h>	// For struct task_struct
#include <linux/delay.h>	// For msleep() 

MODULE_LICENSE("GPL");

struct task_struct *ts;

static int my_kthread_function(void *data)
{
    int i = 0;
    int ret;
    while ((ret = kthread_should_stop()) == 0)
    {
        printk(KERN_INFO "my_kthread_function(): i = %d\n", i);
        msleep(2000);
        i++;
    }
    return 0;
}

static int __init start_module(void)
{
    printk(KERN_INFO "Hello Kernel\n");

    ts = kthread_create(my_kthread_function, NULL, "my_kthread");
    if (ts == NULL)
    {
        printk(KERN_INFO "Failed to create kernel thread.\n");
	return -1;
    }
    else
	wake_up_process(ts);

    return 0;
}

static void __exit end_module(void)
{
    printk(KERN_INFO "Bye-bye Kernel\n");

    if (ts != NULL)
        kthread_stop(ts);

    return ;
}

module_init(start_module);
module_exit(end_module);

