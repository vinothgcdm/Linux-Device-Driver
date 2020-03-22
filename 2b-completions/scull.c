#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/completion.h>
#include <linux/uaccess.h>
#include "scull.h"

static int scull_major = 0;
static int scull_minor = 0;
static int scull_nr_devs = 1;
static struct cdev cdev;
static char kbuf[100];

static struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .open = scull_open,
    .release = scull_release,
    .read = scull_read,
    .write = scull_write
};

DECLARE_COMPLETION(comp);

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

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	int len = (count > sizeof(kbuf)) ? sizeof(kbuf) : count;

	if (sizeof(kbuf) <= *f_pos)
		return 0;

	printk("scull_read: process %i (%s) going to sleep.\n",
		current->pid, current->comm);
	wait_for_completion(&comp);
	printk("scull_read: awaken %i (%s).\n", current->pid, current->comm);
	if (printk_ratelimit()) {
		printk("scull_read: f_pos: %lu, count: %lu\n", *((long*)f_pos), count);
	}

	copy_to_user(buf, kbuf, len);
	*f_pos = len;

	return len;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	int len = (count > sizeof(kbuf)) ? sizeof(kbuf) : count;

	printk("scull_write: process %i (%s) awaking the readers...\n",
		current->pid, current->comm);
	copy_from_user(kbuf, buf, len);
	//complete(&comp);
	complete_all(&comp);
	return len;
}

int scull_release(struct inode *inode, struct file *filp)
{
	printk("scull_release\n");
	return 0;
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(scull_init);
module_exit(scull_exit);
