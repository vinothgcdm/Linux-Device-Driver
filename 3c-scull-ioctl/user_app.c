#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "scull_ioctl.h"

int main(void)
{
	int quantum = 8;
	int qset = 3;
	int fd;
    int ret;

	fd = open("/dev/scull", O_RDWR);
	printf("fd: %d\n", fd);
	if (fd > 0) {
		ret = ioctl(fd, SCULL_IOCSQUANTUM, &quantum);
		printf("SCULL_IOCGQUANTUM : Ret: %d, Quantum set to %d\n", ret, quantum);

		ret = ioctl(fd, SCULL_IOCSQSET, &qset);
		printf("SCULL_IOCGQSET    : Ret: %d, Qset set to %d\n", ret, qset);

		ret = ioctl(fd, SCULL_IOCGQUANTUM, &quantum);
		printf("SCULL_IOCGQUANTUM : Ret: %d, Quantum: %d\n", ret, quantum);

		ret = ioctl(fd, SCULL_IOCGQSET, &qset);
		printf("SCULL_IOCGQSET    : Ret: %d, Qset: %d\n", ret, qset);

        ret = ioctl(fd, SCULL_IOCTQUANTUM, 10);
        printf("SCULL_IOCTQUANTUM : Ret: %d, Quantum set to 10\n", ret);

        ret = ioctl(fd, SCULL_IOCTQSET, 5);
        printf("SCULL_IOCTQSET    : Ret: %d, Qset set to 5\n", ret);

        quantum = 200;
        ret = ioctl(fd, SCULL_IOCXQUANTUM, &quantum);
        printf("SCULL_IOCXQUANTUM : Ret: %d, Exchange quantum value %d -> %d\n", ret, quantum, 200);

        qset = 20;
        ret = ioctl(fd, SCULL_IOCXQSET, &qset);
        printf("SCULL_IOCXQSET    : Ret: %d, Exchange qset value %d -> %d\n", ret, qset, 20);

        quantum = 123;
        ret = ioctl(fd, SCULL_IOCHQUANTUM, &quantum);
        printf("SCULL_IOCHQUANTUM : Ret: %d, Quantum shift with %d\n", ret, quantum);

        qset = 45;
        ret = ioctl(fd, SCULL_IOCHQSET, &qset);
        printf("SCULL_IOCHQSET    : Ret: %d, Qset shift with %d\n", ret, qset);

        ret = ioctl(fd, SCULL_IOCRESET);
        printf("SCULL_RESET       : Ret: %d\n", ret);

		ret = ioctl(fd, SCULL_IOCGQUANTUM, &quantum);
		ret = ioctl(fd, SCULL_IOCGQSET, &qset);
		printf("SCULL_IOCGQUANTUM : Ret: %d, Quantum: %d\n", ret, quantum);
		printf("SCULL_IOCGQSET    : Ret: %d, Qset: %d\n", ret, qset);
		close(fd);
	}

	return 0;
}
