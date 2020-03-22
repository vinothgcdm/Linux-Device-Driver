#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "scull.h"

static int scull_major = 0;
static int scull_minor = 0;
static int scull_nr_devs = 1;
static int scull_quantum = 10;
static int scull_qset = 5;
static struct scull_dev *scull_device;

static struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .open = scull_open,
    .release = scull_release,
    .read = scull_read,
    .write = scull_write
};

/* ---------- Start create_proc_entry ---------- */
static void *scull_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos >= scull_nr_devs) {
        return NULL;
    }
    return scull_device;
}

static int scull_seq_show(struct seq_file *s, void *v)
{
    int i;
    int j;
    struct scull_dev *dev = (struct scull_dev*)v;
    struct scull_qset *d;

    if (dev == NULL) {
        return 0;
    }

    seq_printf(s, "qset: %d[%p], quantum: %d, size: %lu\n",
               dev->qset, dev->data, dev->quantum, dev->size);
    /* iterate quantum set */
    for (d = dev->data; d; d = d->next) {
        seq_printf(s, "qset[%p, %p(next)]:\n", d, d->next);
        /* iterate quantum pointers */
        for (i = 0; d->data[i] && (i < dev->qset); i++) {
            seq_printf(s, "Quantum[%d, %p, %p(next)]  ",
                       i, d->data[i], ((i + 1) < dev->qset) ? (d->data[i + 1]) : NULL);
            /* print quantum data */
            for (j = 0; j < dev->quantum; j++) {
                char ch = *((char*)(d->data[i]) + j);
                if (((ch >= 'a') && (ch <= 'z')) ||
                    ((ch >= 'A') && (ch <= 'Z')) ||
                    (ch == '#')) {
                    seq_printf(s, "%c", ch);
                } else {
                    seq_printf(s, ".");
                }
            }
            seq_printf(s, "\n");
        }
        seq_printf(s, "\n");
    }

    return 0;
}

static void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    if (*pos >= scull_nr_devs) {
        return NULL;
    }
    return scull_device;
}

static void scull_seq_stop(struct seq_file *s, void *v)
{
}

static struct seq_operations scull_seq_ops = {
    .start = scull_seq_start,
    .next = scull_seq_next,
    .show = scull_seq_show,
    .stop = scull_seq_stop
};

static int scull_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &scull_seq_ops);
}

static const struct file_operations scull_proc_ops = {
    .owner = THIS_MODULE,
    .open = scull_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};
/* ---------- End create_proc_entry ---------- */

static int scull_trim(struct scull_dev *dev)
{
    int i;
    int j;
    struct scull_qset *ptr;
    struct scull_qset *next_ptr;

    if ((dev == NULL) || (dev->data == NULL)) {
        return -1;
    }

    /* iterate quantum set */
    for (j = 0, ptr = dev->data; ptr; j++, ptr = next_ptr) {
        if (ptr->data) {
            PPDEBUG("scull_trim: qset[%d]: %p\n", j, ptr);
            /* iterate quantum pointers */
            for (i = 0; i < dev->qset; i++) {
                if (ptr->data[i] == NULL) {
                    break;
                }
                PPDEBUG("scull_trim: quantum[%d]: %p\n", i, ptr->data[i]);
                kfree(ptr->data[i]);
            }
        }
        next_ptr = ptr->next;
        kfree(ptr);
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
    }
    scull_major = MAJOR(dev);
    scull_minor = MINOR(dev);

    scull_device = (struct scull_dev*)kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);
    if (scull_device == NULL) {
        printk("kmalloc failed to allocate for scull_device\n");
        goto fail;
    }
    memset(scull_device, 0, scull_nr_devs * sizeof(struct scull_dev));
    scull_device[0].qset = scull_qset;
    scull_device[0].quantum = scull_quantum;

    cdev_init(&scull_device[0].cdev, &scull_fops);
    ret = cdev_add(&scull_device[0].cdev, dev, 1);
    if (ret < 0) {
        printk("Add scull device into kernel failed.\n");
        goto fail;
    }

    proc_create("scullmem", 0, NULL, &scull_proc_ops);
    return 0;

fail:
    unregister_chrdev_region(dev, scull_nr_devs);
    return ret;
}

static void scull_exit(void)
{
    dev_t dev = MKDEV(scull_major, scull_minor);

    remove_proc_entry("scullmem", NULL);
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
            PDEBUG("scull_follow: malloc failed to create head qset.");
            return NULL;
        }
        dev->data = ptr;
        memset(ptr, 0, sizeof(struct scull_qset));
    }

    PPDEBUG("scull_follow: ptr: %p\n", ptr);
    while (qset--) {
        if (ptr->next == NULL) {
            /* allocate memory for next quantum set */
            ptr->next = (struct scull_qset*)kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
            if (ptr->next == NULL) {
                PDEBUG("scull_follow: malloc failed to create next qset.");
                return NULL;
            }
            memset(ptr->next, 0, sizeof(struct scull_qset));
        }
        ptr = ptr->next;
        PPDEBUG("scull_follow: ptr->next: %p\n", ptr);
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
    PDEBUG("scull_write: dest_qset: %d, dest_quantum: %d, dest_quantum_pos: %d, f_pos: %lu\n",
            dest_qset, dest_quantum, dest_quantum_pos, *((long*)f_pos));

    /* follow the quantum_set in list upto right position */
    ptr = scull_follow(dev, dest_qset);
    if (ptr == NULL) {
        PDEBUG("scull_write: scull_follow failed\n");
        goto out;
    }

    if (ptr->data == NULL) {
        /* allocate memory for array of quantum pointers (qset) */
        ptr->data = kmalloc(qset * sizeof(char*), GFP_KERNEL);
        if (ptr->data == NULL) {
            PDEBUG("scull_write: malloc failed to create quantum pointers.\n");
            goto out;
        }
        memset(ptr->data, 0, qset * sizeof(char*));
    }

    if (ptr->data[dest_quantum] == NULL) {
        /* allocate memory for quantum array */
        ptr->data[dest_quantum] = kmalloc(quantum, GFP_KERNEL);
        if (ptr->data[dest_quantum] == NULL) {
            PDEBUG("scull_write: malloc failed to create quantum array.\n");
            goto out;
        }
        memset(ptr->data[dest_quantum], '#', quantum);
    }

    if (count > (quantum - dest_quantum_pos)) {
        count = quantum - dest_quantum_pos;
    }

    if (copy_from_user(ptr->data[dest_quantum] + dest_quantum_pos, buf, count)) {
        PDEBUG("scull_write: copy_from_user failed.\n");
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
