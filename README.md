# Linux-Device-Driver
Reading notes of Linux Device Driver by Jonathan Corbet; Alessandro Rubini; Greg Kroah-Hartman

We can download all given sample code in this book from https://resources.oreilly.com/examples/9780596005900/tree/master

# 1-scull-simple-module
```
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

# 2-scull-simple-device-driver
```
Makefile  scull.c  scull.h
[20:53]vinoth@vinoth-VBox /2-scull-simple-device-driver: $ make
make -C /lib/modules/4.15.0-88-generic/build M=/home/vinoth/Documents/Linux-Device-Driver/2-scull-simple-device-driver modules
make[1]: Entering directory '/usr/src/linux-headers-4.15.0-88-generic'
  CC [M]  /home/vinoth/Documents/Linux-Device-Driver/2-scull-simple-device-driver/scull.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/vinoth/Documents/Linux-Device-Driver/2-scull-simple-device-driver/scull.mod.o
  LD [M]  /home/vinoth/Documents/Linux-Device-Driver/2-scull-simple-device-driver/scull.ko
make[1]: Leaving directory '/usr/src/linux-headers-4.15.0-88-generic'
[20:53]vinoth@vinoth-VBox /2-scull-simple-device-driver: $ sudo insmod scull.ko && sudo dmesg -c
[  128.362339] Hello from Scull
[  128.362342] Major: 243, Minor: 0
[20:54]vinoth@vinoth-VBox /2-scull-simple-device-driver: $ grep "scull" /proc/devices 
243 scull
[20:54]vinoth@vinoth-VBox /2-scull-simple-device-driver: $ sudo mknod /dev/scull c 243 0
[20:54]vinoth@vinoth-VBox /2-scull-simple-device-driver: $ ll /dev/scull 
crw-r--r-- 1 root root 243, 0 Mac  12 20:54 /dev/scull
[20:54]vinoth@vinoth-VBox /2-scull-simple-device-driver: $ sudo su
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/2-scull-simple-device-driver# echo "hello" > /dev/scull 
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/2-scull-simple-device-driver# dmesg -c
[  175.161076] scull_open
[  175.161090] scull_write
[  175.161092] scull_release
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/2-scull-simple-device-driver# cat /dev/scull 
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/2-scull-simple-device-driver# dmesg -c
[  185.377731] scull_open
[  185.377736] scull_read
[  185.377741] scull_release
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/2-scull-simple-device-driver# exit
[20:55]vinoth@vinoth-VBox /2-scull-simple-device-driver: $
```

# 3-scull-device-implementation
```
[20:04]vinoth@vinoth-VBox /3-scull-device-implementation: $ l
Makefile  scull.c  scull.h
[20:04]vinoth@vinoth-VBox /3-scull-device-implementation: $ make
make -C /lib/modules/4.15.0-88-generic/build M=/home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation modules
make[1]: Entering directory '/usr/src/linux-headers-4.15.0-88-generic'
  CC [M]  /home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation/scull.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation/scull.mod.o
  LD [M]  /home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation/scull.ko
make[1]: Leaving directory '/usr/src/linux-headers-4.15.0-88-generic'
[20:04]vinoth@vinoth-VBox /3-scull-device-implementation: $ sudo insmod scull.ko && sudo dmesg -c
[ 1554.839775] Hello from scull
[20:04]vinoth@vinoth-VBox /3-scull-device-implementation: $ grep "scull" /proc/devices 
243 scull
[20:05]vinoth@vinoth-VBox /3-scull-device-implementation: $ sudo mknod /dev/scull c 243 0
[20:05]vinoth@vinoth-VBox /3-scull-device-implementation: $ ll /dev/scull 
crw-r--r-- 1 root root 243, 0 Mac  13 20:05 /dev/scull
[20:05]vinoth@vinoth-VBox /3-scull-device-implementation: $ sudo su
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation# echo "I am learning Linux device driver. It is so interesting." > /dev/scull root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation# head -c 10 /dev/scull && echo "" # This echo is only for newline
I am learn
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation# head -c 20 /dev/scull && echo "" # This echo is only for newline
I am learning Linux 
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation# head -c 30 /dev/scull && echo "" # This echo is only for newline
I am learning Linux device dri
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation# head -c 40 /dev/scull && echo "" # This echo is only for newline
I am learning Linux device driver. It is
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation# head -c 50 /dev/scull && echo "" # This echo is only for newline
I am learning Linux device driver. It is so intres
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation# head -c 60 /dev/scull && echo "" # This echo is only for newline
I am learning Linux device driver. It is so interesting.

root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation# head -c 100 /dev/scull && echo "" # This echo is only for newline
I am learning Linux device driver. It is so interesting.

root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3-scull-device-implementation# 
```
