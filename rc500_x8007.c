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
#include <linux/miscdevice.h>
#include <asm/uaccess.h> /* copy_to/from_user */
#include "debug_print.h"
#include "rc500_x8007.h"
#include "gpio_bus_comm.h"
#include "rc500.h"
#include "x8007.h"
#include "ISO7816.h"
#include "ISO14443_3A.h"
#include "ISO14443_4A.h"

#define DEVICE_NAME "CST-RC500"

#define SELECT_TDA8007B	0xff
#define SELECT_RC500	0x00

static struct semaphore open_close;
static struct semaphore read_write;
TranSciveBuffer command;  /* RC500一次操作成功后的返回结果 */
static char select_dev;


static int dev_open(struct inode *inode, struct file *filp)
{
	int ret;

	ret = down_trylock(&open_close);  /* 判断是否已经被其他进程所使用 */
	if(ret) { /* 获取失败 */
		return -1;
	}else {   /* 获取成功 */
		#if 1
		ret = PCD_init();
		if(ret != 0) {
			up(&open_close);//添加代码
			return -1;
		}
		#endif
		tda8007b_open(NULL, NULL);
		return 0;
	}
}

static int dev_close(struct inode *inode, struct file *filp)
{
	PCD_antenna(0);
	up(&open_close);
	return 0;
}

/* read调用成功返回读到的字节数,读到尾端返回0,出错返回-1. */
static ssize_t dev_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	int ret = 0;
	unsigned char length;
	unsigned char str[256];

	down_interruptible(&read_write);
	str[0] = command.MfCommand;
	str[1] = command.MfCtlFlag;
	str[2] = command.MfSector;
	str[3] = command.MfLength;
	memcpy(&str[4], command.MfData, command.MfLength);
	str[command.MfLength+4] = command.MfStatus;
	length = command.MfLength + 5;

	/* copy_to_user:若拷贝成功返回0,失败返回未拷贝的字节数 */
	ret = copy_to_user(buf, (void *)str, length);
	if(ret) { /* 若未拷贝完全 */
		ret = -1; /* 返回-1,表示执行失败 */
	}else {
		ret = length;
	}
	up(&read_write);
	return ret;
}

/* write写成功返回写入的字节数,若失败则返回-1 */
static ssize_t dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	int ret = -1;
	unchar str[256];
	TranSciveBuffer send;


	down_interruptible(&read_write);
	ret = copy_from_user(&str, buf, count);
	if(ret != 0){
		up(&read_write);
		return -1;  /* 出错返回 */
	}
	/* write by tda8007b */
	select_dev = str[0];
	if(select_dev == SELECT_TDA8007B) {
		ret = tda8007b_write(filp, str, count, ppos);
	}else {
	/* write by rc500 */
		memset(&command, 0, sizeof(TranSciveBuffer));
		send.MfCommand = str[0];
		send.MfCtlFlag = str[1];
		send.MfSector  = str[2];
		send.MfLength  = str[3];
		memcpy(send.MfData, &str[4], str[3]);
		send.MfStatus  = str[str[3]+4];
		ret = sendtoRC500(&send, &command);
		if(ret == 0) {
			ret = count;  /* 返回已写字节数,表示写入成功. */
		}else {
			ret = -1;  /* 返回写失败 */
		}
	}
	up(&read_write);
  
	return ret;
}

static struct file_operations dev_fops = {
	.owner	 = THIS_MODULE,
	.open    = dev_open,
	.release = dev_close,
	.read    = dev_read,
	.write   = dev_write,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR, /* 动态分配混杂设备次设备号 */
	.name = DEVICE_NAME,
	.fops = &dev_fops,  /* 关联文件操作 */
};

static int __init dev_init(void)
{
	int ret;

	gpio_bus_init();

	ret = misc_register(&misc);

	if(ret == 0){
		init_MUTEX(&read_write);
		init_MUTEX(&open_close);
		printk("Insmod MFRC500 and X8007 OK.\n");
	}else {
		printk("Insmod MFRC500 and X8007.\n");
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
