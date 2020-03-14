#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include "scull.h"

static int scull_major = 0;
static int scull_minor = 0;
static int scull_nr_devs = 1;
static int scull_quantum = 100;
static int scull_qset = 10;
static struct scull_dev *scull_device;

static struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .open = scull_open,
    .release = scull_release,
    .read = scull_read,
    .write = scull_write
};

static int scull_trim(struct scull_dev *dev)
{
    int i;
    struct scull_qset *delete_ptr;
    struct scull_qset *next_ptr;

    if ((dev == NULL) || (dev->data == NULL)) {
        return -1;
    }

    for (delete_ptr = dev->data; delete_ptr; delete_ptr = next_ptr) {
        if (delete_ptr->data) {
            for (i = 0; i < dev->qset; i++) {
                kfree(delete_ptr->data + i);
            }
        }
        next_ptr = delete_ptr->next;
        kfree(delete_ptr);
    }
    dev->size = 0;
    dev->data = NULL;

    return 0;
}

static int scull_init(void)
{
    dev_t dev = 0;
    int ret = 0;

    PDEBUG("scull_init\n");
    ret = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
    if (ret < 0) {
        printk("Allocating device number is failed.\n");
        return ret;
    } else {
        scull_major = MAJOR(dev);
        scull_minor = MINOR(dev);
    }

    scull_device = (struct scull_dev*)kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);
    if (scull_device == NULL) {
        printk("kmalloc failed to allocate for scull_device\n");
        goto fail;
    } else {
        memset(scull_device, 0, scull_nr_devs * sizeof(struct scull_dev));
        scull_device[0].qset = scull_qset;
        scull_device[0].quantum = scull_quantum;
    }

    cdev_init(&scull_device[0].cdev, &scull_fops);
    ret = cdev_add(&scull_device[0].cdev, dev, 1);
    if (ret < 0) {
        printk("Add scull device into kernel failed.\n");
        goto fail;
    }

    return 0;
fail:
    unregister_chrdev_region(dev, scull_nr_devs);
    return ret;
}

static void scull_exit(void)
{
    dev_t dev = MKDEV(scull_major, scull_minor);

    cdev_del(&scull_device[0].cdev);
    unregister_chrdev_region(dev, scull_nr_devs);
    scull_trim(scull_device);
    kfree(scull_device);
    
    PDEBUG("scull_exit\n");
}

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev = NULL;

    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    PDEBUG("scull_open\n");
    return 0;
}

static struct scull_qset* scull_follow(struct scull_dev *dev, int qset)
{
    struct scull_qset *ptr = dev->data;

    if (ptr == NULL) {
        /* allocate memory for head quantum set */
        ptr = (struct scull_qset*)kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if (ptr == NULL) {
            return NULL;
        }
        dev->data = ptr;
        memset(ptr, 0, sizeof(struct scull_qset));
    }

    while (qset--) {
        if (ptr->next == NULL) {
            /* allocate memory for next quantum set */
            ptr->next = (struct scull_qset*)kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
            if (ptr->next == NULL) {
                return NULL;
            }
            memset(ptr, 0, sizeof(struct scull_qset));
        }
        ptr = ptr->next;
    }

    return ptr;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int quantum;
    int qset;
    int qset_size;
    int source_quantum;
    int source_quantum_pos;
    int source_qset;
    int retval = 0;
    struct scull_dev *dev = (struct scull_dev*)filp->private_data;
    struct scull_qset *ptr = NULL;

    PDEBUG("scull_read\n");
    if (dev == NULL) {
        goto out;
    }
    if ((*f_pos >= dev->size) || (dev->size == 0)) {
        goto out;
    }

    quantum = dev->quantum;
    qset = dev->qset;
    qset_size = quantum * qset;
    source_qset = (long)*f_pos / qset_size;
    source_quantum = ((long)*f_pos % qset_size) / quantum;
    source_quantum_pos = ((long)*f_pos % qset_size) % quantum;

    /* follow the list upto right position */
    ptr = scull_follow(dev, source_qset);
    if ((ptr == NULL) || (ptr->data == NULL) || (ptr->data[source_quantum] == NULL)) {
        goto out;
    }

    /* read only upto end of the quantum */
    if (count > (quantum - source_quantum_pos)) {
        count = quantum - source_quantum_pos;
    }

    if (copy_to_user(buf, ptr->data[source_quantum] + source_quantum_pos, count)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

out:
    return retval;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    int quantum;
    int qset;
    int qset_size;
    int dest_quantum;
    int dest_quantum_pos;
    int dest_qset;
    int retval = 0;
    struct scull_dev *dev = (struct scull_dev*)filp->private_data;
    struct scull_qset *ptr = NULL;

    PDEBUG("scull_write\n");
    quantum = dev->quantum;
    qset = dev->qset;
    qset_size = quantum * qset;
    dest_qset = (long)*f_pos / qset_size;
    dest_quantum = ((long)*f_pos % qset_size) / quantum;
    dest_quantum_pos = ((long)*f_pos % qset_size) % quantum;

    /* follow the quantum_set in list upto right position */
    ptr = scull_follow(dev, dest_qset);
    if (ptr == NULL) {
        goto out;
    }

    if (ptr->data == NULL) {
        /* allocate memory for array of quantum pointers (qset) */
        ptr->data = kmalloc(qset * sizeof(char*), GFP_KERNEL);
        if (ptr->data == NULL) {
            goto out;
        }
        memset(ptr->data, 0, qset * sizeof(char*));
    }

    if (ptr->data[dest_quantum] == NULL) {
        /* allocate memory for quantum array */
        ptr->data[dest_quantum] = kmalloc(quantum, GFP_KERNEL);
        if (ptr->data[dest_quantum] == NULL) {
            goto out;
        }
    }

    if (count > (quantum - dest_quantum_pos)) {
        count = quantum - dest_quantum_pos;
    }

    if (copy_from_user(ptr->data[dest_quantum] + dest_quantum_pos, buf, count)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

    if (dev->size < *f_pos) {
        dev->size = *f_pos;
    }

out:
    return retval;
}

int scull_release(struct inode *inode, struct file *filp)
{
    PDEBUG("scull_release\n");
    return 0;
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(scull_init);
module_exit(scull_exit);
