
# Comment/uncomment the following line to disable/enable debugging
# DEBUG = y
# Add my debugging flag (or not) to CFLAG
#ifeq ($(DEBUG),y)
#	DEBFLAGS = -O -g -DRPI_GPIO_MODULE_DEBUG
#else
#	DEBFLAGS = -O2
#endif
#EXTRA_CFLAGS += $(DEBFLAGS)

obj-m := rpi_gpio.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf .tmp_versions

