/*
 * Raspberry Pi - controll GPIO
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <asm/io.h>

/*
 * Debug option
 */
#define RPI_GPIO_MODULE_DEBUG

#undef PDEBUG
#ifdef RPI_GPIO_MODULE_DEBUG
#  ifdef __KERNEL__
#    define PDEBUG(fmt, args...) printk(KERN_DEBUG "[RPI_GPIO] " fmt, ## args)
#  else
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...)
#endif


/* Macro for function select (mode)
 * 000 : input
 * 001 : output
 * 100 : alternate function 0
 * 101 : alternate function 1
 * 110 : alternate function 2
 * 111 : alternate function 3
 * 011 : alternate function 4
 * 010 : alternate function 5
 */
#define M_INPUT 0
#define M_OUTPUT 1

/* Macro for GPIO status */
#define S_LOW 0
#define S_HIGH 1

/* I/O Peripherals base address on the virtual memory in kernel */
#define BCM2837_PERI_BASE 0xF2000000

/* GPIO base address on the virtual memory in kernel */
#define RPI_GPIO_BASE (BCM2837_PERI_BASE + 0x00200000)

/* return GPIO base address */
static volatile unsigned int *get_gpio_addr(void)
{
	return (volatile unsigned int *)RPI_GPIO_BASE; 
}

#define RPI_GPIO_PAGE_SIZE 0x1000

/* Module name */
#define RPI_GPIO_NAME "rpi_gpio"

/* Module major number */
static dev_t devMajorNumber;

/* The cdev structure */
static struct cdev gpioCDev;

/* Tie with the device model */
struct class *gpioClass;

/* Global variables */
static unsigned long *gpio_ioremap = NULL;
static int gpio_usage = 0;



/* turn on LED that is connected to GPIO 16 and print a message */
static int rpi_gpio_open(struct inode *inode,
						 struct file *filp)
{
	PDEBUG("%s:%d gpio_usage = %d\n", __FUNCTION__, __LINE__, gpio_usage);
#if 1
	
	if (gpio_usage != 0) {
		PDEBUG("%s:%d GPIO is busy\n", __FUNCTION__, __LINE__);
		return -EBUSY;
	}
	gpio_ioremap = (unsigned long *)ioremap(RPI_GPIO_BASE, RPI_GPIO_PAGE_SIZE);
	
	if (!check_mem_region((unsigned long)gpio_ioremap, RPI_GPIO_PAGE_SIZE)) {
		request_mem_region((unsigned long)gpio_ioremap, RPI_GPIO_PAGE_SIZE, RPI_GPIO_NAME);
	} else {
		PDEBUG("%s:%d Can'tget IO Region 0x%x\n\n", __FUNCTION__, __LINE__, (unsigned int)gpio_ioremap);
	}
	gpio_usage = 1;
	PDEBUG("%s:%d gpio_usage = %d\n", __FUNCTION__, __LINE__, gpio_usage);
#else
	gpio_usage = 1;
#endif
	return 0;
}

/* turn off LED that is connected to GPIO 16 and print a message */
static int rpi_gpio_release(struct inode *inode,
							struct file *filp)
{
	PDEBUG("%s:%d\n", __FUNCTION__, __LINE__);
#if 1
	iounmap(gpio_ioremap);
	release_mem_region((unsigned long)gpio_ioremap, RPI_GPIO_PAGE_SIZE);
	gpio_usage = 0;
#else
	gpio_usage = 0;
#endif	
	return 0;
}

static ssize_t rpi_gpio_read(struct file *filp,
							 char __user *buf,
							 size_t count,
							 loff_t *f_pos)
{
	PDEBUG("%s:%d\n", __FUNCTION__, __LINE__);
	return 0;
}

static ssize_t rpi_gpio_write(struct file *filp,
							  const char __user *buf,
							  size_t count,
							  loff_t *f_pos)
{
	PDEBUG("%s:%d\n", __FUNCTION__, __LINE__);
	return 0;
}


static struct file_operations rpi_gpio_fops = {
	.owner = THIS_MODULE,
	.open = rpi_gpio_open,
	.release = rpi_gpio_release,
	.read = rpi_gpio_read,
	.write = rpi_gpio_write
};

static int rpi_gpio_init(void)
{
	int _ERR_;

	/* Request dynamic allocation of a device major number */
	_ERR_ = alloc_chrdev_region(&devMajorNumber, 0, 1, RPI_GPIO_NAME);
	if(_ERR_ < 0) {
		PDEBUG("%s:%d Can't register device\n", __FUNCTION__, __LINE__);
		return -1;
	}

	PDEBUG("%s:%d Major Number [%d]\n", __FUNCTION__, __LINE__, MAJOR(devMajorNumber));

	/* Populate sysfs entries */
	gpioClass = class_create(THIS_MODULE, RPI_GPIO_NAME);

	/* Connect the file operations with the cdev */
	cdev_init(&gpioCDev, &rpi_gpio_fops);

	/* Connect the major/minor number to the cdev */
	_ERR_ = cdev_add(&gpioCDev, devMajorNumber, 1);
	if( _ERR_ < 0 )	{
		PDEBUG("%s:%d Bad cdev\n", __FUNCTION__, __LINE__);		
		return -1;
	}

	/* Creates a device and registers it with sysfs */
	device_create(gpioClass, NULL, devMajorNumber, NULL, RPI_GPIO_NAME);

	PDEBUG("%s:%d RPI GPIO Initialized\n", __FUNCTION__, __LINE__);
	return 0;
}

static void rpi_gpio_exit(void)
{
	PDEBUG("%s:%d\n", __FUNCTION__, __LINE__);
	unregister_chrdev_region(devMajorNumber, 1);
	cdev_del(&gpioCDev);
	device_destroy(gpioClass, gpioCDev.dev);
	class_destroy(gpioClass);
}

module_init(rpi_gpio_init);
module_exit(rpi_gpio_exit);
MODULE_LICENSE("Dual BSD/GPL");

