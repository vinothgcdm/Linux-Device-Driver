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

# 3b-scull-add-proc-entry modules
```
[15:21]vinoth@vinoth-VBox /5-scull-add-proc-entry: $ make
make -C /lib/modules/4.15.0-88-generic/build M=/home/vinoth/Documents/Linux-Device-Driver/5-scull-add-proc-entry modules
make[1]: Entering directory '/usr/src/linux-headers-4.15.0-88-generic'
  CC [M]  /home/vinoth/Documents/Linux-Device-Driver/5-scull-add-proc-entry/scull.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/vinoth/Documents/Linux-Device-Driver/5-scull-add-proc-entry/scull.mod.o
  LD [M]  /home/vinoth/Documents/Linux-Device-Driver/5-scull-add-proc-entry/scull.ko
make[1]: Leaving directory '/usr/src/linux-headers-4.15.0-88-generic'
[15:21]vinoth@vinoth-VBox /5-scull-add-proc-entry: $ l
Makefile  scull.c  scull.mod.c  scull.h  scull.ko  scull.mod.o  scull.o  modules.order  Module.symvers
[15:21]vinoth@vinoth-VBox /5-scull-add-proc-entry: $ sudo insmod scull.ko && sudo dmesg -c
[15:22]vinoth@vinoth-VBox /5-scull-add-proc-entry: $ grep "scull" /proc/devices 
243 scull
[15:22]vinoth@vinoth-VBox /5-scull-add-proc-entry: $ sudo mknod /dev/scull c 243 0
[15:22]vinoth@vinoth-VBox /5-scull-add-proc-entry: $ sudo su
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/5-scull-add-proc-entry# cat scull.c > /dev/scull 
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/5-scull-add-proc-entry# exit
[15:22]vinoth@vinoth-VBox /5-scull-add-proc-entry: $ sudo cat /proc/scullmem | head
qset: 5[000000002db5c954], quantum: 10, size: 9837
qset[000000002db5c954, 0000000049e39773(next)]:
Quantum[0, 00000000bc065401, 000000001ff78cf2(next)]  #include..
Quantum[1, 000000001ff78cf2, 000000006cb775cb(next)]  linux.init
Quantum[2, 000000006cb775cb, 00000000f50ea53c(next)]  .h..#inclu
Quantum[3, 00000000f50ea53c, 00000000da1c33ae(next)]  de..linux.
Quantum[4, 00000000da1c33ae,           (null)(next)]  module.h..

qset[0000000049e39773, 000000003ca47a9d(next)]:
Quantum[0, 000000002cc749c5, 000000000c4ad1f0(next)]  #include..
[15:22]vinoth@vinoth-VBox /5-scull-add-proc-entry: $ sudo cat /proc/scullmem | tail
Quantum[2, 00000000ed49298c, 00000000a786e671(next)]  .Dual.BSD.
Quantum[3, 00000000a786e671, 00000000775acb50(next)]  GPL....mod
Quantum[4, 00000000775acb50,           (null)(next)]  ule.init.s

qset[00000000c14ad569,           (null)(next)]:
Quantum[0, 000000003e96591e, 000000001421ee7d(next)]  cull.init.
Quantum[1, 000000001421ee7d, 00000000811ef17c(next)]  ..module.e
Quantum[2, 00000000811ef17c, 00000000b2050be9(next)]  xit.scull.
Quantum[3, 00000000b2050be9,           (null)(next)]  exit...###

[15:22]vinoth@vinoth-VBox /5-scull-add-proc-entry: $ sudo rmmod scull.ko && sudo dmesg -c
[15:23]vinoth@vinoth-VBox /5-scull-add-proc-entry: $ 
```

# 3c-scull-ioctl
```
[12:30]vinoth@vinoth-VBox /3c-scull-ioctl: $ l
Makefile  scull.c  user_app.c  scull.h  scull_ioctl.h
[12:30]vinoth@vinoth-VBox /3c-scull-ioctl: $ make
make -C /lib/modules/4.15.0-91-generic/build M=/home/vinoth/Documents/Linux-Device-Driver/3c-scull-ioctl modules
make[1]: Entering directory '/usr/src/linux-headers-4.15.0-91-generic'
  CC [M]  /home/vinoth/Documents/Linux-Device-Driver/3c-scull-ioctl/scull.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/vinoth/Documents/Linux-Device-Driver/3c-scull-ioctl/scull.mod.o
  LD [M]  /home/vinoth/Documents/Linux-Device-Driver/3c-scull-ioctl/scull.ko
make[1]: Leaving directory '/usr/src/linux-headers-4.15.0-91-generic'
[12:30]vinoth@vinoth-VBox /3c-scull-ioctl: $ l
Makefile  scull.c  scull.mod.c  user_app.c  scull.h  scull_ioctl.h  scull.ko  scull.mod.o  scull.o  modules.order  a.out*  Module.symvers
[12:30]vinoth@vinoth-VBox /3c-scull-ioctl: $ sudo insmod scull.ko && sudo dmesg -c
[10818.422374] Hello from scull
[12:30]vinoth@vinoth-VBox /3c-scull-ioctl: $ grep "scull" /proc/devices 
243 scull
[12:31]vinoth@vinoth-VBox /3c-scull-ioctl: $ sudo mknod /dev/scull c 243 0
[12:31]vinoth@vinoth-VBox /3c-scull-ioctl: $ ll /dev/scull 
crw-r--r-- 1 root root 243, 0 Mac  23 12:31 /dev/scull
[12:31]vinoth@vinoth-VBox /3c-scull-ioctl: $ sudo su
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3c-scull-ioctl# gcc user_app.c 
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3c-scull-ioctl# ./a.out 
fd: 3
SCULL_IOCGQUANTUM : Ret: 0, Quantum set to 8
SCULL_IOCGQSET    : Ret: 0, Qset set to 3
SCULL_IOCGQUANTUM : Ret: 0, Quantum: 8
SCULL_IOCGQSET    : Ret: 0, Qset: 3
SCULL_IOCTQUANTUM : Ret: 0, Quantum set to 10
SCULL_IOCTQSET    : Ret: 0, Qset set to 5
SCULL_IOCXQUANTUM : Ret: 0, Exchange quantum value 10 -> 200
SCULL_IOCXQSET    : Ret: 0, Exchange qset value 5 -> 20
SCULL_IOCHQUANTUM : Ret: 200, Quantum shift with 123
SCULL_IOCHQSET    : Ret: 20, Qset shift with 45
SCULL_RESET       : Ret: 0
SCULL_IOCGQUANTUM : Ret: 0, Quantum: 100
SCULL_IOCGQSET    : Ret: 0, Qset: 10
root@vinoth-VBox:/home/vinoth/Documents/Linux-Device-Driver/3c-scull-ioctl# 
```
