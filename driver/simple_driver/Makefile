
obj-m := simple_driver.o
all:
	make -C /lib/modules/5.10.17-v7l+/build M=$(PWD) modules

clean:
	make -C /lib/modules/5.10.17-v7l+/build M=$(PWD) clean