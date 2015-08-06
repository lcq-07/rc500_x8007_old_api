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
#include "debug_print.h"
#include "gpio_bus_comm.h"
#include "rc500.h"
#include "ISO14443_3A.h"
#include "ISO14443_4A.h"

unsigned char PCD_INFO[16];
unsigned char PCD_BUF[FSD];
struct MF_CMD_INFO PCD_CMD;

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
	#if 1
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
		PCD_write(RegCwConductance, 0x3f);
		PCD_set_timeout(1);
		PCD_antenna(1);
	}
	#endif
	return ret;
}

int PCD_cmd(struct MF_CMD_INFO *cmd_info, unsigned char *buf)
{
	int status = 0;
	int guard_cnt;
	int cnt;
	unsigned char irq_en; /* RC500中断源使能 */
	unsigned char irq_sr; /* RC500中断源 */
	unsigned char err_flg; /* RC500错误标志 */
	unsigned char wait_for;

	if(cmd_info->len > FSD) {
		return -1;
	}

	PCD_write(RegInterruptEn, 0x7F); /* disable all interrupts */
	PCD_write(RegInterruptRq, 0x7F); /* reset interrupt requests */
	PCD_write(RegCommand, PCD_IDLE); /* terminate probably running command */
	PCD_flush_fifo(); /* flush FIFO buffer */

	switch(cmd_info->cmd) {
	case PCD_STARTUP:
	case PCD_IDLE:
		irq_en   = PCD_IRQ_TIMER;
		wait_for = PCD_IRQ_TIMER;
		break;
	case PCD_WRITEE2:
		irq_en   = PCD_IRQ_TIMER | PCD_IRQ_TX | PCD_IRQ_LOALERT;
		wait_for = PCD_IRQ_TIMER | PCD_IRQ_TX;
		break;
	case PCD_READE2:
		irq_en   = PCD_IRQ_TIMER | PCD_IRQ_IDLE | PCD_IRQ_HIALERT | PCD_IRQ_LOALERT;
		wait_for = PCD_IRQ_TIMER | PCD_IRQ_IDLE;
		break;
	case PCD_AUTHENT2:
		irq_en   = PCD_IRQ_TIMER | PCD_IRQ_IDLE;
		wait_for = PCD_IRQ_TIMER | PCD_IRQ_IDLE;
		break;
	case PCD_CALCCRC:
		irq_en   = PCD_IRQ_TIMER | PCD_IRQ_TX | PCD_IRQ_LOALERT;
		wait_for = PCD_IRQ_TIMER | PCD_IRQ_TX;
		break;
	case PCD_RECEIVE:
		irq_en   = PCD_IRQ_TIMER | PCD_IRQ_IDLE | PCD_IRQ_HIALERT;
		wait_for = PCD_IRQ_TIMER | PCD_IRQ_IDLE;
		break;
	case PCD_LOADCONFIG:
	case PCD_LOADKEYE2:
	case PCD_LOADKEY:
	case PCD_AUTHENT1:
	case PCD_TRANSMIT:
		irq_en   = PCD_IRQ_TIMER | PCD_IRQ_IDLE | PCD_IRQ_LOALERT;
		wait_for = PCD_IRQ_TIMER | PCD_IRQ_IDLE;
		break;
	case PCD_TRANSCEIVE:
		irq_en   = PCD_IRQ_TIMER | PCD_IRQ_TX | PCD_IRQ_RX | PCD_IRQ_IDLE | PCD_IRQ_LOALERT;
		wait_for = PCD_IRQ_TIMER | PCD_IRQ_IDLE;
		break;
	default:
		status = -1;
		break;
	}

	if(status == 0) {
		irq_sr = 0;
		PCD_write(RegInterruptEn, irq_en|0x80);
		DEBUG_PRINT("W_PCD[% 2d]>> ", cmd_info->len);
		for(cnt=0; cnt<cmd_info->len; cnt++) {
			PCD_write(RegFIFOData, buf[cnt]);
			DEBUG_PRINT("%02X", buf[cnt]);
		}
		DEBUG_PRINT("\n");
		PCD_write(RegCommand, cmd_info->cmd); /* start command */

		guard_cnt = 80000; /* about 800ms */
		do{
			irq_sr = PCD_read(RegInterruptRq);
			if(irq_sr & wait_for) {
				break;	
			}
		}while(--guard_cnt);
		err_flg = PCD_read(RegErrorFlag);
		cmd_info->len = PCD_read(RegFIFOLength);
		DEBUG_PRINT("IRQ_SR   : %02X\n", irq_sr);
		DEBUG_PRINT("ERR_FLG  : %02X\n", err_flg);
		DEBUG_PRINT("FIFOLEN  : % 2d\n", cmd_info->len);
		DEBUG_PRINT("WAIT_FOR : %02X\n", wait_for);
		DEBUG_PRINT("GUARD_CNT: %d\n", guard_cnt);
		if(guard_cnt) {
			DEBUG_PRINT("R_PCD[% 2d]<< ", cmd_info->len);
			for(cnt=0; cnt<cmd_info->len; cnt++) {
				buf[cnt] = PCD_read(RegFIFOData);
				DEBUG_PRINT("%02X", buf[cnt]);
			}
			DEBUG_PRINT("\n");
			if(irq_sr & 0x20) { /* timeout */
				status = -1;
			}
			if(err_flg & 0x17) {
				status = -1;
				if(err_flg & 0x01) { /* collision detected */
					cmd_info->coll_pos = PCD_read(RegCollPos);
				}
			}else if(cmd_info->cmd == PCD_TRANSCEIVE) {
				/* number of bits in the last byte */
				cmd_info->recv_bit = PCD_read(RegSecondaryStatus) & 0x07;
				if(cmd_info->recv_bit && cmd_info->len) {
					cmd_info->len -= 1;
				}
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
	int ret;

	if(len>FSD) {
		return -1;
	}

	PCD_CMD.cmd = PCD_READE2;
	PCD_CMD.len = 3;
	PCD_BUF[0] = adr&0xff;
	PCD_BUF[1] = (adr>>8)&0xff;
	PCD_BUF[2] = len;

	ret = PCD_cmd(&PCD_CMD, PCD_BUF);
	if(PCD_CMD.len < len) {
		ret = -1;
	}
	if(ret == 0) {
		for(i=0; i<PCD_CMD.len; i++) {
			dat[i] = PCD_BUF[i];
		}
	}

	return ret;
}


int PCD_write_E2(unsigned short adr, unsigned char len, unsigned char *dat)
{
	int i;
	int ret;

	if(len>(FSD-3)) {
		return -1;
	}

	PCD_CMD.cmd = PCD_WRITEE2;
	PCD_CMD.len = 3+len;
	PCD_BUF[0] = adr&0xff;
	PCD_BUF[1] = (adr>>8)&0xff;
	PCD_BUF[2] = len;

	for(i=0; i<PCD_CMD.len; i++) {
		PCD_BUF[i+3] = dat[i];
	}
	ret = PCD_cmd(&PCD_CMD, PCD_BUF);

	return ret;
}

/* 
 * 获取RC500信息
 */
int PCD_get_info(void)
{
	int ret;

	ret = PCD_read_E2(0, 16, PCD_INFO);

	DEBUG_PRINT("PID: %08X\n", *(unsigned int *)PCD_INFO);
	DEBUG_PRINT("VER: %02X\n", PCD_INFO[4]);
	DEBUG_PRINT("PSN: %08X\n", *(unsigned int *)(PCD_INFO+8));

	#if 0
	struct timespec clock[2];
	for(i=1; i<8; i++) {
		PCD_set_timeout(i);
		PCD_write(RegInterruptEn, 0x7f);
		PCD_write(RegInterruptRq, 0x7f);
		clock[0] = current_kernel_time();
		PCD_bitset(RegControl, 0x02);
		DEBUG_PRINT("tmr:%d\n", PCD_read(RegTimerValue));
		DEBUG_PRINT("rld:%d\n", PCD_read(RegTimerReload));
		DEBUG_PRINT("ctl:%d\n", PCD_read(RegTimerControl));
		int cnt;
		cnt = 0;
		while(++cnt) {
			sta = PCD_read(RegInterruptRq);
			if(sta&0x20) {
				break;
			}
		}
		clock[1] = current_kernel_time();
		DEBUG_PRINT("clk0:%d %d\n", clock[0].tv_sec, clock[0].tv_nsec);
		DEBUG_PRINT("clk1:%d %d\n", clock[1].tv_sec, clock[1].tv_nsec);
		DEBUG_PRINT("irq:%02X\n", sta);
		PCD_bitclr(RegControl, 0x04);
		DEBUG_PRINT("%d\n", cnt);
	}
	#endif /* _DEBUG_ */

	return ret;
}

/* RC500总入口 */
int sendtoRC500(TranSciveBuffer *send, TranSciveBuffer *receive)
{
	int ret = 0;
  
	receive->MfCommand = send->MfCommand;
	receive->MfCtlFlag = send->MfCtlFlag;
	receive->MfSector  = send->MfSector;
	switch(send->MfCommand) {
	case 0x10: /* 初始化RC500 */
		ret = PCD_init();
		if(ret == 0) {
			receive->MfLength = 0x0;
			receive->MfStatus = 0x00;
		}
		break;
	case 0x36: /* 响应天线命令 */
		PCD_antenna(send->MfCtlFlag);
		ret = 0x00;
		receive->MfLength = 0x0;
		receive->MfStatus = 0x00;
		break;
	case 0x37: /* 响应寻A卡命令，返回ATQ */
		ret = PICC_request(send->MfCtlFlag, receive->MfData);
		if(ret == 0x00) {
			receive->MfLength = 0x2;
			receive->MfStatus = 0x00;
		}
		break;
	case 0x38: /* 响应A卡防冲突命令，返回UID */
		ret = PICC_anticoll(receive->MfData); /*  则执行防冲突检测*/
		if(ret == 0x00) {
			receive->MfLength = 0x4;
			receive->MfStatus = 0x00;
		}
		break;
	case 0x39: /* 响应A卡锁定命令，返回SAK */
		ret = PICC_select(send->MfData, receive->MfData);
		if(ret == 0x00){
			receive->MfLength = 0x1;
			receive->MfStatus = 0x00;
		}
		break;
	case 0x29: /* 响应A卡HALT命令*/
		ret = PICC_halt();
		if(ret == 0x00) {
			receive->MfLength = 0x0;
			receive->MfStatus = 0x00;
		}
		break;
	case 0x4A:  /* 响应A卡密钥验证命令 */
		ret = PICC_MFauth(send->MfCtlFlag, send->MfSector, send->MfData);
		if(ret == 0x00) {
			receive->MfLength = 0x0;
			receive->MfStatus = 0x00;
		}
		break; 
	case 0x4B: /* 响应读M1卡命令 */
		ret = PICC_MFread(send->MfSector, receive->MfData);
		if(ret == 0x00) {
			receive->MfLength = 0x10;
			receive->MfStatus = 0x00;
		}
		break;
	case 0x4C: /*  响应写M1卡命令 */
		ret = PICC_MFwrite(send->MfSector, send->MfData);
		if(ret == 0x00) {
			receive->MfLength = 0x0;
			receive->MfStatus = 0x00;
		}
		break;
	#if 0
	case 0x4D: /*  响应初始化钱包命令,写M1卡块 */
	ComM1Initval();
		break; 
					
	case 0x4E: /*  响应读钱包命令 */
	ComM1Readval();
		break;
					
	case 0x4F: /*  响应扣款命令 */
	ComM1Decrement();
		break;
					
	case 0x50: /*  响应充值命令 */
	ComM1Increment();
		break;
					
	case 0x51: /*  响应M1卡备份钱包命令*/
	  ComM1BakValue();
		break;
	case 0x52:  /* 响应UltraLight卡的防冲突命令 */
		ComUL_PcdAnticoll();
		break;
					
	case 0x53: /*  响应写UltraLight卡命令*/
		if(0x00 == UL_PcdWrite(send->MfSector,send->MfData))
			ret = 0;
		else
			ret = -1;
		break;  
	#endif						
	case 0x54: /* RATS命令 */
		ret = PICC_rats(receive->MfData, &receive->MfLength);
		if(ret == 0x00) {
			receive->MfStatus = 0x00;
		}
		break; 
	case 0x55: /* 响应T=CL卡COS命令*/
		ret = PICC_tcl(send->MfData, receive->MfData, &send->MfLength);
		if(ret == 0x00) {
			receive->MfLength = send->MfLength;
			receive->MfStatus = 0x00;
		}else {
			receive->MfLength = 0x00;
		}
		break;
	case 0x56: /* DESELECT命令 */
		ret = PICC_deselect();
		break;
	default:
		ret = -1;
		break;

	}  
	return ret;
}


