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
#include "ISO14443_3A.h"
#include "ISO14443_4A.h"

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
	int i;
	int j;
	unsigned char ATQ[2];
	unsigned char sbuf[256];
	unsigned char rbuf[256];

	gpio_bus_init();
	ret = PCD_init();
	if(ret == 0) {
		printk("RC500 init OK.\n");
	}else {
		printk("RC500 init failed.\n");
	}
	PCD_get_info();
	mdelay(300);
	i = 3;
	while(--i) {	
	PICC_request(0x52, ATQ);
	PICC_anticoll(PICC_UID);
	PICC_select(PICC_UID, NULL);
	PICC_rats(NULL, &ret);
	sbuf[0] = 0x00;
	sbuf[1] = 0xA4;
	sbuf[2] = 0x04;
	sbuf[3] = 0x00;
	sbuf[4] = 0x0E;
	sbuf[5] = 0x32;
	sbuf[6] = 0x50;
	sbuf[7] = 0x41;
	sbuf[8] = 0x59;
	sbuf[9] = 0x2E;
	sbuf[10] = 0x53;
	sbuf[11] = 0x59;
	sbuf[12] = 0x53;
	sbuf[13] = 0x2E;
	sbuf[14] = 0x44;
	sbuf[15] = 0x44;
	sbuf[16] = 0x46;
	sbuf[17] = 0x30;
	sbuf[18] = 0x31;
	j = 19;
	ret = PICC_tcl(sbuf, rbuf, &j);
	if(ret == 0) {
		for(ret=0; ret<j; ret++) {
			printk("%02X", rbuf[ret]);
		}
		printk("\n");
	}
	sbuf[0] = 0x00;
	sbuf[1] = 0xA4;
	sbuf[2] = 0x04;
	sbuf[3] = 0x00;
	sbuf[4] = 0x08;
	sbuf[5] = 0xA0;
	sbuf[6] = 0x00;
	sbuf[7] = 0x00;
	sbuf[8] = 0x03;
	sbuf[9] = 0x33;
	sbuf[10] = 0x01;
	sbuf[11] = 0x01;
	sbuf[12] = 0x01;
	j = 13;
	ret = PICC_tcl(sbuf, rbuf, &j);
	if(ret == 0) {
		for(ret=0; ret<j; ret++) {
			printk("%02X", rbuf[ret]);
		}
		printk("\n");
	}
	PICC_deselect();
	PICC_deselect();
#if 0
	//PICC_MFauth(0x60, 3, "\xA0\xA1\xA2\xA3\xA4\xA5");
	PICC_MFauth(0x61, 3, "\xFF\xFF\xFF\xFF\xFF\xFF");
	PICC_MFread(0, NULL);
	for(ret=0; ret<16; ret++) {
		printk("%02X", PCD_BUF[ret]);
	}
	printk("\n");
	PICC_MFread(1, NULL);
	for(ret=0; ret<16; ret++) {
		printk("%02X", PCD_BUF[ret]);
	}
	printk("\n");
	PICC_MFread(2, NULL);
	for(ret=0; ret<16; ret++) {
		printk("%02X", PCD_BUF[ret]);
	}
	printk("\n");
	//PICC_MFauth(0x61, 3, "\xFF\xFF\xFF\xFF\xFF\xFF");
	//PICC_MFwrite(0, "\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF");
	PICC_MFwrite(1, "\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF");
	PICC_MFwrite(2, "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF");
	PICC_MFread(0, NULL);
	for(ret=0; ret<16; ret++) {
		printk("%02X", PCD_BUF[ret]);
	}
	printk("\n");
	PICC_MFread(1, NULL);
	for(ret=0; ret<16; ret++) {
		printk("%02X", PCD_BUF[ret]);
	}
	printk("\n");
	PICC_MFread(2, NULL);
	for(ret=0; ret<16; ret++) {
		printk("%02X", PCD_BUF[ret]);
	}
#endif
	printk("\n");
	//PICC_halt();
	}

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
