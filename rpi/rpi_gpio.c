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
//#define BCM2837_PERI_BASE 0x20000000

/* GPIO base address on the virtual memory in kernel */
#define RPI_GPIO_BASE (BCM2837_PERI_BASE + 0x00200000)

#define RPI_GPIO_RANGE 0x40

/* Module name */
#define RPI_GPIO_NAME "rpi_gpio"

/* Module major number */
static dev_t devMajorNumber;

/* The cdev structure */
static struct cdev gpioCDev;

/* Tie with the device model */
struct class *gpioClass;

/* Global variables */
//static unsigned long *gpio_addr_ioremap = NULL;
static void __iomem *gpio_addr_ioremap = NULL;
static int gpio_usage = 0;



/* return GPIO base address */
static volatile unsigned int *get_gpio_addr(void)
{
	PDEBUG("%s:%d (unsigned long) gpio_addr_ioremap = 0x%x\n", __FUNCTION__, __LINE__, gpio_addr_ioremap);
	PDEBUG("%s:%d (unsigned int) gpio_addr_ioremap = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)gpio_addr_ioremap);
	PDEBUG("%s:%d (unsigned int *) gpio_addr_ioremap = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int *)gpio_addr_ioremap);
	return (volatile unsigned int *)gpio_addr_ioremap;
}




/* 주소 addr에 값 val을 mask만큼 잘라낸 후 shift만큼 이동시켜 할당하는 함수 */
static int set_bits(volatile unsigned int *addr,
					const unsigned int shift,
					const unsigned int val,
					const unsigned int mask)
{
	unsigned int temp = *addr;

	PDEBUG("%s:%d *addr = 0x%x, shift = 0x%x, val = 0x%x, mask = 0x%x\n", __FUNCTION__, __LINE__, *addr, shift, val, mask);	
	PDEBUG("%s:%d (1) temp = 0x%x\n", __FUNCTION__, __LINE__, temp);
	
	/* initialize an assigned part */
	temp &= ~(mask << shift);

	PDEBUG("%s:%d (2) temp = 0x%x\n", __FUNCTION__, __LINE__, temp);
	
	/* set val into addr */
	temp |= (val & mask) << shift;

	PDEBUG("%s:%d (3) temp = 0x%x\n", __FUNCTION__, __LINE__, temp);
	
	*addr = temp;

	PDEBUG("%s:%d (4) temp = 0x%x\n", __FUNCTION__, __LINE__, temp);
	
	return 0;
}

/* 
 * assign a function of GPIO 16 to mode
 * Macro for function select (mode)
 * 000 : input
 * 001 : output
 * 100 : alternate function 0
 * 101 : alternate function 1
 * 110 : alternate function 2
 * 111 : alternate function 3
 * 011 : alternate function 4
 * 010 : alternate function 5
 */
static int func_pin_16(const unsigned int mode)
{
	volatile unsigned int *gpio = get_gpio_addr(); /* return 0xF2200000 */
	const unsigned int sel = 0x04;                 /* Function Select 1 to use GPIO 16 */
	const unsigned int shift = 18;                 /* GPIO 16 <== 18~20 bit */

	PDEBUG("%s:%d gpio = 0x%x\n", __FUNCTION__, __LINE__, gpio);
	PDEBUG("%s:%d *gpio = 0x%x\n", __FUNCTION__, __LINE__, *gpio);
	PDEBUG("%s:%d mode = %d\n", __FUNCTION__, __LINE__, mode);	

	if (mode > 7) {
		return -1;
	}

	/* assign 18~20bit of addr 0xF2200004 to mode */
	/* basic operation unit is sizeof(unsigned int) */
	/* access to GPIO 16 after (gpio + 1) */
	/* shift 0x7 because it is 111b */
	/* shfit+1, shift+2 set to 1b  */
	set_bits(gpio + sel/sizeof(unsigned int), shift, mode, 0x07);

	return 0;
}

/* set GPIO 16 */
static int set_pin_16(void)
{
	volatile unsigned int *gpio = get_gpio_addr(); /* return 0xF2200000 */
	const unsigned int sel = 0x1C;
	const unsigned int shift = 16;

	/* initialize register that is GPIO 16 after setting to High */
	set_bits(gpio + sel/sizeof(unsigned int), shift, S_HIGH, 0x01);
	set_bits(gpio + sel/sizeof(unsigned int), shift, S_LOW, 0x01);
	
	return 0;
}

/* clear GPIO 16 */
static int clr_pin_16(void)
{
	volatile unsigned int *gpio = get_gpio_addr();
	const unsigned int sel = 0x28;
	const unsigned int shift = 16;

	/* initialize register that is GPIO 16 after setting to High */
	set_bits(gpio + sel/sizeof(unsigned int), shift, S_HIGH, 0x01);
	set_bits(gpio + sel/sizeof(unsigned int), shift, S_LOW, 0x01);
	
	return 0;
}


/* turn on LED that is connected to GPIO 16 and print a message
 *
 * 1. PORT_ADDR, RANGE
 * 2. request_mem_region()
 * 3. ioremap
 *
 */
static int rpi_gpio_open(struct inode *inode,
						 struct file *filp)
{
	PDEBUG("%s:%d gpio_usage = %d\n", __FUNCTION__, __LINE__, gpio_usage);

	if (gpio_usage != 0) {
		PDEBUG("%s:%d GPIO is busy\n", __FUNCTION__, __LINE__);
		return -EBUSY;
	}
	
	if (!request_mem_region(RPI_GPIO_BASE,
							RPI_GPIO_RANGE,
							RPI_GPIO_NAME))	{
		PDEBUG("%s:%d Can't get IO Region\n", __FUNCTION__, __LINE__);
	}

	gpio_addr_ioremap = ioremap(RPI_GPIO_BASE, RPI_GPIO_RANGE);
	
	PDEBUG("%s:%d gpio_addr_ioremap = 0x%x\n", __FUNCTION__, __LINE__, gpio_addr_ioremap);
	PDEBUG("%s:%d &gpio_addr_ioremap = 0x%x\n", __FUNCTION__, __LINE__, &gpio_addr_ioremap);	
	//PDEBUG("%s:%d *gpio_addr_ioremap = 0x%x\n", __FUNCTION__, __LINE__, *gpio_addr_ioremap);
	PDEBUG("%s:%d (unsigned int) gpio_addr_ioremap = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)gpio_addr_ioremap);
	PDEBUG("%s:%d (unsigned int *) gpio_addr_ioremap = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int *)gpio_addr_ioremap);

	if (func_pin_16(M_OUTPUT) != 0) {
		PDEBUG("%s:%d: Error!\n", __FUNCTION__, __LINE__);
		return -1;
	}
	set_pin_16();
	
	gpio_usage = 1;
	PDEBUG("%s:%d gpio_usage = %d\n", __FUNCTION__, __LINE__, gpio_usage);
	return 0;
}

/* turn off LED that is connected to GPIO 16 and print a message */
static int rpi_gpio_release(struct inode *inode,
							struct file *filp)
{
	PDEBUG("%s:%d\n", __FUNCTION__, __LINE__);
	iounmap(gpio_addr_ioremap);
	release_mem_region((unsigned long)gpio_addr_ioremap, RPI_GPIO_RANGE);
	gpio_usage = 0;
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
	if(_ERR_ < 0) {
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

