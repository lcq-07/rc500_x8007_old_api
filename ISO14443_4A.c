/*
 * Filename      : ISO14443_4A.c
 * Author        : nuc0707#163#com
 * Create time   : 2015-06-26 11:01
 * Modified      : append in the following formate
 * year-month-day description
 * 2015-07-23 nuc0707#163#com 本实现中所有CID均为0且
 * 块前导字段均包含CID
 */



#include <linux/kernel.h>
#include "rc500.h"
#include "ISO14443_4A.h"

unsigned int FSC = 0; /* 卡片处理帧长度 */
unsigned char block_number;

int PICC_rats(unsigned char *pATS, int *ATS_len)
{
	int ret;

	block_number = 0;
	PCD_set_timeout(5);
	PCD_CMD.cmd = PCD_TRANSCEIVE;
	PCD_CMD.len = 2;
	PCD_BUF[0] = 0xE0;
	PCD_BUF[1] = (FSDI<<4) | 0x00; /* FSDI | CID */
	ret = PCD_cmd(&PCD_CMD, PCD_BUF);
	printk("RATS ret: %d len: %d  ", ret, PCD_CMD.len);

	if(ret == 0) {
		//PCD_write(RegRxWait, 0x00);
		for(*ATS_len=0; *ATS_len<PCD_CMD.len; (*ATS_len)++) {
			//pATS[*ATS_len] = PCD_BUF[*ATS_len];
			printk("%02X", PCD_BUF[*ATS_len]);
		}
		printk("\n");
	}

	return ret;
}

int PICC_deselect(void)
{
	int ret;
	int i;

	PCD_set_timeout(3);
	PCD_CMD.cmd = PCD_TRANSCEIVE;
	PCD_CMD.len = 2;
	PCD_BUF[0] = 0xCA;
	PCD_BUF[1] = 0x00;
	ret = PCD_cmd(&PCD_CMD, PCD_BUF);
	printk("DESELECT ret: %d len: %d  ", ret, PCD_CMD.len);
	for(i=0; i<PCD_CMD.len; i++) {
		printk("%02X", PCD_BUF[i]);
	}
	printk("\n");

	if((ret == 0) && (PCD_CMD.len > 0)) {
		block_number = !block_number;
	}

	return ret;
}

#if 1
/* 以下实现中I-block均含CID */
/* PCB [CID NAD] INF [CRC_A_LSB CRC_A_MSB] */
int PICC_tcl(unsigned char *psend, unsigned char *precv, int *len)
{
	int ret = 0;
	int cnt = 0;
	int remain_len;
	int send_len;
	int recv_len = 0;
	unsigned char PCB_R;
	unsigned char *psbuf;
	unsigned char *prbuf;

	PCD_set_timeout(7);
	remain_len = *len;
	psbuf = psend;
	prbuf = precv;
	PCD_BUF[0] = 0xA0;
	PCD_CMD.len = 2;
	PCB_R = 0xAA;
	while(ret == 0) {
		if((PCD_BUF[0] & 0xF0) == 0x00) {
		/* 通讯结束 */
			for(cnt=2; cnt<PCD_CMD.len; cnt++) {
				*prbuf++ = PCD_BUF[cnt];
			}
			if(cnt > 2) {
				recv_len += cnt - 2;
			}
			*len = recv_len;
			printk("TCL ret: %d len: %d\n", ret, *len);
			break;
		}
		if((PCD_BUF[0] & 0xF0) == 0xA0) {
		/* 发送后续数据 */
			send_len = FSD - 2;
			if(remain_len <= send_len) {
			/* 最后一块，清chaining位*/
				send_len = remain_len;
				PCD_BUF[0] = 0x0A ^ block_number;
			}else {
			/* 还有后续块，置chaining位*/
				PCD_BUF[0] = 0x1A ^ block_number;
			}
			remain_len -= send_len;
			PCD_CMD.len = send_len + 2;
			PCD_BUF[1] = 0x00; /* CID */
			for(cnt=0; cnt< send_len; cnt++) {
				PCD_BUF[2+cnt] = *psbuf++;
			}
			block_number = !block_number;
			PCD_CMD.cmd = PCD_TRANSCEIVE;
			ret = PCD_cmd(&PCD_CMD, PCD_BUF);
			continue;
		}
		if((PCD_BUF[0] & 0xF0) == 0x10) {
		/* 接收后续数据 */
			for(cnt=2; cnt<PCD_CMD.len; cnt++) {
				*prbuf++ = PCD_BUF[cnt];
			}
			if(cnt > 2) {
				recv_len += cnt - 2;
			}
			if(PCD_BUF[0] & 0x01) {
				PCB_R &= 0xFE;
			}else {
				PCB_R |= 0x01;
			}
			PCD_BUF[0] = PCB_R;
			PCD_BUF[1] = 0x00; /* CID */
			PCD_CMD.len = 2;
			PCD_CMD.cmd = PCD_TRANSCEIVE;
			ret = PCD_cmd(&PCD_CMD, PCD_BUF);
			continue;
		}
		ret = -1;
		break;
	}

	return ret;	
}
#endif

