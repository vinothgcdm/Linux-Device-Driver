#ifndef _SCULL_H_
#define _SCULL_H_

#include <linux/ioctl.h>
#include "scull_ioctl.h"

#define SCULL_QUANTUM 100
#define SCULL_QSET 10

struct scull_qset {
	void **data;
	struct scull_qset *next;
};

struct scull_dev {
	struct scull_qset *data;	/* Poninting to fist quantum set (qset) */
	int qset;					/* No.of arrays in one qset */
	int quantum;				/* Array length in qset */
	unsigned long size;			/* Amount of data stored here */
	struct cdev cdev;			/* Char device structure */
};

int scull_open(struct inode *inode, struct file *filp);
ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
int scull_release(struct inode *inode, struct file *filp);
static int scull_trim(struct scull_dev *dev);
static struct scull_qset* scull_follow(struct scull_dev *dev, int qset);
long scull_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#endif
