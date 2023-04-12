// How to run the module.
//$ sudo insmod km_mutex.ko
//$ sudo rmmod km_mutex

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kthread.h>	// For kthread
#include <linux/sched.h>	// For struct task_struct
#include <linux/delay.h>	// For msleep() 

#include <linux/mutex.h>	// For mutex lock

MODULE_LICENSE("GPL");

struct task_struct *ts1;
struct task_struct *ts2;

static int g_var=0;
static struct mutex mtx;

static int kthread_func1(void *data)
{
    int ret;
    while ((ret = kthread_should_stop()) == 0)
    {
	mutex_lock(&mtx);
	g_var++;
        printk(KERN_INFO "kthread_func1(): g_var = %d\n", g_var);
	mutex_unlock(&mtx);
        msleep(1000);
    }
    return 0;
}

static int kthread_func2(void *data)
{
    int ret;
    while ((ret = kthread_should_stop()) == 0)
    {
	mutex_lock(&mtx);
	g_var++;
        printk(KERN_INFO "kthread_func2(): g_var = %d\n", g_var);
	mutex_unlock(&mtx);
        msleep(1000);
    }
    return 0;
}

static int __init start_module(void)
{
    printk(KERN_INFO "Hello Kernel\n");

    mutex_init(&mtx);

    ts1 = kthread_run(kthread_func1, NULL, "my_kthread1");
    if (ts1 == NULL)
    {
        printk(KERN_INFO "Failed to create kernel thread my_kthread1.\n");
	return -1;
    }

    ts2 = kthread_run(kthread_func2, NULL, "my_kthread2");
    if (ts2 == NULL)
    {
        printk(KERN_INFO "Failed to create kernel thread my_kthread2.\n");
	return -1;
    }

    return 0;
}

static void __exit end_module(void)
{
    printk(KERN_INFO "Bye-bye Kernel\n");

    if (ts1 != NULL)
        kthread_stop(ts1);

    if (ts2 != NULL)
        kthread_stop(ts2);

    return ;
}

module_init(start_module);
module_exit(end_module);

