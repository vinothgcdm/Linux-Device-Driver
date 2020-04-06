#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>     // kmalloc()
#include <linux/uaccess.h>  // copy_to_user() copy_from_user()
#include <linux/wait.h>     // waitqueue
#include <linux/sched/signal.h> // signal_pending()

#define SCULLP_DEBUG
#ifdef SCULLP_DEBUG
    #define PDEBUG(fmt, arg...) printk(KERN_DEBUG fmt, ## arg)
#else
    #define PDEBUG(fmt, arg...) // do nothing
#endif
#define DPDEBUG(fmt, arg...) // do nothing

struct scull_pipe {
    wait_queue_head_t inq;
    wait_queue_head_t outq;
    char *buf_start;
    char *buf_end;
    char *rp;
    char *wp;
    int buf_size;
    int buf_free_space;
    struct cdev cdev;
};

static int scullp_major = 0;
static int scullp_minor = 0;
static int scullp_nr_devs = 1;
static int scullp_buf_size = 1024;
static struct scull_pipe *scullp_dev = NULL;

static int scullp_open(struct inode *inode, struct file *filp)
{
    struct scull_pipe *dev;

    PDEBUG("scullp_open\n");
    dev = container_of(inode->i_cdev, struct scull_pipe, cdev);
    filp->private_data = dev;

    return 0;
}

static ssize_t scullp_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_pipe *dev = filp->private_data;
    wait_queue_entry_t wait;

    PDEBUG("scullp_read: start: free_space: %d, count: %ld, f_pos: %lld, dev: %px, wp: %px, rp: %px \n",
            dev->buf_free_space, count, *f_pos, dev, dev->wp, dev->rp);

    /* sleep if no data available */
    while (dev->buf_free_space == dev->buf_size) {
        PDEBUG("scullp_read: [%d] No data available, going to sleep...\n", current->pid);
        init_wait(&wait);
        prepare_to_wait(&dev->inq, &wait, TASK_INTERRUPTIBLE);
        if (dev->buf_free_space == dev->buf_size) {
            schedule();
        }
        finish_wait(&dev->inq, &wait);
        if (signal_pending(current)) {
            return -ERESTART;
        }
    }
   
    /* read upto the buffer boundary or upto write pointer */
    if ((dev->rp > dev->wp)  ||                                     // buf has some data and it is not full
        ((dev->rp == dev->wp) && (dev->buf_free_space == 0))) {     // buf is full with data, no free space
        count = min((size_t)(dev->buf_end - dev->rp), count);
    } else {
        count = min((size_t)(dev->wp - dev->rp), count);
    }
    if (copy_to_user(buf, dev->rp, count)) {
        return -EFAULT;
    }

    dev->buf_free_space += count;
    dev->rp += count;
    if (dev->rp == dev->buf_end) {
        dev->rp = dev->buf_start;
    }

    PDEBUG("scullp_read:   end: free_space: %d, count: %ld, f_pos: %lld, dev: %px, wp: %px, rp: %px \n",
            dev->buf_free_space, count, *f_pos, dev, dev->wp, dev->rp);
    wake_up_interruptible(&dev->outq);

    return count;
}

static ssize_t scullp_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_pipe *dev = filp->private_data;
    wait_queue_entry_t wait;

    PDEBUG("scullp_write: start: free_space: %d, count: %ld, f_pos: %lld, dev: %px, wp: %px, rp: %px\n",
            dev->buf_free_space, count, *f_pos, dev, dev->wp, dev->rp);

    /* slepp if no space available to write */
    while (dev->buf_free_space == 0) {
        PDEBUG("scullp_write: [%d] No space to write, going to sleep...\n", current->pid);
        init_wait(&wait);
        prepare_to_wait(&dev->outq, &wait, TASK_INTERRUPTIBLE);
        if (dev->buf_free_space == 0) {
            schedule();
        }
        finish_wait(&dev->outq, &wait);
        if (signal_pending(current)) {
            return -ERESTART;
        }
    }

    if (dev->buf_free_space <= 0) {
        printk("scullp_write: No space available\n");
        return -EAGAIN;
    }

    if (dev->wp >= dev->rp) {
        count = min((size_t)(dev->buf_end - dev->wp), count);
    } else {
        count = min((size_t)(dev->rp - dev->wp), count);
    }
    if (copy_from_user(dev->wp, buf, count)) {
        return -EFAULT;
    }

    dev->buf_free_space -= count;
    dev->wp += count;
    if (dev->wp == dev->buf_end) {
        dev->wp = dev->buf_start;
    }

    PDEBUG("scullp_write:   end: free_space: %d, count: %ld, f_pos: %lld, dev: %px, wp: %px, rp: %px\n",
            dev->buf_free_space, count, *f_pos, dev, dev->wp, dev->rp);
    wake_up_interruptible(&dev->inq);

    return count;
}

static int scullp_release(struct inode *inode, struct file *filp)
{
    PDEBUG("scullp_release\n");
    return 0;
}

static struct file_operations scullp_fops = {
    .owner = THIS_MODULE,
    .open = scullp_open,
    .release = scullp_release,
    .read = scullp_read,
    .write = scullp_write
};

static int scullp_init(void)
{
    dev_t dev = 0;
    int ret = 0;

    PDEBUG("scullp_init: Hello from Scull\n");
    ret = alloc_chrdev_region(&dev, scullp_minor, scullp_nr_devs, "scullp");
    if (ret < 0) {
        printk("scullp_init: Allocating device number is failed.\n");
        return ret;
    }

    scullp_major = MAJOR(dev);
    scullp_minor = MINOR(dev);
    PDEBUG("scullp_init: Major: %d, Minor: %d\n", scullp_major, scullp_minor);

    scullp_dev = kmalloc(sizeof(struct scull_pipe), GFP_KERNEL);
    if (scullp_dev == NULL) {
        printk("scullp_init: kmalloc failed\n");
        unregister_chrdev_region(dev, scullp_nr_devs);
    }

    /* init scull pipe device */
    scullp_dev->buf_start = kmalloc(scullp_buf_size, GFP_KERNEL);
    if (scullp_dev->buf_start == NULL) {
        printk("scullp_init: kmalloc failed to scull pipe device\n");
        return -ENOMEM;
    }
    memset(scullp_dev->buf_start, 0, scullp_buf_size);
    /* pointing to next cell of the boundary */
    scullp_dev->buf_end = scullp_dev->buf_start + scullp_buf_size;
    scullp_dev->rp = scullp_dev->buf_start;
    scullp_dev->wp = scullp_dev->buf_start;
    scullp_dev->buf_size = scullp_buf_size;
    scullp_dev->buf_free_space = scullp_buf_size;

    /* init read/write waitqueue */
    init_waitqueue_head(&scullp_dev->inq);
    init_waitqueue_head(&scullp_dev->outq);

    /* init and create char device */
    cdev_init(&(scullp_dev->cdev), &scullp_fops);
    ret = cdev_add(&scullp_dev->cdev, dev, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev, scullp_nr_devs);
        printk("scullp_init: Add scull device into kernel failed.\n");
        return ret;
    }

    return 0;
}

static void scullp_exit(void)
{
    dev_t dev = MKDEV(scullp_major, scullp_minor);

    cdev_del(&scullp_dev->cdev);
    unregister_chrdev_region(dev, scullp_nr_devs);
    kfree(scullp_dev->buf_start);
    kfree(scullp_dev);

    PDEBUG("scullp_exit: Goodbye from Scull\n");
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(scullp_init);
module_exit(scullp_exit);
