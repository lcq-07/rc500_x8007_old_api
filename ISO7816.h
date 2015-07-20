/*
 * Filename      : ISO7816.h
 * Author        : nuc0707#163#com
 * Create time   : 2015-07-16 10:12
 * Modified      : append in the following formate
 * year-month-day description
 */

#ifndef _ISO_7816_
#define _ISO_7816_

struct APDU_MSG {
	unsigned char CLA; /* 指令类 */
	unsigned char INS; /* 指令 */
	unsigned char P1;  /* 参数1 */
	unsigned char P2;  /* 参数2 */
	unsigned char Lc;  /* DAT长度 */
	unsigned char DATA[260]; /* 数据（数据+Le|数据+SW） */
};

#endif /* _ISO_7816_ */
