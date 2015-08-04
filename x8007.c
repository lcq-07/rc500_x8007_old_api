/*
 * Filename      : x8007.c
 * Author        : nuc0707#163#com
 * Create time   : 2015-06-26 10:57
 * Modified      : append in the following formate
 * year-month-day description
 * ISO-7816 智能卡控制器驱动
 */

#include <linux/fs.h>
#include "gpio_bus_comm.h"
#include "rc500.h"
#include "x8007.h"

#define PSAM_STA_OK	0
#define PSAM_STA_ERR	1

static char psam_sta; 
#if 1
#define TDA8007B_DEBUG
#endif

static inline void tda8007b_select_card(char num)
{
	char csr;
	csr = gpio_bus_read(DEV_TDA8007B, TDA8007B_CSR);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_CSR, (1<<((num&0x3)-1))|0x08);
}
static inline void tda8007b_reset_uart(void)
{
	char csr;
	csr = gpio_bus_read(DEV_TDA8007B, TDA8007B_CSR);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_CSR, csr&0xf0);
}

static void tda8007b_card_deactivated(void)
{
	char pcr;
	pcr = gpio_bus_read(DEV_TDA8007B, TDA8007B_PCR);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_PCR, pcr&~0x3f);
}

static int tda8007b_card_activated(void)
{
	char pcr;
	pcr = gpio_bus_read(DEV_TDA8007B, TDA8007B_PCR);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_PCR, pcr&~0x01);
	return 0;
}

static int tda8007b_send_byte(char dat)
{
	char rd;
	int i;

	rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_UCR1);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_UCR1, (rd&~0x04)|0x08);

	i = 0;
	do{
		udelay(10);
		if(++i > 1000) {
			rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
#ifdef TDA8007B_DEBUG
			printk("\33[35m[TDA8007B-MSR:%.02x]send byte can not wait 8007 ready\n\33[0m", rd);
#endif
			return (-1);
		}
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
	}while(!(rd & 0x10));

	gpio_bus_write(DEV_TDA8007B, TDA8007B_UTR, dat);
	i = 0;
	do{
		//udelay(10);
		if(++i > 1000) {
			return (-2);
		}
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_USR);
	}while(!(rd&0x01));
	udelay(10);
//#ifdef TDA8007B_DEBUG
#if 0
	printk("\33[35m[%.02x] send\n\33[0m", dat);
#endif
	return 0;
}

static int tda8007b_send_last_byte(char dat)
{
	char rd;
	int i;

	i = 0;
	do{
		udelay(10);
		if(++i > 1000) {
			rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
#ifdef TDA8007B_DEBUG
			printk("\33[35m[TDA8007B-MSR:%.02x]send byte can not wait 8007 ready\n\33[0m", rd);
#endif
			return (-1);
		}
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
	}while(!(rd & 0x10));

	rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_UCR1);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_UCR1, rd|0x04|0x08);

	gpio_bus_write(DEV_TDA8007B, TDA8007B_UTR, dat);
#if 0	
	i = 0;
	do{
		if(++i > 1000) {
			return (-2);
		}
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_USR);
	}while(!(rd&0x01));
#endif
	//while(!(gpio_bus_read(DEV_TDA8007B, TDA8007B_USR) & 0x01));

	udelay(10);
//#ifdef TDA8007B_DEBUG
#if 0
	printk("\33[35m[%.02x] last send\n\33[0m", dat);
#endif
	return 0;
}

static int tda8007b_recv_byte(char *dat)
{
	int rd;
/*
	int i;
	//rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_UCR1);
	//gpio_bus_write(DEV_TDA8007B, TDA8007B_UCR1, rd&~0x08);
	do{
		if(++i > 100) {
			rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
#ifdef TDA8007B_DEBUG
			printk("\33[35m[MSR:%.02x]set timer can not wait 8007 ready\n\33[0m", rd);
#endif
			return (-1);
		}
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
	}while(!(rd & 0x10));
*/
	/*
	 * 20141018
	 * 修改接收FIFO深度后，接收直接判断FIFO是否为空
	 */
	rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
	if((rd & 0x40)) {
		rd = -1;
	}else {
		*dat = gpio_bus_read(DEV_TDA8007B, TDA8007B_URR);
		//udelay(200);
#ifdef TDA8007B_DEBUG
		printk("\33[35m[%.02x] recv\n\33[0m", *dat);
#endif
		rd = 0;
	}
	return rd;
}

static int tda8007b_set_timer(char toc, char tor1, char tor2, char tor3)
{
	int i;
	char rd;

	i = 0;
	do{
		if(++i > 100) {
			rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
#ifdef TDA8007B_DEBUG
			printk("\33[35m[TDA8007B-MSR:%.02x]set timer can not wait 8007 ready\n\33[0m", rd);
#endif
			return (-1);
		}
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
	}while(!(rd & 0x10));
	gpio_bus_write(DEV_TDA8007B, TDA8007B_TOC, 0x00);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_TOR1, tor1);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_TOR2, tor2);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_TOR3, tor3);
	i = 0;
	do{
		if(++i > 100) {
			rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
#ifdef TDA8007B_DEBUG
			printk("\33[35m[TDA8007B-MSR:%.02x]set timer can not wait 8007 ready\n\33[0m", rd);
#endif
			return (-1);
		}
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
	}while(!(rd & 0x10));
	gpio_bus_write(DEV_TDA8007B, TDA8007B_TOC, toc);

	return 0;
}

void tda8007b_init(void)
{
	char rd;
	char warm_rst_ctl;
	char ATR[64];
	char GTR;
	char done;
	char historicalBytes = 0;
	char expectedCharacters = 0;
	int idx;
	int i;
	int ret;
	int TAn;
	int TBn;
	int TCn;
	int TDn;

	psam_sta = PSAM_STA_ERR;

	tda8007b_select_card(1);
	tda8007b_card_deactivated();
	tda8007b_select_card(2);
	tda8007b_card_deactivated();

	tda8007b_select_card(1);
	tda8007b_reset_uart();
	tda8007b_select_card(2);
	tda8007b_reset_uart();

	//powerup and activate
	tda8007b_select_card(1);
	rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_CSR);
#ifdef TDA8007B_DEBUG
	printk("\33[35m[TDA8007B-CSR:%.2x]\n\33[0m", rd);
#endif
	
	// Compile time setting of operating crystal frequency
#if ((CRYSTAL_FREQUENCY_8007 >= 1000000L) && (CRYSTAL_FREQUENCY_8007 <= 5000000L))
	// Set smartcard clock to 1/1 crystal
  	gpio_bus_write(DEV_TDA8007B, TDA8007B_CCR, 0x00);
#elif ((CRYSTAL_FREQUENCY_8007 >= 2000000L) && (CRYSTAL_FREQUENCY_8007 <= 10000000L))
	// Set smartcard clock to 1/2 crystal
	gpio_bus_write(DEV_TDA8007B, TDA8007B_CCR, 0x01);
#elif ((CRYSTAL_FREQUENCY_8007 >= 4000000L) && (CRYSTAL_FREQUENCY_8007 <= 20000000L))
	// Set smartcard clock to 1/4 crystal
	gpio_bus_write(DEV_TDA8007B, TDA8007B_CCR, 0x02);
#elif ((CRYSTAL_FREQUENCY_8007 >= 8000000L) && (CRYSTAL_FREQUENCY_8007 <= 40000000L))
	// Set smartcard clock to 1/8 crystal
  	gpio_bus_write(DEV_TDA8007B, TDA8007B_CCR, 0x03);
#else
	// Set smartcard clock to 1/2 internal oscillator (about 1.44MHz)
  	// NOTE: Can only change CCR.2 when shifting to internal oscillator
	gpio_bus_write(DEV_TDA8007B, TDA8007B_CCR, 0x01);
  	gpio_bus_write(DEV_TDA8007B, TDA8007B_CCR, 0x05);


	// Wait for internal oscillator to engage (check the MSR.CLKSW bit on page 18)
	i = 0
	while(1) {
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
		if(rd & 0x80) {
			break;
		}
		if(++i>100) {
			printk("\33[35m[8007-MSR:%.02x]clock problem detected\n\33[0m", rd);
			return;
		}
		mdelay(1);
	}
#endif
	warm_rst_ctl = 0;
warm_rst:
	i = 0;
	do {
		if(++i>1000) {
			return;
		}
		//C8-HI C4-HI RST-LO 5V START
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_PCR);
		//gpio_bus_write(DEV_TDA8007B, TDA8007B_PCR, (rd&~0x3e)|0x01);//5V
		gpio_bus_write(DEV_TDA8007B, TDA8007B_PCR, (rd&~0x3e)|0x3);//3V
		//gpio_bus_write(DEV_TDA8007B, TDA8007B_PCR, (rd&~0x3e)|0x9);//1.8V
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_HSR);
		if(rd & (0x6d)) {
			printk("\33[35m[TDA8007B-HSR:%.02x]power problem detected\n\33[0m", rd);
			return;
		}
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_PCR);
	}while((!(rd&0x01)));
	tda8007b_select_card(1);
	tda8007b_reset_uart();
	tda8007b_select_card(1);
	/*
	 * 20141019
	 * 修改FIFO深度为8字节，解决接收接收溢出问题
	 */
	gpio_bus_write(DEV_TDA8007B, TDA8007B_FCR, 0x07);//FIFO 8 byte
	gpio_bus_write(DEV_TDA8007B, TDA8007B_PDR, 12);//divisor
	gpio_bus_write(DEV_TDA8007B, TDA8007B_UCR2, 0x00);//prescaler, auto convention
	gpio_bus_write(DEV_TDA8007B, TDA8007B_UCR1, 0x02);
	//initial etu=372/f=0.1ms
	//work etu=1/D*F/f
	//ATR interval (400~40000 clk)
	//baute 8065 bit/s
	//set timer to wait powerup
	mdelay(40);
	//RST-HI
	rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_PCR);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_PCR, rd|0x04);
	rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_PCR);
#ifdef TDA8007B_DEBUG
	printk("\33[35m[TDA8007B-PCR:%.02x]\n\33[0m", rd);
#endif
	//gpio_bus_write(DEV_TDA8007B, TDA8007B_GTR, 0xff);
	//毅能达
	gpio_bus_write(DEV_TDA8007B, TDA8007B_GTR, 0x00);
	//wait ATR
	
	i = 0;
	do {
		ret = tda8007b_set_timer(0x61, 0x00, 0xe8, 0x03);//1000ETU
	}while((ret<0)&&(++i<2));
	while(1){
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
		if(rd & 0x01) {
			break;
		}
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_USR);
		if(rd & 0x1e) {
			printk("\33[35m[%d][TDA8007B-USR:%.02x] error\n\33[0m", __LINE__, rd);
			if(warm_rst_ctl == 0) {
				warm_rst_ctl++;
				goto warm_rst;
			}
			return;
		}
		if(rd & 0x80) {
			printk("\33[35m[%d]wait ATR timeout\n\33[0m", __LINE__);
			if(warm_rst_ctl == 0) {
				warm_rst_ctl++;
				goto warm_rst;
			}
			return;
		}
		//udelay(500);
		udelay(500);
		if(++i>1000) {
			return;
		}
	}
	//get ATR
	printk("\33[35mwait: %d\nATR: \n\33[0m", i);
	//set recv timer
	
	i = 0;
	idx = 0;
	GTR = 0;
	TAn = 0;
	TBn = 0;
	TCn = 0;
	TDn = 0;
	done = 0;
	while (!done) {
		rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_MSR);
		if(!(rd & 0x40)) {
			rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_URR);
			ATR[idx++] = rd;
			if (idx == 1) {
				printk("\33[35mTS : %02x\n\33[0m", rd);
				if((rd != 0x3f) && (rd != 0x3b)) {
#ifdef TDA8007B_DEBUG
					printk("\33[35m[%d]wrong\n\33[0m", __LINE__);
#endif
					return;
				}
			}

			if (idx == 2) {
				historicalBytes = rd & 0x0F;
				expectedCharacters = rd & 0xF0;
				printk("\33[35mT0 : %.02x\n\33[0m", rd);
			}

			if (idx > 2){
				switch(expectedCharacters){
				case 0x00:
					// Historical characters
					historicalBytes--;
					if (historicalBytes == 0){
						done = 1;
					}
					printk("\33[35mHIS: %.02x\n\33[0m", rd);
					break;
				case 0x01:
					// TCK case
					done = 1;
					printk("\33[35mTCK: %.02x\n\33[0m", rd);
					break;

				case 0x10:
				case 0x30:
				case 0x50:
				case 0x70:
				case 0x90:
				case 0xB0:
				case 0xD0:
				case 0xF0:
					// TA case
					expectedCharacters &= 0xE0;
					printk("\33[35mTA%d: %.02x\n\33[0m", TAn, rd);
					TAn += 1;
					break;

				case 0x20:
				case 0x60:
				case 0xA0:
				case 0xE0:
					// TB case
					expectedCharacters &= 0xD0;
					printk("\33[35mTB%d: %.02x\n\33[0m", TBn, rd);
					TBn += 1;
					break;

				case 0x40:
				case 0xC0:
					// TC case
					expectedCharacters &= 0xB0;
					if(TCn == 1) {
						GTR = rd;
					}
					printk("\33[35mTC%d: %.02x\n\33[0m", TCn, rd);
					TCn += 1;
					break;

				case 0x80:
					// TD case
					expectedCharacters=(rd&0xF0);
					printk("\33[35mTD%d: %.02x\n\33[0m", TDn, rd);
					TDn += 1;
					if ((expectedCharacters == 0x00) && (historicalBytes == 0)){
						done = 1;
					}
					break;
				default:
					return;
				}
			}
		}else {
			//udelay(10);
			if(++i > 20000) {
#ifdef TDA8007B_DEBUG
				printk("\33[35m[%d]recv ATR abort\n\33[0m", __LINE__);
#endif
				done = 1;;
			}
		}
	}
	if(idx<3) {
		return;
	}
	//set pram accord ATR
	rd = gpio_bus_read(DEV_TDA8007B, TDA8007B_UCR1);
#ifdef TDA8007B_DEBUG
	printk("\33[35m[TDA8007B-UCR1:%.02x]\n\33[0m", rd);
#endif
	gpio_bus_write(DEV_TDA8007B, TDA8007B_UCR1, rd&~0x10);
	gpio_bus_write(DEV_TDA8007B, TDA8007B_GTR, GTR);

	psam_sta = PSAM_STA_OK;
}


int tda8007b_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	int i;
	int ret;
	int cnt;
	int apdu_len;
	int apdu_Le;
	int apdu_Lc;
	char *apdu_buf;
	char apdu_INS;
	char procedure_byte = 0;

	if(psam_sta == PSAM_STA_ERR) {
problem_occur:
		command.MfLength = 2;
		command.MfData[0] = 0xff;
		command.MfData[1] = 0xff;
		psam_sta = PSAM_STA_ERR;
		return count;
	}

	apdu_len = buf[3];
	apdu_buf = &buf[4];
	apdu_INS = apdu_buf[1];
	apdu_Lc  = apdu_buf[4];
	apdu_Le  = apdu_buf[apdu_len-1];
	if(apdu_len>=5) {
		if((apdu_len == 5) || (apdu_len == 6)) {
			apdu_Lc = 0;
		} else if((apdu_len-5) == apdu_Lc) {
			apdu_Le = 0;
		}
	} else {
			return -1;
	}

#ifdef TDA8007B_DEBUG
	printk("\33[35m[INS:%.02x][Lc:%.02x][Le:%.02x][APDU_Len:%d]\n\33[0m", apdu_INS, apdu_Lc, apdu_Le, apdu_len);
#endif
#if 1
	for(cnt=0; cnt<4; cnt++) {
		ret = tda8007b_send_byte(apdu_buf[cnt]);
		if(ret != 0) {
			return -1;
		}
	}
	ret = tda8007b_send_last_byte(apdu_buf[cnt++]);
#ifdef TDA8007B_DEBUG
	printk("send last byte ok\n");
#endif
	i = 0;
	do{
		ret = tda8007b_recv_byte(&procedure_byte);
		//if((ret==0)&&(procedure_byte==apdu_INS)) {
		if((ret==0)) {
#ifdef TDA8007B_DEBUG
			printk("procedure byte:%02x\n", procedure_byte);
#endif
			if(procedure_byte == apdu_INS) {
			/* procedure byte is INS, next send the reast data */
				command.MfLength = 0;
				break;
			}else if(procedure_byte == 0x60) {
			/* procedure byte is 0x60, recv new procedure byte */
				continue;
			
			}else if(((procedure_byte&0xF0)==0x60)||((procedure_byte&0xF0)==0x90)) {
			/* procedure byte is SW1, next recv SW2 */
				apdu_Le = 0;
				apdu_len = 5;
				command.MfLength = 0;
				command.MfData[command.MfLength++] = procedure_byte;
				break; 
			}else if(procedure_byte==(apdu_INS^0xFF)) {
			/* procedure byte is INS xor 0xff, send one of reast data and recv new procedure byte */
				ret = tda8007b_send_last_byte(apdu_buf[cnt++]);
				continue;
			}
		}
	}while((++i<30000)&&(cnt<=apdu_len));
#else
	if(apdu_len>5) {
		for(cnt=0; cnt<5; cnt++) {
			ret = tda8007b_send_byte(apdu_buf[cnt]);
			if(ret != 0) {
				goto problem_occur;
			}
		}
	}else {
		for(cnt=0; cnt<apdu_len-1; cnt++) {
			ret = tda8007b_send_byte(apdu_buf[cnt]);
			if(ret != 0) {
				goto problem_occur;
			}
		}
	}
#endif
	mdelay(5);
	for(; cnt<(apdu_len-1); cnt++) {
		ret = tda8007b_send_byte(apdu_buf[cnt]);
		if(ret != 0) {
			goto problem_occur;
		}
	}
	if((apdu_len>5)&&(cnt<apdu_len)) {
		ret = tda8007b_send_last_byte(apdu_buf[cnt]);
	}

	cnt = 0;
	do {
		ret = tda8007b_recv_byte(&command.MfData[command.MfLength]);
		if(ret == 0) {
			command.MfLength++;
			if(((apdu_Le==0)&&(command.MfLength==2)) || ((apdu_Le+3)==command.MfLength)) {
				break;
			}
		}else {
			//udelay(100);
			//udelay(50);
		}
	}while(++cnt<30000);
#ifdef TDA8007B_DEBUG
	printk("\33[35mtry[%d]\n\33[0m", cnt);
#endif
/*
	if((command.MfData[0]==apdu_INS) && (command.MfLength>2)) {
		command.MfLength -= 1;
		memcpy(&command.MfData[0], &command.MfData[1], command.MfLength);
	}
*/
	/*
	 * 20141019 
	 * 下面延时用于解决8007两次发送间隔过短造成部分发送失败的问题 
	 */
	mdelay(2);

	return count;
}

int tda8007b_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	return 0;
}

int tda8007b_release(struct inode *inode, struct file *filp)
{
	tda8007b_select_card(1);
	tda8007b_card_deactivated();
	tda8007b_select_card(2);
	tda8007b_card_deactivated();

	gpio_bus_release();
	return 0;
}

int tda8007b_open(struct inode *inode, struct file *filp)
{
	tda8007b_init();
	return 0;
}

int tda8007b_close(struct inode *inode,struct file *filp)
{
	tda8007b_select_card(1);
	tda8007b_card_deactivated();
	tda8007b_select_card(2);
	tda8007b_card_deactivated();

	return 0;
}

