/*
 * Filename      : x8007.h
 * Author        : nuc0707#163#com
 * Create time   : 2015-07-30 12:43
 * Modified      : append in the following formate
 * year-month-day description
 */

#ifndef _X8007_H_
#define _X8007_H_

#include <linux/types.h>
#include <linux/ioctl.h>

// 8007 Crystal Frequency
#define CRYSTAL_FREQUENCY_8007 14284800L

//#define DEVICE_NAME "CST-TDA8007B"
#define TDA8007B_IRQ_NO	(3)
#define MAX_BUF_LEN	512
#define MAX_ATR_LEN	(33)
#define MAX_CMD_LEN	(262)	/* header : 5, data : 256(max), le : 1, plus all 262 */

typedef struct{
	int num;
	volatile unsigned int state;	/* 0 - everything goes well, 1 - you must resolve error first */
	volatile unsigned int errno;	/* error number */
	unsigned char atr_buffer[MAX_ATR_LEN];	/* atr string */
	unsigned int len;			/* atr len */
	volatile unsigned int timeout, bCardChanged;	/* 0 - no error 1 - timeout , or error occured */
	unsigned int openflag;
	unsigned int clock;	/* frequence in KHz */
	unsigned int etu; 	/* in millisecond */
	unsigned int cwt;	/* timeout value between characters  ( in etus) */
	unsigned int bwt;	/* timeout value before first character ( in etus) */
	unsigned int jiffies;	/* time out counter */
	unsigned int voltage;
}SmartCard_Dev;

/* registers for TDA8007BHL */
#define TDA8007B_CSR	0x00
#define TDA8007B_CCR	0x01
#define TDA8007B_PDR	0x02
#define TDA8007B_UCR2	0x03
#define TDA8007B_GTR	0x05
#define TDA8007B_UCR1	0x06
#define TDA8007B_PCR	0x07
#define TDA8007B_TOC	0x08
#define TDA8007B_TOR1	0x09
#define TDA8007B_TOR2	0x0a
#define TDA8007B_TOR3	0x0b
#define TDA8007B_MSR	0x0c
#define TDA8007B_FCR	0x0c
#define TDA8007B_UTR	0x0d
#define TDA8007B_URR	0x0d
#define TDA8007B_USR	0x0e
#define TDA8007B_HSR	0x0f

/* smartcard */
struct sc_parameter{
	unsigned int n, t;		/* amount of etu in cycles, and others are in etus */
	unsigned int cwt, bwt;
	unsigned int fi, di;		/* default value is 372 , 1 */
};

#define SMARTCARD_NUM	2
/* define I/O control command */
#define SMARTCARD_IOC_MAGIC	's'
#define SC_IOC_MAXNR			16

#define SC_IOC_POWERON		_IOW(SMARTCARD_IOC_MAGIC, 0, int)		/* card power on */
#define SC_IOC_POWEROFF		_IO(SMARTCARD_IOC_MAGIC, 1)			/* card power off */

/* this two command is for power saving */
#define SC_IOC_POWERUP		_IO(SMARTCARD_IOC_MAGIC, 2)			/* card power up */
#define SC_IOC_POWERDOWN	_IO(SMARTCARD_IOC_MAGIC, 3)			/* card power down */
#define SC_IOC_ACTIVATE		_IO(SMARTCARD_IOC_MAGIC, 4)			/* activate card and save ATR to buffer, you can use GETATR to retrieve */
#define SC_IOC_DEACTIVATE	_IO(SMARTCARD_IOC_MAGIC, 5)			/* deactivate */
#define SC_IOC_C4C8READ		_IOR(SMARTCARD_IOC_MAGIC, 6, int)		/* for general use */
#define SC_IOC_C4C8WRITE	_IOW(SMARTCARD_IOC_MAGIC, 7, int)
#define SC_IOC_SETPARAMETER	_IOW(SMARTCARD_IOC_MAGIC, 8, struct sc_parameter *)	/* set reader parameter to adjust card */
#define SC_IOC_CLEARFIFO	_IO(SMARTCARD_IOC_MAGIC, 9)			/* clear reception buffer */
#define SC_IOC_GETSTATUS	_IOR(SMARTCARD_IOC_MAGIC, 10, int)		/* check card present */
#define SC_IOC_SELECTCARD	_IO(SMARTCARD_IOC_MAGIC, 11)			/* for multi-card  use */
#define SC_IOC_GETATR		_IOR(SMARTCARD_IOC_MAGIC, 12, MAX_ATR_LEN)
#define SC_IOC_GETERRORNO	_IOR(SMARTCARD_IOC_MAGIC, 13, int)		/* get error number */
#define SC_IOC_MATCHREADER	_IOR(SMARTCARD_IOC_MAGIC, 14, int)		/* check whether the reader support the baudrate or not */

#define SC_IOC_SETPARITY	_IOW(SMARTCARD_IOC_MAGIC, 15, int)
#define SC_IOC_SETCLOCK		_IOW(SMARTCARD_IOC_MAGIC, 16, int)

/* iso7816 operation class */
#define SC_ISO_OPERATIONCLASS_A	(0x01)
#define SC_ISO_OPERATIONCLASS_B	(0x02)

/* define err_flag */
#define SC_ERR_ID			(0x00)

#define SC_ERR_NOERROR			(0x00)
#define SC_ERR_CARDREMOVED		(0x01 | SC_ERR_ID)
#define SC_ERR_TIMEOUT			(0x02 | SC_ERR_ID)
#define SC_ERR_READ			(0x03 | SC_ERR_ID)
#define SC_ERR_WRITE			(0x04 | SC_ERR_ID)
#define SC_ERR_NOTHISBAUDRATE		(0x05 | SC_ERR_ID)

#define SC_ERR_DEVICE			(0x10 | SC_ERR_ID)

#define SC_ERR_DEV_GENERAL		SC_ERR_DEVICE
#define SC_ERR_DEV_OVERHEAT		(0x01 | SC_ERR_DEVICE)
#define SC_ERR_DEV_PROTECTION		(0x02 | SC_ERR_DEVICE)
#define SC_ERR_DEV_EARLYANSWER		(0x03 | SC_ERR_DEVICE)
#define SC_ERR_DEV_PARITY		(0x04 | SC_ERR_DEVICE)
#define SC_ERR_DEV_OVERRUN		(0x05 | SC_ERR_DEVICE)
#define SC_ERR_DEV_FRAMING		(0x06 | SC_ERR_DEVICE)

void tda8007b_init(void);
int tda8007b_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
int tda8007b_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
int tda8007b_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int tda8007b_release(struct inode *inode, struct file *filp);
int tda8007b_open(struct inode *inode, struct file *filp);
int tda8007b_close(struct inode *inode, struct file *filp);



#endif /* _X8007_H_ */

