obj-m += kernet.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc usernet.c -o usernet.o
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm usernet.o