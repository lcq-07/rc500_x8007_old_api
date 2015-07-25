/*
 * Filename      : rc500_x8007.c
 * Author        : nuc0707#163#com
 * Create time   : 2015-06-26 10:54
 * Modified      : append in the following formate
 * year-month-day description
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "rc500_x8007.h"
#include "gpio_bus_comm.h"
#include "rc500.h"
#include "ISO7816.h"
#include "ISO14443_3A.h"
#include "ISO14443_4A.h"

static dev_t dev_no = 0;
static struct cdev *scr_dev[SC_READER_MAX];
static struct class *scr_class;
static unsigned char *scr_buf[SC_READER_MAX];

static int dev_open(struct inode *inode, struct file *filp)
{
	int ret;
	int cur_dev_major = imajor(inode);
	int cur_dev_minor = iminor(inode);

	switch(cur_dev_minor) { 
	case SC_READER_MIN+0: /* MFRC500 */
		ret = PCD_init();
		scr_buf[cur_dev_minor - SC_READER_MIN] = kmalloc(sizeof(struct APDU_MSG),GFP_KERNEL);
		if(scr_buf[cur_dev_minor - SC_READER_MIN] == NULL) {
			printk(KERN_NOTICE "kmalloc fail!\n");
			ret = -1;
		}
	case SC_READER_MIN+1: /* TDA8007B(DS8007) */
		ret = 0;
		break;
	default:
		ret = 0;
		break;
	}
	if(ret == 0) {
		printk(KERN_NOTICE SC_READER"[%d:%d] open ok!\n", cur_dev_major, cur_dev_minor);
	}else {
		printk(KERN_NOTICE SC_READER"[%d:%d] open fail!\n", cur_dev_major, cur_dev_minor);
	}
	filp->private_data = inode;

	return ret;
}

static int dev_close(struct inode *inode, struct file *filp)
{
	int ret;
	int cur_dev_major = imajor(inode);
	int cur_dev_minor = iminor(inode);

	switch(cur_dev_minor) { 
	case SC_READER_MIN+0: /* MFRC500 */
		if(scr_buf[cur_dev_minor - SC_READER_MIN] != NULL) {
			kfree(scr_buf[cur_dev_minor - SC_READER_MIN]);
		}
	case SC_READER_MIN+1: /* TDA8007B(DS8007) */
		ret = 0;
		break;
	default:
		ret = 0;
		break;
	}
	printk(KERN_NOTICE SC_READER"[%d:%d] close ok!\n", cur_dev_major, cur_dev_minor);

	return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
	return size;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
	int ret;
	int cur_dev_major = imajor(filp->private_data);
	int cur_dev_minor = iminor(filp->private_data);

	switch(cur_dev_minor) { 
	case SC_READER_MIN+0: /* MFRC500 */
		
		if(scr_buf[cur_dev_minor - SC_READER_MIN] != NULL) {
			kfree(scr_buf[cur_dev_minor - SC_READER_MIN]);
		}
	case SC_READER_MIN+1: /* TDA8007B(DS8007) */
		ret = 0;
		break;
	default:
		ret = 0;
		break;
	}

	return size;
}

static struct file_operations dev_fops = {
	.owner	 = THIS_MODULE,
	.open    = dev_open,
	.release = dev_close,
	.read    = dev_read,
	.write   = dev_write,
};

static int __init dev_init(void)
{
	int ret;
	int i;
	int j;
	unsigned char ATQ[2];

	ret = alloc_chrdev_region(&dev_no, SC_READER_MIN, SC_READER_MAX, SC_READER);
	if (ret < 0) {
		printk(KERN_WARNING "smard card reader: device number registration failed\n");
		return 0;
	}
	printk(KERN_NOTICE "dev no[%d:%d]\n", MAJOR(dev_no), MINOR(dev_no));
	scr_class = class_create(THIS_MODULE, SC_READER);
	if(!scr_class) {
		printk(KERN_WARNING "smard card reader: class_create failed\n");
		goto init_fail;
	}

	for(i=0; i<SC_READER_MAX; i++) {
		scr_dev[i] = cdev_alloc();
		if(!scr_dev[i]) {
			printk(KERN_WARNING "smard card reader: cdev_alloc failed\n");
			goto init_fail;
		}
		cdev_init(scr_dev[i], &dev_fops);
		scr_dev[i]->owner = THIS_MODULE;
		scr_dev[i]->ops = &dev_fops;
		ret = cdev_add(scr_dev[i], MKDEV(MAJOR(dev_no), SC_READER_MIN+i), 1);
		if(ret) {
			printk(KERN_NOTICE "Error %d adding smard card reader\n", ret);
			goto init_fail;
		}
		device_create(scr_class, NULL, MKDEV(MAJOR(dev_no), SC_READER_MIN+i), NULL, SC_READER"%d", SC_READER_MIN+i);
	}

	gpio_bus_init();
	return 0;
init_fail:
	for(i=0; i<SC_READER_MAX; i++) {
		if(scr_dev[i]) {
			device_destroy(scr_class, MKDEV(MAJOR(dev_no), SC_READER_MIN+i));
			cdev_del(scr_dev[i]);
		}
	}
	unregister_chrdev_region(MKDEV(MAJOR(dev_no), SC_READER_MIN), SC_READER_MAX);
	class_destroy(scr_class);

	return ret;
}

static void __exit dev_exit(void)
{
	int i;

	for(i=0; i<SC_READER_MAX; i++) {
		if(scr_dev[i]) {
			device_destroy(scr_class, MKDEV(MAJOR(dev_no), SC_READER_MIN+i));
			cdev_del(scr_dev[i]);
		}
	}
	unregister_chrdev_region(MKDEV(MAJOR(dev_no), SC_READER_MIN), SC_READER_MAX);
	class_destroy(scr_class);

	gpio_bus_release();
}

module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("nuc0707@163.com");
