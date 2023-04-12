// This RAM disk driver handles Multi-queue I/O request.
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/buffer_head.h>
#include <linux/hdreg.h>
#include <linux/blk-mq.h> // For multi-queue I/O request
 
MODULE_LICENSE("GPL");

struct my_blk_dev
{
     int size;                       // Disk size in units of bytes 
     u8 *data;                       // Data buffer for emulating RAM Disk   
     struct blk_mq_tag_set tag_set;  // For multi-queue properties/operation 
     struct request_queue *queue;    // I/O request queue for the disk
     struct gendisk *gd;             // gendisk structure
};

// Constant definitions
#define KERNEL_SECTOR_SIZE	512	// Kernel sector size
#define SECTOR_SHIFT	9	
/*
 * One minor number is used by the disk device itself.
 * Each partition of the disk requires a separate minor number as each partition * is treated as a separate device having its own device file. Therefore, 
 * 3 minor numbers aare required for a disk with two partitions.
 */
#define DRV_BLK_MINORS	3

// Let the system allocate the major number for this device type/driver.
static unsigned int blk_major = 0;
static struct my_blk_dev *bdev = NULL;
static int nsectors = 800; // Number of sectors for 400KB RAM disk
static int hardsect_size = 512;	// Hardware/Disk sector size

static int drv_blk_open (struct block_device *bd, fmode_t fmode)
{
    printk(KERN_INFO "drv_blk_open()\n");

    return 0;
} 

static void drv_blk_release (struct gendisk *gd, fmode_t fmode)
{
    printk(KERN_INFO "drv_blk_release()\n");

    return ;
}

static int drv_blk_ioctl (struct block_device *bd, fmode_t fmode, unsigned int cmd, unsigned long arg)
{
    printk(KERN_INFO "drv_blk_ioctl()\n");

    return -ENOTTY; /* unknown IOCTL command */
}

// Process the I/O requests.
static int process_io_request (struct request *rq, unsigned int *nr_bytes)
{
    int ret = 0;
    struct bio_vec bvec;
    struct req_iterator iter;
    struct my_blk_dev *bdev = rq->q->queuedata;
    loff_t pos = blk_rq_pos(rq) << SECTOR_SHIFT;
    loff_t dev_size = (loff_t)(bdev->size << SECTOR_SHIFT);

    printk(KERN_INFO "process_io_request()\n");
    
    rq_for_each_segment(bvec, rq, iter)
    {
        unsigned long b_len = bvec.bv_len;

        void* b_buf = page_address(bvec.bv_page) + bvec.bv_offset;

        if ((pos + b_len) > dev_size)
            b_len = (unsigned long)(dev_size - pos);

        if (rq_data_dir(rq))	// WRITE request
            memcpy(bdev->data + pos, b_buf, b_len); // Write into the RAM disk
        else	// READ request
            memcpy(b_buf, bdev->data + pos, b_len); // Read from the RAM disk

        pos += b_len;
        *nr_bytes += b_len;
    }

    return ret; 
}

// Process I/O request queue
static blk_status_t process_io_request_queue (struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bmqd)
{
    unsigned int nr_bytes = 0;
    blk_status_t status = BLK_STS_OK;
    struct request *rq = bmqd->rq; 

    printk(KERN_INFO "process_io_request_queue()\n");
   
    // Start I/O request serving procedure.
    blk_mq_start_request (rq);

    // Process the I/O requests.
    if (process_io_request (rq, &nr_bytes) != 0)
        status = BLK_STS_IOERR;

    // Notify the kernel about processed number of bytes (nr_bytes).
    if (blk_update_request (rq, status, nr_bytes))
        status = BLK_STS_IOERR;

    // Stop I/O request serving procedure.
    __blk_mq_end_request (rq, status);

    return status;
}

static struct blk_mq_ops bmq_ops = {
    .queue_rq = process_io_request_queue
};

static struct block_device_operations blk_dev_ops = {
    .owner = THIS_MODULE,
    .open = drv_blk_open,
    .release = drv_blk_release,
    .ioctl = drv_blk_ioctl
};

static int __init drv_blk_init (void)
{
        int ret;

        printk(KERN_INFO "Hello Kernel\n");

        blk_major = register_blkdev(blk_major, "cgrd");
        if (blk_major <= 0)
        {
                printk(KERN_ERR "Can not allocate major number for device.\n");
                return -EBUSY;
        }
        printk(KERN_INFO "Major Num = %d\n", blk_major);
 
        // Allocate memory for holding driver related data/info. 
        bdev = (struct my_blk_dev *) kmalloc(sizeof (struct my_blk_dev), GFP_KERNEL);
        if (bdev == NULL)
        {
            printk (KERN_ERR "kmalloc for my_blk_dev failed.\n");
            goto blk_dev_error; 
        }
        memset (bdev, 0, sizeof (struct my_blk_dev));
        bdev->size = nsectors*hardsect_size;

        // Allocate memory for RAM disk data buffer. 
        bdev->data = vmalloc(bdev->size);
        if (bdev->data == NULL)
        {
            printk (KERN_ERR "vmalloc for data buffer failed.\n");
            goto blk_dev_data_error; 
        }

        // Initialize tag-set for defining multi-queue I/O request. 
        memset(&(bdev->tag_set), 0, sizeof(bdev->tag_set));
        bdev->tag_set.ops = &bmq_ops;
        bdev->tag_set.nr_hw_queues = 1;
        bdev->tag_set.queue_depth = 128;
        bdev->tag_set.numa_node = NUMA_NO_NODE;
        bdev->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
        bdev->tag_set.driver_data = bdev;
        ret = blk_mq_alloc_tag_set (&(bdev->tag_set));
        if (ret) {
            printk (KERN_ERR "Unable to allocate tag set for multi-queue.\n");
            goto blk_dev_tag_set_error; 
        }

        // Initialize multi-queue for I/O request.
        bdev->queue = blk_mq_init_queue (&(bdev->tag_set));

#if 0
        /*
         * Some kernel versions have following single function for allocating
         * tag-set and initializing multi-queue for I/O requests.
         */
        
        // bdev->queue = blk_mq_init_sq_queue(&bdev->tag_set, &bmq_ops, 128,
						 BLK_MQ_F_SHOULD_MERGE);
#endif

        if (bdev->queue == NULL)
        {
            printk (KERN_ERR "Failed to create and initiaize I/O request queue.\n");
            goto blk_dev_queue_error; 
        }
        bdev->queue->queuedata = bdev;

        bdev->gd = alloc_disk (DRV_BLK_MINORS);
        if (bdev->gd == NULL)
        {
            printk (KERN_ERR "alloc_disk failed\n");
            goto alloc_disk_error;
        }
        bdev->gd->major = blk_major;
        bdev->gd->first_minor = 0;
        bdev->gd->fops = &blk_dev_ops;
        bdev->gd->queue = bdev->queue;
        bdev->gd->private_data = bdev;
        snprintf (bdev->gd->disk_name, 32, "cgrd%c", 'b');
        set_capacity(bdev->gd, nsectors*(hardsect_size/KERNEL_SECTOR_SIZE));
        add_disk(bdev->gd);

        printk(KERN_INFO "Initialization successful.\n");
        return 0;

alloc_disk_error:
        blk_cleanup_queue(bdev->queue);
blk_dev_queue_error:
        blk_mq_free_tag_set (&(bdev-> tag_set));
blk_dev_tag_set_error:
        vfree(bdev->data);
blk_dev_data_error: 
        kfree(bdev);
blk_dev_error: 
        unregister_blkdev(blk_major, "cgrd");

        return -1;
}
 
static void __exit drv_blk_exit (void)
{
        del_gendisk(bdev->gd); 
        put_disk(bdev->gd); 
        blk_cleanup_queue(bdev->queue);
        blk_mq_free_tag_set (&(bdev-> tag_set));
        vfree(bdev->data);
        kfree(bdev);
        unregister_blkdev(blk_major, "cgrd");

        printk(KERN_INFO "Bye-bye Kernel\n");
        return ;
}
 
module_init(drv_blk_init);
module_exit(drv_blk_exit);
