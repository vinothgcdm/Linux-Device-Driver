#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include "scull.h"

static int scull_major = 0;
static int scull_minor = 0;
static int scull_nr_devs = 1;
static struct cdev cdev;

static struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .open = scull_open,
    .release = scull_release,
    .read = scull_read,
    .write = scull_write
};

static int scull_init(void)
{
	dev_t dev = 0;
	int ret = 0;

	printk("Hello from Scull\n");
	ret = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
	if (ret < 0) {
		printk("Allocating device number is failed.\n");
		return ret;
	}

	scull_major = MAJOR(dev);
	scull_minor = MINOR(dev);
	printk("Major: %d, Minor: %d\n", scull_major, scull_minor);

	cdev_init(&cdev, &scull_fops);
	ret = cdev_add(&cdev, dev, 1);
	if (ret < 0) {
		unregister_chrdev_region(dev, scull_nr_devs);
		printk("Add scull device into kernel failed.\n");
		return ret;
	}

	return 0;
}

static void scull_exit(void)
{
	dev_t dev = MKDEV(scull_major, scull_minor);

	cdev_del(&cdev);
	unregister_chrdev_region(dev, scull_nr_devs);
	printk("Goodbye from Scull\n");
}

int scull_open(struct inode *inode, struct file *filp)
{
	printk("scull_open\n");
	return 0;
}

/*
 * Intentionally this function return 1. We expect these prints will keep printing on consol.
 *
 * The behaviour of @printk_ratelimit can be costomized by modifying /proc/sys/kernel/printk_ratelimit
 * (the no.of seonds wait before re-enabling message) and /proc/sys/kernel/printk_ratelimit_burst
 * (the no.of messages accepted before ratelimit.
 */
ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	if (printk_ratelimit()) {
		printk("scull_read-1\n");
	}
	if (printk_ratelimit()) {
		printk("scull_read-2\n");
	}

	return 1;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	printk("scull_write\n");
	return count;
}

int scull_release(struct inode *inode, struct file *filp)
{
	printk("scull_release\n");
	return 0;
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(scull_init);
module_exit(scull_exit);
