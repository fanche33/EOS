obj-m += mydev.o
#echo $(shell uname -r)
KDIR = /home/fanche/linux/



all:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu CC=aarch64-linux-gnu-gcc -C $(KDIR) M=$(shell pwd) modules
 
clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu CC=aarch64-linux-gnu-gcc -C $(KDIR) M=$(shell pwd) clean
