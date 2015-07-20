/*
 * Filename      : gpio_bus_comm.h
 * Author        : nuc0707@163.com
 * Create time   : 2013-12-10 11:23
 * Modified      : append in the following formate
 * year-month-day description
 */
#ifndef __GPIO_BUS_COMM_H__
#define __GPIO_BUS_COMM_H__
#include <linux/delay.h>

#define DEV_RC500	0
#define DEV_TDA8007B	1

#define GPIO_BUS_UNUSED		0x00
#define GPIO_BUS_OPEN		0x01
#define GPIO_BUS_RC500		0x02
#define GPIO_BUS_TDA8007B	0x04

#define GPB_BASE	0x56000010
#define GPG_BASE	0x56000060

#define SET_LO	0
#define SET_HI	1

#define GPIO_RET_OK	(0)
#define GPIO_RET_ERR	(-1)

/* for RC500		for TDA8007B  
 * GPG11-PWD_RST	GPF3(INT3)-IN
 * GPG10-CS		GPG7-CS
 * GPB8-ALE		GPB8-ALE
 * GPB9-RD		GPB9-RD
 * GPB10-WR		GPB10-WR
 * GPB0~7-AD0~7		GPB0~7-AD0~7
 */

#define RC500_GPIO_INIT() {*gpgcon = ((*gpgcon&~(0x3<<22))|(0x1<<22));\
*gpgcon = ((*gpgcon&~(0x3<<20))|(0x1<<20));\
*gpbcon = ((*gpbcon&~(0x3<<20))|(0x1<<20));\
*gpbcon = ((*gpbcon&~(0x3<<18))|(0x1<<18));\
*gpbcon = ((*gpbcon&~(0x3<<16))|(0x1<<16));}

#define RC500_PD(n)	{*gpgdat = ((*gpgdat & ~(0x1<<11))|(n<<11));}
#define RC500_CS(n)	{*gpgdat = ((*gpgdat & ~(0x1<<10))|(n<<10));}
#define RC500_WR(n)	{*gpbdat = ((*gpbdat & ~(0x1<<10))|(n<<10));}
#define RC500_RD(n)	{*gpbdat = ((*gpbdat & ~(0x1<<9))|(n<<9));}
#define RC500_ALE(n)	{*gpbdat = ((*gpbdat & ~(0x1<<8))|(n<<8));}
#define RC500_IN_AD(n)	{*gpbcon = ((*gpbcon&~(0xffff<<0))); ndelay(2); n = *gpbdat & 0xff;}
#define RC500_OUT_AD(n)	{*gpbcon = ((*gpbcon&~(0xffff<<0))|(0x5555<<0)); ndelay(2); *gpbdat = ((*gpbdat & ~(0xff)) | (n&0xff));}


#define TDA8007B_GPIO_INIT() {*gpgcon = ((*gpgcon&~(0x3<<14))|(0x1<<14));\
*gpbcon = ((*gpbcon&~(0x3<<20))|(0x1<<20));\
*gpbcon = ((*gpbcon&~(0x3<<18))|(0x1<<18));\
*gpbcon = ((*gpbcon&~(0x3<<16))|(0x1<<16));}

#define TDA8007B_CS(n)		{*gpgdat = ((*gpgdat & ~(0x1<<7))|(n<<7));}
#define TDA8007B_WR(n)		{*gpbdat = ((*gpbdat & ~(0x1<<10))|(n<<10));}
#define TDA8007B_RD(n)		{*gpbdat = ((*gpbdat & ~(0x1<<9))|(n<<9));}
#define TDA8007B_ALE(n)		{*gpbdat = ((*gpbdat & ~(0x1<<8))|(n<<8));}
#define TDA8007B_IN_AD(n)	{*gpbcon = ((*gpbcon&~(0xffff<<0))); n = *gpbdat & 0xff;}
#define TDA8007B_OUT_AD(n)	{*gpbcon = ((*gpbcon&~(0xffff<<0))|(0x5555<<0)); *gpbdat = ((*gpbdat & ~(0xff)) | (n&0xff));}

extern volatile unsigned int *gpbcon;
extern volatile unsigned int *gpbdat;
extern volatile unsigned int *gpgcon;
extern volatile unsigned int *gpgdat;

extern void gpio_bus_init(void);
extern void gpio_bus_release(void);
extern unsigned char gpio_bus_read(char device_type, char addr);
extern void gpio_bus_write(char device_type, char addr, char dat);

#endif

