/*
 * Filename      : ISO14443_3A.c
 * Author        : nuc0707#163#com
 * Create time   : 2015-06-26 11:01
 * Modified      : append in the following formate
 * year-month-day description
 */

#include <linux/kernel.h>
#include "debug_print.h"
#include "rc500.h"
#include "ISO14443_3A.h"

unsigned char PICC_UID[16];

/* ISO14443A请求操作 */
int PICC_request(unsigned char req_code, unsigned char *ATQ)
{
	int ret;
	/* initialize */
	PCD_set_timeout(106);
	PCD_write(RegChannelRedundancy, 0x03); /* RxCRC and TxCRC disable, parity enable */
	PCD_bitclr(RegControl, 0x08); /* disable crypto 1 unit */
	PCD_write(RegBitFraming, 0x07); /* set TxLastBits to 7 */
	PCD_bitset(RegTxControl, 0x03); /* Tx2RF-En, Tx1RF-En enable */

	PCD_CMD.cmd = PCD_TRANSCEIVE;
	PCD_CMD.len = 1;
	PCD_BUF[0] = req_code;
	ret = PCD_cmd(&PCD_CMD, PCD_BUF);
	DEBUG_PRINT("REQUEST[%d] ATQ[%d]: %02X%02X\n", ret, PCD_CMD.len, PCD_BUF[1], PCD_BUF[0]);
	if((ret == 0)&&(PCD_CMD.len == 2)) {
		/* 2 bytes expected */
		ATQ[0] = PCD_BUF[0];
		ATQ[1] = PCD_BUF[1];
	}else {
		ret = -1;
	}

	return ret;
}

/* ISO14443A防冲突，仅适用RF场中单张单长度UID */
int PICC_anticoll(unsigned char *picc_uid)
{
	int ret;
	/* Initialisation */
	PCD_set_timeout(106);

	PCD_write(RegDecoderControl, 0x28); /* ZeroAfterColl aktivieren */
	PCD_bitclr(RegControl, 0x08); /* disable crypto 1 unit */
	PCD_write(RegChannelRedundancy, 0x03); /* RxCRC and TxCRC disable, parity enable */

	PCD_CMD.cmd = PCD_TRANSCEIVE;
	PCD_CMD.len = 2;
	PCD_BUF[0] = ISO14443_SEL1;
	PCD_BUF[1] = 0x20;
	ret = PCD_cmd(&PCD_CMD, PCD_BUF);
	DEBUG_PRINT("ANTICOLL[%d] UID[%d]: %02X%02X%02X%02X%02X\n", ret, PCD_CMD.len, 
		PCD_BUF[0], PCD_BUF[1], PCD_BUF[2], PCD_BUF[3], PCD_BUF[4]);
	if((ret == 0)&&(PCD_CMD.len == 5)) {
		picc_uid[0] = PCD_BUF[0];
		picc_uid[1] = PCD_BUF[1];
		picc_uid[2] = PCD_BUF[2];
		picc_uid[3] = PCD_BUF[3];
		picc_uid[4] = PCD_BUF[4]; /* 注意此处该字节为CRC_A */
	}else {
		ret = -1;
	}

	PCD_bitclr(RegDecoderControl, 0x20); /* ZeroAfterColl disable */
	return ret;
}

/* ISO14443A选择，仅适用RF场中单张单长度UID */
int PICC_select(unsigned char *picc_uid, unsigned char *sak)
{
	int ret;

	PCD_set_timeout(4);
	PCD_write(RegChannelRedundancy, 0x0F); /* RxCRC,TxCRC, Parity enable */
	PCD_bitclr(RegControl, 0x08); /* disable crypto 1 unit */

	PCD_CMD.cmd = PCD_TRANSCEIVE;
	PCD_CMD.len = 7;
	PCD_BUF[0] = ISO14443_SEL1;
	PCD_BUF[1] = 0x70; PCD_BUF[6] = 0; /* 注意此处该字节为CRC_A */
	PCD_BUF[2] = picc_uid[0]; PCD_BUF[6] ^= picc_uid[0];
	PCD_BUF[3] = picc_uid[1]; PCD_BUF[6] ^= picc_uid[1];
	PCD_BUF[4] = picc_uid[2]; PCD_BUF[6] ^= picc_uid[2];
	PCD_BUF[5] = picc_uid[3]; PCD_BUF[6] ^= picc_uid[3];
	ret = PCD_cmd(&PCD_CMD, PCD_BUF);
	DEBUG_PRINT("SELECT[%d] SAK[%d]: %02X\n", ret, PCD_CMD.len, PCD_BUF[0]);
	if((ret == 0)&&(PCD_CMD.len == 1)) {
		sak[0] = PCD_BUF[0];
		PICC_UID[0] = picc_uid[0];
		PICC_UID[1] = picc_uid[1];
		PICC_UID[2] = picc_uid[2];
		PICC_UID[3] = picc_uid[3];
	}else{
		ret = -1;
	}

	return ret;
}

/* ISO14443 停卡 */
int PICC_halt(void)
{
	int ret;
	
	PCD_CMD.cmd = PCD_TRANSCEIVE;
	PCD_CMD.len = 2;
	PCD_BUF[0] = ISO14443_HLTA;
	PCD_BUF[1] = 0x00;
	ret = PCD_cmd(&PCD_CMD, PCD_BUF);
	DEBUG_PRINT("HALT[%d]\n", ret);
	PCD_write(RegCommand, PCD_IDLE);

	return ret;
}

/* NXP mifare卡密钥验证操作 */
int PICC_MFauth(unsigned char key_type, unsigned char block, unsigned char *key)
{
	int ret;
	unsigned char ln;
	unsigned char hn;
	unsigned char sta;

	
	PCD_CMD.cmd = PCD_LOADKEY;
	PCD_CMD.len = 12;
	/* 密钥转换为RC500内部格式 */
	for(ret=0; ret<6; ret++) {
		ln = key[ret] & 0x0F;
		hn = key[ret] >> 4;
		PCD_BUF[(ret<<1)+1] = (~ln << 4) | ln;
		PCD_BUF[(ret<<1)]   = (~hn << 4) | hn;
	}
	ret = PCD_cmd(&PCD_CMD, PCD_BUF);
	sta = PCD_read(RegErrorFlag);
	DEBUG_PRINT("LOADKEY[%d] len: %d sta: %02X\n", ret, PCD_CMD.len, sta);
	
	if((ret == 0) && (sta == 0)) {
		PCD_set_timeout(3);
		PCD_write(RegChannelRedundancy, 0x07); /* TxCRC, Parity enable */
		PCD_CMD.cmd = PCD_AUTHENT1;
		PCD_CMD.len = 6;
		PCD_BUF[0] = key_type;
		PCD_BUF[1] = block;
		PCD_BUF[2] = PICC_UID[0];
		PCD_BUF[3] = PICC_UID[1];
		PCD_BUF[4] = PICC_UID[2];
		PCD_BUF[5] = PICC_UID[3];
		ret = PCD_cmd(&PCD_CMD, PCD_BUF);
		sta = PCD_read(RegSecondaryStatus);
		DEBUG_PRINT("AUTHENT1[%d] len: %d sta: %02X\n", ret, PCD_CMD.len, sta);
		if((ret == 0) && ((sta&0x07) == 0)) {
			PCD_CMD.cmd = PCD_AUTHENT2;
			PCD_CMD.len = 0;
			ret = PCD_cmd(&PCD_CMD, PCD_BUF);
			sta = PCD_read(RegControl);
			DEBUG_PRINT("AUTHENT2 [%d] len: %d sta: %02X\n", ret, PCD_CMD.len, sta);
			if((ret == 0) && ((sta&0x08) == 0x08)) {
				/* Crypto1 activated */
			}else {
				ret = -1;
			}
		}else {
			ret = -1;
		}
	}else {
		ret = -1;
	}

	return ret;
}

/* NXP mifare卡读操作 */
int PICC_MFread(unsigned block, unsigned char *buf)
{
	int ret;
	PCD_set_timeout(3);
	PCD_write(RegChannelRedundancy, 0x0F); /* RxCRC, TxCRC, Parity enable */
	
	PCD_CMD.cmd = PCD_TRANSCEIVE;
	PCD_CMD.len = 2;
	PCD_BUF[0] = PICC_MFREAD;
	PCD_BUF[1] = block;
	ret = PCD_cmd(&PCD_CMD, PCD_BUF);
	DEBUG_PRINT("MFREAD[%d] len: %d\n", ret, PCD_CMD.len);
	if((ret == 0) && (PCD_CMD.len == 16)) {
		buf[0] = PCD_BUF[0]; buf[8] = PCD_BUF[8];
		buf[1] = PCD_BUF[1]; buf[9] = PCD_BUF[9];
		buf[2] = PCD_BUF[2]; buf[10] = PCD_BUF[10];
		buf[3] = PCD_BUF[3]; buf[11] = PCD_BUF[11];
		buf[4] = PCD_BUF[4]; buf[12] = PCD_BUF[12];
		buf[5] = PCD_BUF[5]; buf[13] = PCD_BUF[13];
		buf[6] = PCD_BUF[6]; buf[14] = PCD_BUF[14];
		buf[7] = PCD_BUF[7]; buf[15] = PCD_BUF[15];
	}else {
		ret = -1;
	}

	PCD_set_timeout(1);
	return ret;
}

/* NXP mifare写卡操作 */
int PICC_MFwrite(unsigned block, unsigned char *buf)
{
	int ret;
	
	PCD_write(RegChannelRedundancy, 0x07); /* TxCRC, Parity enable */
	PCD_set_timeout(4);
	PCD_CMD.cmd = PCD_TRANSCEIVE;
	PCD_CMD.len = 2;
	PCD_BUF[0] = PICC_MFWRITE;
	PCD_BUF[1] = block;
	ret = PCD_cmd(&PCD_CMD, PCD_BUF);
	DEBUG_PRINT("MFWRITE1[%d] recv_bit: %d[%02X]\n", ret, PCD_CMD.recv_bit, PCD_BUF[0]);
	if((ret == 0) && (PCD_CMD.recv_bit == 4) && ((PCD_BUF[0]&0x0f) == 0x0a)) {
		PCD_set_timeout(4);
		PCD_CMD.cmd = PCD_TRANSCEIVE;
		PCD_CMD.len = 16;
		PCD_BUF[0] = buf[0]; PCD_BUF[8]  = buf[8];
		PCD_BUF[1] = buf[1]; PCD_BUF[9]  = buf[9];
		PCD_BUF[2] = buf[2]; PCD_BUF[10] = buf[10];
		PCD_BUF[3] = buf[3]; PCD_BUF[11] = buf[11];
		PCD_BUF[4] = buf[4]; PCD_BUF[12] = buf[12];
                PCD_BUF[5] = buf[5]; PCD_BUF[13] = buf[13];
                PCD_BUF[6] = buf[6]; PCD_BUF[14] = buf[14];
                PCD_BUF[7] = buf[7]; PCD_BUF[15] = buf[15];
		ret = PCD_cmd(&PCD_CMD, PCD_BUF);
		DEBUG_PRINT("MFWRITE2[%d] recv_bit: %d[%02X]\n", ret, PCD_CMD.recv_bit, PCD_BUF[0]);
		if((ret == 0) && (PCD_CMD.recv_bit == 4) && ((PCD_BUF[0]&0x0f) == 0x0a)) {
		}else {
			ret = -1;
		}
	}else {
		ret = -1;
	}

	PCD_set_timeout(1);
	return ret;
}

