obj-m += rc500_tda8007b.o
rc500_tda8007b-objs := rc500_x8007.o gpio_bus_comm.o rc500.o

KDIR = ../../linux-2.6.32.60

all:
	make -C $(KDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=arm-linux-

clean:
	rm -rf *.order *.o *.ko *.symvers *.mod.c .*.cmd .tmp_versions

