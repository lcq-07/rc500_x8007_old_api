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
#include <linux/miscdevice.h>
#include "rc500_x8007.h"
#include "gpio_bus_comm.h"
#include "rc500.h"

#define DEVICE_NAME "SC0"

static int dev_open(struct inode *inode,struct file *filp)
{
	return 0;
}

static int dev_close(struct inode *inode,struct file *filp)
{
	return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
	return 0;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
	return 0;
}

static struct file_operations dev_fops = {
	.owner	 = THIS_MODULE,
	.open    = dev_open,
	.release = dev_close,
	.read    = dev_read,
	.write   = dev_write,
};


static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dev_fops,
};

static int __init dev_init(void)
{
	int ret;

	gpio_bus_init();
	ret = PCD_init();
	if(ret == 0) {
		printk("RC500 init OK.\n");
	}else {
		printk("RC500 init failed.\n");
	}
	PCD_get_info();	

	ret = misc_register(&misc);

	if(ret == 0) {
		printk("ISO14443 ISO7816 modules init OK.\n");
	}else {
		printk("ISO14443 ISO7816 modules init failed.\n");
	}

	return ret;
}

static void __exit dev_exit(void)
{ 
	gpio_bus_release();
	misc_deregister(&misc);
}

module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("nuc0707@163.com");
