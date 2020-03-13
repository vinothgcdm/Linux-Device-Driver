#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include "scull.h"

static int scull_major = 0;
static int scull_minor = 0;
static int scull_nr_devs = 1;
static struct scull_dev *scull_device;

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

	printk("Hello from scull\n");
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

	kfree(scull_device);
	cdev_del(&scull_device[0].cdev);
	unregister_chrdev_region(dev, scull_nr_devs);
	
	printk("Goodbye from scull\n");
}

int scull_open(struct inode *inode, struct file *filp)
{
	printk("scull_open\n");
	return 0;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	printk("scull_read\n");
	return 0;
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
