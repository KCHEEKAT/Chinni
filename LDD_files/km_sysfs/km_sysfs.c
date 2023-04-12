// How to run the module.
//$ sudo insmod km_sysfs.ko

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include<linux/sysfs.h> 
#include<linux/kobject.h> 

MODULE_LICENSE("GPL");

volatile int km_sysfs_i=0;
volatile char km_sysfs_s[10]="Hello";

struct kobject *kobj_ref;

static ssize_t  km_sysfs_show(struct kobject *kobj, 
                        struct kobj_attribute *attr, char *buf)
{
        printk(KERN_INFO "km_sysfs - read operation\n");
        return sprintf(buf, "%d\n", km_sysfs_i);
}

static ssize_t  km_sysfs_save(struct kobject *kobj, 
                        struct kobj_attribute *attr,const char *buf, size_t count)
{
        printk(KERN_INFO "km_sysfs - write operation\n");
        sscanf(buf,"%d\n",&km_sysfs_i);
        return count;
}

static ssize_t  km_sysfs_msg_show(struct kobject *kobj, 
                        struct kobj_attribute *attr, char *buf)
{
        printk(KERN_INFO "km_sysfs - read operation\n");
        return sprintf(buf, "%s\n", km_sysfs_s);
}

static ssize_t  km_sysfs_msg_save(struct kobject *kobj, 
                        struct kobj_attribute *attr,const char *buf, size_t count)
{
        printk(KERN_INFO "km_sysfs - write operation\n");
        sscanf(buf,"%s\n",km_sysfs_s);
        return count;
}

struct kobj_attribute km_sysfs_attr = __ATTR(xyz, 0660, km_sysfs_show, km_sysfs_save);
struct kobj_attribute km_sysfs_msg_attr = __ATTR(abc, 0660, km_sysfs_msg_show, km_sysfs_msg_save);


static int __init start_module(void)
{
    printk(KERN_INFO "Hello Kernel\n");

    // Creating dir under /sys/module
    kobj_ref = kobject_create_and_add("kmsysfs",kernel_kobj);
    if (kobj_ref == NULL)
    {
         printk("Failed to create sysfs dir.\n");
         goto sysfs_dir_err;
    }
 
    // Creating sysfs file for km_sysfs_i 
    if(sysfs_create_file(kobj_ref,&km_sysfs_attr.attr))
    {
         printk("Failed to create sysfs file.\n");
         goto sysfs_file_err1;
    }
    // Creating sysfs file for km_sysfs_s
    if(sysfs_create_file(kobj_ref,&km_sysfs_msg_attr.attr))
    {
         printk("Failed to create sysfs file.\n");
         goto sysfs_file_err2;
    }
 
    return 0;

sysfs_file_err2:
    sysfs_remove_file(kernel_kobj, &km_sysfs_attr.attr);

sysfs_file_err1:
    kobject_put(kobj_ref); 

sysfs_dir_err:

    return -1;
}

static void __exit end_module(void)
{
    printk(KERN_INFO "Bye-bye Kernel\n");

    kobject_put(kobj_ref); 
    sysfs_remove_file(kernel_kobj, &km_sysfs_attr.attr);

    return ;
}
module_init(start_module);
module_exit(end_module);

