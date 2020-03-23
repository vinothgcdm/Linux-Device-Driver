#ifndef __SCULL_IOCTL_H__
#define __SCULL_IOCTL_H__

/*
 * ioctl definations
 */

/* use 's' as magic number */
#define SCULL_IOC_MAGIC 'k'

/*
 * S means "Set" through a ptr
 * G means "Get" through a ptr
 * T means "Tell" directly with the argument value
 * Q means "Query" response is on return value
 * X means "eXchange": G and S
 * H means "sHift": T and Q
 */
#define SCULL_IOCRESET _IO(SCULL_IOC_MAGIC, 0)
#define SCULL_IOCSQUANTUM _IOW(SCULL_IOC_MAGIC, 1, int)
#define SCULL_IOCSQSET    _IOW(SCULL_IOC_MAGIC, 2, int)
#define SCULL_IOCTQUANTUM _IO(SCULL_IOC_MAGIC, 3)
#define SCULL_IOCTQSET    _IO(SCULL_IOC_MAGIC, 4)
#define SCULL_IOCGQUANTUM _IOR(SCULL_IOC_MAGIC, 5, int)
#define SCULL_IOCGQSET    _IOR(SCULL_IOC_MAGIC, 6, int)
#define SCULL_IOCQQUANTUM _IO(SCULL_IOC_MAGIC, 7)
#define SCULL_IOCQQSET    _IO(SCULL_IOC_MAGIC, 8)
#define SCULL_IOCXQUANTUM _IOWR(SCULL_IOC_MAGIC, 9, int)
#define SCULL_IOCXQSET    _IOWR(SCULL_IOC_MAGIC, 10, int)
#define SCULL_IOCHQUANTUM _IO(SCULL_IOC_MAGIC, 11)
#define SCULL_IOCHQSET    _IO(SCULL_IOC_MAGIC, 12)
#define SCULL_IOC_MAXNR 12

#endif
