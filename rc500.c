/*
 * Filename      : rc500.c
 * Author        : nuc0707#163#com
 * Create time   : 2015-06-26 10:59
 * Modified      : append in the following formate
 * year-month-day description
 * ISO/IEC-14443 PCD设备驱动
 */

#include <linux/time.h>
#include <linux/delay.h>
#include "gpio_bus_comm.h"
#include "rc500.h"

unsigned char PCD_read(unsigned char addr)
{
	return (gpio_bus_read(DEV_RC500, addr));
}

void PCD_write(unsigned char addr, unsigned char data)
{
	gpio_bus_write(DEV_RC500, addr, data); 
}

void PCD_bitset(unsigned char addr, unsigned char bits)
{
	PCD_write(addr, PCD_read(addr)|bits);
}

void PCD_bitclr(unsigned char addr, unsigned char bits)
{
	PCD_write(addr, PCD_read(addr)&(~bits));
}

/*
 * 天线控制
 */
void PCD_antenna(unsigned char ctl)
{
	if(ctl) {
		/* 打开TX1 TX2信号 */
		PCD_bitset(RegTxControl, 0x03);	
	}else {
		/* 关闭TX1 TX2信号 */
		PCD_bitclr(RegTxControl, 0x03);	
	}
}

/* 清空FIFO */
void PCD_flush_fifo(void)
{
	PCD_bitset(RegControl, 0x01);	
}

/* 设置定时器 */
void PCD_set_timeout(int tmo)
{
	/* timer clock frequency 13,56 MHz */
	switch(tmo) { 
	case 1: /* 1.0 ms */
		PCD_write(RegTimerClock ,0x07);  /* TAutoRestart=0,TPrescale=128 */
		PCD_write(RegTimerReload ,0x6a); /* TReloadVal = 'h6a =106(dec) */
		break;
	case 2: /* 1.5 ms */
		PCD_write(RegTimerClock ,0x07);  /* TAutoRestart=0,TPrescale=128 */
		PCD_write(RegTimerReload ,0xa0); /* TReloadVal = 'ha0 =160(dec) */
		break;
	case 3: /* 6 ms */
		PCD_write(RegTimerClock ,0x09);  /* TAutoRestart=0,TPrescale=4*128 */
		PCD_write(RegTimerReload ,0xa0); /* TReloadVal = 'ha0 =160(dec) */
		break;
	case 4: /* 9.6 ms */
		PCD_write(RegTimerClock ,0x09);  /* TAutoRestart=0,TPrescale=4*128 */
		PCD_write(RegTimerReload ,0xff); /* TReloadVal = 'hff =255(dec) */
		break;
	case 5: /* 38.5 ms */
		PCD_write(RegTimerClock ,0x0b);  /* TAutoRestart=0,TPrescale=16*128 */
		PCD_write(RegTimerReload ,0xff); /* TReloadVal = 'hff =255(dec) */
		break;
	case 6: /* 154 ms */
		PCD_write(RegTimerClock ,0x0d);  /* TAutoRestart=0,TPrescale=64*128 */
		PCD_write(RegTimerReload ,0xff); /* TReloadVal = 'hff =255(dec) */
		break;
	case 7: /* 616.2 ms */
		PCD_write(RegTimerClock ,0x0f);  /* TAutoRestart=0,TPrescale=256*128 */
		PCD_write(RegTimerReload ,0xff); /* TReloadVal = 'hff =255(dec) */
		break;
	default:
		PCD_write(RegTimerClock ,0x07);  /* TAutoRestart=0,TPrescale=128 */
		PCD_write(RegTimerReload,tmo);   /* TReloadVal = 'h6a =tmo(dec) */
		break;
	}
}

/* 复位 */
int PCD_reset(void)
{
	int i;

	/* startup */
	RC500_PD(SET_HI); /* >=100us */
	mdelay(1);
	RC500_PD(SET_LO);
	mdelay(1);
	i = 0x2000;
	/* wait until reset command recognized */
	/* while reset sequence in progress */
	while((PCD_read(RegCommand)&PCD_STARTUP) && (i--)) {
		ndelay(15);
	}
	if(i == 0) {
		return -1;
	}

	i = 0x2000;
	/* Dummy access in order to determine the bus configuration */
	PCD_write(RegPage, 0x80);
	/* necessary read access after first write access, 
 	 * the returned value should be zero ==> interface
 	 * recognized 
 	 */
	while((PCD_read(RegCommand)!=PCD_IDLE) && (i--)) {
		ndelay(15);
	}
	if(i == 0) {
		return -1;
	}
	/* configure to linear address mode */
	PCD_write(RegPage, 0x00);
	/* init config */

	return 0;
}
/* 初始化 */
int PCD_init(void)
{
	int ret;

	ret = PCD_reset();
	#if 0
	if(ret == 0) {
		/* test clock Q calibration - value in the range of 0x46 expected */
		PCD_write(RegClockQControl, 0x0);
		PCD_write(RegClockQControl, 0x40);
		/* wait approximately 100 us - calibration in progress */
		udelay(100);  
		/* clear bit ClkQCalib for further calibration */
		PCD_bitclr(RegClockQControl, 0x40); 
		PCD_write(RegBitPhase, 0xAD);      
		PCD_write(RegRxThreshold, 0xFF);
		#if 0
		PCD_write(RegRxControl2, 0x41);
		PCD_write(RegRxControl1, 0x72);
		#endif
		PCD_write(RegFIFOLevel, 0x04);   
		PCD_write(RegTimerControl, 0x02);
		PCD_write(RegIRqPinConfig, 0x03); 
		PCD_write(RegTxControl, 0x5b); 
		PCD_set_timeout(1);
		PCD_antenna(1);
	}
	#endif
	return ret;
}

/* 
 * 获取RC500信息
 */
void PCD_get_info(void)
{
	int i;
	unsigned char product_info[16];

	PCD_write(RegFIFOData, 0);
	PCD_write(RegFIFOData, 0);
	PCD_write(RegFIFOData, 16);
	PCD_write(RegCommand, PCD_READE2);

	for(i=0; i<16; i++) {
		printk("[%d]", PCD_read(RegFIFOLength));
		product_info[i] = PCD_read(RegFIFOData);
		printk("%02X ", product_info[i]);
	}
	printk("\n");

	printk("PID: %08X\n", *(unsigned int *)product_info);
	printk("VER: %02X\n", product_info[4]);
	printk("PSN: %08X\n", *(unsigned int *)(product_info+8));

	#define _DEBUG_
	#ifdef _DEBUG_
	struct timespec clock[2];
	for(i=1; i<8; i++) {
		PCD_set_timeout(i);
		PCD_write(RegInterruptEn, 0x7f);
		PCD_write(RegInterruptRq, 0x7f);
		clock[0] = current_kernel_time();
		PCD_bitset(RegControl, 0x02);
		printk("tmr:%d\n", PCD_read(RegTimerValue));
		printk("rld:%d\n", PCD_read(RegTimerReload));
		printk("ctl:%d\n", PCD_read(RegTimerControl));
		int cnt;
		int sta;
		cnt = 0;
		while(++cnt) {
			sta = PCD_read(RegInterruptRq);
			if(sta&0x20) {
				break;
			}
		}
		clock[1] = current_kernel_time();
		printk("clk0:%d %d\n", clock[0].tv_sec, clock[0].tv_nsec);
		printk("clk1:%d %d\n", clock[1].tv_sec, clock[1].tv_nsec);
		printk("irq:%02X\n", sta);
		PCD_bitclr(RegControl, 0x02);
		printk("%d\n", cnt);
	}
	#endif /* _DEBUG_ */
}

int PCD_cmd(struct MF_CMD_INFO *cmd_info, unsigned char *buf)
{
	int status = 0;
	int guard_cnt;
	int cnt;
	unsigned char irq_en; /* RC500中断源使能 */
	unsigned char irq_sr; /* RC500中断源 */
	unsigned char irq_mask; /* RC500中断使能状态 */
	unsigned char irq_bits; /* RC500中断发生状态 */
	unsigned char err_flg; /* RC500错误标志 */
	unsigned char wait_for;

	if(cmd_info->len > FSD) {
		return -1;
	}

	switch(cmd_info->cmd) {
	case PCD_STARTUP:
	case PCD_IDLE:
		irq_en = 0x00;
		wait_for = 0x00;
		break;
	case PCD_WRITEE2:
		irq_en = 0x11;
		wait_for = 0x10;
		break;
	case PCD_READE2:
		irq_en = 0x07;
		wait_for = 0x04;
		break;
	case PCD_AUTHENT2:
		irq_en = 0x04;
		wait_for = 0x04;
		break;
	case PCD_CALCCRC:
		irq_en = 0x11;
		wait_for = 0x10;
		break;
	case PCD_RECEIVE:
		irq_en = 0x06;
		wait_for = 0x04;
		break;
	case PCD_LOADCONFIG:
	case PCD_LOADKEYE2:
	case PCD_LOADKEY:
	case PCD_AUTHENT1:
	case PCD_TRANSMIT: /* LoAlert and IdleIRq */
		irq_en = 0x05;
		wait_for = 0x04;
		break;
	case PCD_TRANSCEIVE: /* TxIrq, RxIrq, IdleIRq and LoAlert */
		irq_en = 0x3D;
		wait_for = 0x04;
		break;
	default:
		status = -1;
		break;
	}

	if(status == 0) {
		irq_sr = 0;
		irq_en |= 0x20 | 0x80; /* always enable timout irq */
		wait_for |= 0x20; /* always wait for timeout */

		PCD_write(RegInterruptEn, 0x7F); /* disable all interrupts */
		PCD_write(RegInterruptRq, 0x7F); /* reset interrupt requests */
		PCD_write(RegCommand, PCD_IDLE); /* terminate probably running command */
		PCD_flush_fifo(); /* flush FIFO buffer */
		PCD_write(RegInterruptEn, irq_en);
		for(cnt=0; cnt<cmd_info->len; cnt++) {
			PCD_write(RegFIFOData, buf[cnt]);
		}
		PCD_write(RegCommand, pcd_cmd); /* start command */

		guard_cnt = 300000;
		while((guard_cnt--) || !(irq_sr&wait_for)) {
			irq_sr = PCD_read(RegInterruptRq);
		}
		if(guard_cnt) {
			err_flg = PCD_read(RegErrorFlag);
			cmd_info->len = PCD_read(RegFIFOLength);
			for(cnt=0; cnt<cmd_info->len; cnt++) {
				buf[cnt] = PCD_read(RegFIFOData);
			}
			if(err_flg & 0x17) {
				status = -1;
				if(err_flg & 0x01) { /* collision detected */
					cmd_info->coll_pos = PCD_read(RegCollpos);
				}
			}else if(pcd_cmd == PCD_TRANSCEIVE) {
				/* number of bits in the last byte */
				cmd_info->recv_bit = ReadIO(RegSecondaryStatus) & 0x07;
			}
		}else {
			status = -1;
		}

		PCD_write(RegInterruptEn, 0x7F); 
		PCD_write(RegInterruptRq, 0x7F);
		PCD_bitset(RegControl, 0x04); /* stop timer */
		PCD_write(RegCommand, PCD_IDLE); 
	}

	return status;
}

int PCD_read_E2(unsigned short adr, unsigned char len, unsigned char *dat)
{
	int i;

	PCD_write(RegFIFOData, adr&0xff);
	PCD_write(RegFIFOData, (adr>>8)&0xff);
	PCD_write(RegFIFOData, len);
	PCD_write(RegCommand, PCD_READE2);

	for(i=0; i<len; i++) {
		dat[i] = PCD_read(RegFIFOData);
		printk("%02X ", dat[i]);
	}

	return 0;
}


int PCD_write_E2(unsigned short adr, unsigned char len, unsigned char *dat)
{
	int i;

	PCD_write(RegFIFOData, adr&0xff);
	PCD_write(RegFIFOData, (adr>>8)&0xff);
	PCD_write(RegFIFOData, len);
	PCD_write(RegCommand, PCD_WRITEE2);

	for(i=0; i<len; i++) {
		PCD_write(RegFIFOData, dat[i]);
	}

	return 0;
}


