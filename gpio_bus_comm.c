/*
 * Filename      : gpio_bus_comm.c
 * Author        : nuc0707@163.com
 * Create time   : 2013-12-10 11:02
 * Modified      : append in the following formate
 * year-month-day description
 */

#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "gpio_bus_comm.h"

volatile unsigned int *gpbcon;
volatile unsigned int *gpbdat;
volatile unsigned int *gpgcon;
volatile unsigned int *gpgdat;
EXPORT_SYMBOL(gpbcon);
EXPORT_SYMBOL(gpbdat);
EXPORT_SYMBOL(gpgcon);
EXPORT_SYMBOL(gpgdat);
EXPORT_SYMBOL(gpio_bus_init);
EXPORT_SYMBOL(gpio_bus_release);
EXPORT_SYMBOL(gpio_bus_read);
EXPORT_SYMBOL(gpio_bus_write);

/* common */

void gpio_bus_init(void)
{
	gpbcon = (volatile unsigned int *)ioremap(0x56000010, 12);
	gpbdat = gpbcon + 1;
	gpgcon = (volatile unsigned int *)ioremap(0x56000060, 12);
	gpgdat = gpgcon + 1;
	printk("gpio_bus_comm init ok\ngpbcon[%p],gpgcon[%p]\n", gpbcon, gpgcon);

	RC500_GPIO_INIT();
	RC500_PD(SET_HI);
	RC500_CS(SET_HI);
	RC500_WR(SET_HI);
	RC500_RD(SET_HI);
	RC500_ALE(SET_HI);
	TDA8007B_GPIO_INIT();
	TDA8007B_CS(SET_HI);
	TDA8007B_WR(SET_HI);
	TDA8007B_RD(SET_HI);
	TDA8007B_ALE(SET_HI);
	TDA8007B_OUT_AD(0x00);
	RC500_CS(SET_HI);
}

void gpio_bus_release(void)
{
	RC500_PD(SET_HI);
	RC500_CS(SET_HI);
	RC500_WR(SET_HI);
	RC500_RD(SET_HI);
	RC500_ALE(SET_HI);
	TDA8007B_CS(SET_HI);
	TDA8007B_WR(SET_HI);
	TDA8007B_RD(SET_HI);
	TDA8007B_ALE(SET_HI);
	TDA8007B_OUT_AD(0x00);
	iounmap(gpgcon);
	iounmap(gpbcon);
}

unsigned char gpio_bus_read(char device_type, char addr)
{
	unsigned char data = 0;	

	switch(device_type) {
	case DEV_RC500:
		RC500_ALE(SET_HI); /* >=20ns */
		RC500_OUT_AD(addr);
		ndelay(25);
		RC500_ALE(SET_LO);
		ndelay(8);
		RC500_SET_IN();
		RC500_CS(SET_LO); 
		RC500_RD(SET_LO); /* >=65ns */
		RC500_IN_AD(data);
		ndelay(70);
		RC500_RD(SET_HI);
		RC500_CS(SET_HI);
		RC500_SET_OUT();
		RC500_OUT_AD(0);
		break;
	case DEV_TDA8007B:
		RC500_CS(SET_HI);
		TDA8007B_WR(SET_HI);
		TDA8007B_RD(SET_HI);
		TDA8007B_SET_OUT();
		TDA8007B_ALE(SET_LO);
		TDA8007B_CS(SET_LO);
		TDA8007B_OUT_AD(addr);
		TDA8007B_ALE(SET_HI);
		TDA8007B_ALE(SET_LO);
		TDA8007B_SET_IN();
		TDA8007B_RD(SET_LO);
		TDA8007B_IN_AD(data);
		TDA8007B_RD(SET_HI);
		TDA8007B_ALE(SET_HI);
		TDA8007B_CS(SET_HI);
		TDA8007B_SET_OUT();
		TDA8007B_OUT_AD(0);
		break;
	default :
		break;
	}
	return data;
}

void gpio_bus_write(char device_type, char addr, char data)
{
	switch(device_type) {
	case DEV_RC500:
		RC500_ALE(SET_HI); /* >=20ns */
		RC500_OUT_AD(addr);
		ndelay(25);
		RC500_ALE(SET_LO);
		ndelay(8);
		RC500_CS(SET_LO); 
		RC500_OUT_AD(data);
		RC500_WR(SET_LO); /* >=65ns */
		ndelay(70);
		RC500_WR(SET_HI);
		RC500_CS(SET_HI);
		break;
	case DEV_TDA8007B:
		RC500_CS(SET_HI);
		TDA8007B_WR(SET_HI);
		TDA8007B_RD(SET_HI);
		TDA8007B_SET_OUT();
		TDA8007B_ALE(SET_LO);
		TDA8007B_CS(SET_LO);
		TDA8007B_OUT_AD(addr);
		TDA8007B_ALE(SET_HI);
		TDA8007B_ALE(SET_LO);
		TDA8007B_OUT_AD(data);
		TDA8007B_WR(SET_LO);
		TDA8007B_WR(SET_HI);
		TDA8007B_ALE(SET_HI);
		TDA8007B_CS(SET_HI);
		break;
	default :
		break;
	}
}
