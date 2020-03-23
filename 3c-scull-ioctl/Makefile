KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	@rm -fr *.o *~ *.ko *.mod.c *.order *.symvers *.out
	@echo "Build directory is cleaned"

.PHONY: all clean

obj-m := scull.o
