# Linux-Device-Driver
Reading notes of Linux Device Driver by Jonathan Corbet; Alessandro Rubini; Greg Kroah-Hartman

We can download all given sample code in this book from https://resources.oreilly.com/examples/9780596005900/tree/master

# 1-scull-simple-module
```bash
vinoth@vinoth-VBox:~/Linux-Device-Driver/scull$ l
main.c  Makefile
vinoth@vinoth-VBox:~/Linux-Device-Driver/scull$ sudo make
make -C /lib/modules/4.15.0-88-generic/build M=/home/vinoth/Linux-Device-Driver/scull modules
make[1]: Entering directory '/usr/src/linux-headers-4.15.0-88-generic'
  CC [M]  /home/vinoth/Linux-Device-Driver/scull/main.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/vinoth/Linux-Device-Driver/scull/main.mod.o
  LD [M]  /home/vinoth/Linux-Device-Driver/scull/main.ko
make[1]: Leaving directory '/usr/src/linux-headers-4.15.0-88-generic'
vinoth@vinoth-VBox:~/Linux-Device-Driver/scull$ sudo insmod main.ko && sudo dmesg -c
[10547.535638] Hello from Scull
vinoth@vinoth-VBox:~/Linux-Device-Driver/scull$ sudo rmmod main.ko && sudo dmesg -c
[10551.089210] Goodbye from Scull
vinoth@vinoth-VBox:~/Linux-Device-Driver/scull$
```
