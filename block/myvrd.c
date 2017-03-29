#include <linux/init.h>			/* module_init(), module_exit() */
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/vmalloc.h>

#include <linux/hdreg.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <asm/uaccess.h>

/*
 * Debug option
 */
#undef PDEBUG
#ifdef MYVRD_MODULE_DEBUG
#  ifdef __KERNEL__
#    define PDEBUG(fmt, args...) printk(KERN_DEBUG "MYVRD blkdev module : " fmt, ## args)
#  else
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...)
#endif

/* Global variables of the block device driver */
#define MYVRD_DEV_NAME "myvrd"
#define MYVRD_MAX_DEVICE 1
#define MYVRD_SECTOR_SIZE 512
#define MYVRD_SIZE (4*1024*1024)
#define MYVRD_SECTOR_TOTAL (MYVRD_SIZE/MYVRD_SECTOR_SIZE)

static int MYVRD_DEV_MAJOR = 240;

typedef struct {
	unsigned char *data;
	struct request_queue *queue;
	struct gendisk *gd;
} myvrd_device;

static char *vdisk[MYVRD_MAX_DEVICE];
static myvrd_device device[MYVRD_MAX_DEVICE];

static int myvrd_make_request(struct request_queue *q, struct bio *bio)
{
	myvrd_device *pdevice;
	char *pVHDDdata;
	char *pBuffer;
	struct bio_vec *bvec;
	int i;

	if (((bio->bi_sector * MYVRD_SECTOR_SIZE) + bio->bi_size) > MYVRD_SIZE) {
		//bio_io_error(bio, bio->bi_size); //until 2.6.22
		bio_io_error(bio);                 //after 2.6.24
		return 0;
	}

	pdevice = (myvrd_device *)bio->bi_bdev->bd_disk->private_data;
	pVHDDdata = pdevice->data + (bio->bi_sector * MYVRD_SECTOR_SIZE);

	bio_for_each_segment(bvec, bio, i) {
		pBuffer = kmap(bvec->bv_page) + bvec->bv_offset;
		switch(bio_data_dir(bio)) {
		case READA:
		case READ:
			memcpy(pBuffer, pVHDDdata, bvec->bv_len);
			break;
		case WRITE:
			memcpy(pVHDDdata, pBuffer, bvec->bv_len);
			break;
		default:
			kunmap(bvec->bv_page);
			//bio_io_error(bio, bio->bi_size); //until 2.6.22
			bio_io_error(bio);                 //after 2.6.24
			return 0;
		}
		kunmap(bvec->bv_page);
		pVHDDdata += bvec->bv_len;
	}
	//bio_endio(bio, bio->bi_size, 0); //until 2.6.22
	bio_endio(bio, 0);                 //after 2.6.24
	return 0;
}

static int myvrd_open(struct inode *inode, struct file *filp)
{
	PDEBUG("%s:%d\n", __FUNCTION__, __LINE__);
	return 0;
}

static int myvrd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("%s:%d\n", __FUNCTION__, __LINE__);
	return 0;
}

static int myvrd_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	PDEBUG("%s:%d\n", __FUNCTION__, __LINE__);
	return -ENOTTY;
}

static struct block_device_operations myvrd_fops = {
	.owner = THIS_MODULE,
	.open = myvrd_open,
	.release = myvrd_release,
	.ioctl = myvrd_ioctl
};

static int __init myvrd_init(void)
{
	int lp;
	vdisk[0] = vmalloc(MYVRD_SIZE);

	register_blkdev(MYVRD_DEV_MAJOR, MYVRD_DEV_NAME);

	for (lp = 0; lp < MYVRD_MAX_DEVICE; lp++) {
		device[lp].data = vdisk[lp];
		device[lp].gd = alloc_disk(1);
		device[lp].queue = blk_alloc_queue(GFP_KERNEL);
		blk_queue_make_request(device[lp].queue, &myvrd_make_request);
		device[lp].gd->major = MYVRD_DEV_MAJOR;
		device[lp].gd->first_minor = lp;
		device[lp].gd->fops = &myvrd_fops;
		device[lp].gd->queue = device[lp].queue;
		device[lp].gd->private_data = &device[lp];
		sprintf(device[lp].gd->disk_name, "myvrd%c", 'a'+lp);
		set_capacity(device[lp].gd, MYVRD_SECTOR_TOTAL);
		add_disk(device[lp].gd);
	}
	PDEBUG("%s:%d ==> myvrd_init done\n", __FUNCTION__, __LINE__);
	return 0;
}


static void __exit myvrd_exit(void)
{
	int lp;

	for (lp = 0; lp < MYVRD_MAX_DEVICE; lp++) {
		del_gendisk(device[lp].gd);		
		put_disk(device[lp].gd);
		blk_cleanup_queue(device[lp].queue);
	}
	
	unregister_blkdev(MYVRD_DEV_MAJOR, MYVRD_DEV_NAME);
	PDEBUG("%s:%d ==> myvrd_exit done\n", __FUNCTION__, __LINE__);
	
	vfree(vdisk[0]);
}

module_init(myvrd_init);
module_exit(myvrd_exit);

MODULE_LICENSE("GPL");



