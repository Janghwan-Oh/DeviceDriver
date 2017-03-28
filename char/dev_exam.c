
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <asm/uaccess.h>

/*
 * Debug option
 */
#undef PDEBUG
#ifdef CDEV_MODULE_DEBUG
#  ifdef __KERNEL__
#    define PDEBUG(fmt, args...) printk(KERN_DEBUG "CDEV module : " fmt, ## args)
#  else
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...)
#endif


#define DEV_NAME "dev_exam"
#define DEV_MAJOR 254
#define DEV_MINOR 5

#define MY_IOC_NUM 100
#define MY_IOC_READ _IOR(MY_IOC_NUM, 0, int)
#define MY_IOC_WRITE _IOW(MY_IOC_NUM, 1, int)
#define MY_IOC_STATUS _IO(MY_IOC_NUM, 2)
#define MY_IOC_READ_WRITE _IOWR(MY_IOC_NUM, 3, int)
#define MY_IOC_NR 4

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andy");
MODULE_DESCRIPTION("character device driver for kernel 2.6");

dev_t dev_num = -1;
struct cdev *dev_exam_cdev = NULL;

static int dev_exam_open(struct inode *inode, struct file *filp)
{
	PDEBUG("%s:%d\n", __FUNCTION__, __LINE__);	
	return 0;
}

static int dev_exam_release(struct inode *inode, struct file *filp)
{
	PDEBUG("%s:%d\n", __FUNCTION__, __LINE__);	
	return 0;
}

static int dev_exam_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	int size = 0;

	if (_IOC_TYPE(cmd) != MY_IOC_NUM) {
		return -EINVAL;
	}

	if (_IOC_NR(cmd) != MY_IOC_NR) {
		return -EINVAL;
	}

	if (_IOC_DIR(cmd) & _IOC_READ) {
		err = access_ok(VERIFY_READ, (void*)arg, sizeof(unsigned long));
		if (err < 0) {
			return -EINVAL;
		}
	}

	size = _IOC_SIZE(cmd);

	switch(cmd) {
	case MY_IOC_READ:
		PDEBUG("%s:%d ==> _IOC_READ\n", __FUNCTION__, __LINE__);	
		break;
	case MY_IOC_WRITE:
		PDEBUG("%s:%d ==> _IOC_WRITE\n", __FUNCTION__, __LINE__);	
		break;
	case MY_IOC_STATUS:
		PDEBUG("%s:%d ==> _IOC_STATUS\n", __FUNCTION__, __LINE__);	
		break;
	case MY_IOC_READ_WRITE:
		PDEBUG("%s:%d ==> _IOC_READ_WRITE\n", __FUNCTION__, __LINE__);			
		break;
	default:
		break;		
	}
	
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = dev_exam_open,
	.release = dev_exam_release,
	.ioctl = dev_exam_ioctl
};

static int __init dev_exam_init(void)
{
	dev_num = MKDEV(DEV_MAJOR, DEV_MINOR);
	register_chrdev_region(dev_num, 1, DEV_NAME);
	dev_exam_cdev = cdev_alloc();
	cdev_init(dev_exam_cdev, &fops);
	cdev_add(dev_exam_cdev, dev_num, 1);

	PDEBUG("%s:%d ==> device init and created dev = %d\n", __FUNCTION__, __LINE__, dev_num);
	return (int)dev_num;
}

static void __exit dev_exam_exit(void)
{
	unregister_chrdev_region(dev_num, 1);
	PDEBUG("%s:%d ==> device exit\n", __FUNCTION__, __LINE__);		
}

module_init(dev_exam_init);
module_exit(dev_exam_exit);







