obj-m += pz1_symb_drv.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules -I/lib/modules/$(shell uname -r)/build/include

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean