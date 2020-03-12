# Linux-Device-Driver
Reading notes of Linux Device Driver by Jonathan Corbet; Alessandro Rubini; Greg Kroah-Hartman

We can download all given sample code in this book from https://resources.oreilly.com/examples/9780596005900/tree/master

# 1-scull-simple-module
```bash
vinoth@vinoth-VBox:~/1-scull-simple-module$ l
main.c  Makefile
vinoth@vinoth-VBox:~/1-scull-simple-module$ sudo make
[sudo] password for vinoth: 
make -C /lib/modules/4.15.0-88-generic/build M=/home/vinoth/1-scull-simple-module modules
make[1]: Entering directory '/usr/src/linux-headers-4.15.0-88-generic'
  CC [M]  /home/vinoth/1-scull-simple-module/main.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/vinoth/1-scull-simple-module/main.mod.o
  LD [M]  /home/vinoth/1-scull-simple-module/main.ko
make[1]: Leaving directory '/usr/src/linux-headers-4.15.0-88-generic'
vinoth@vinoth-VBox:~/1-scull-simple-module$ sudo insmod main.ko && sudo dmesg -c
[12216.212852] Hello from Scull
vinoth@vinoth-VBox:~/1-scull-simple-module$ sudo rmmod main.ko && sudo dmesg -c
[12220.175599] Goodbye from Scull
vinoth@vinoth-VBox:~/1-scull-simple-module$
```
