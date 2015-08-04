/*
 * Filename      : rc500_x8007.h
 * Author        : nuc0707#163#com
 * Create time   : 2015-06-26 10:54
 * Modified      : append in the following formate
 * year-month-day description
 */

#ifndef _RC500_X8007_H_
#define _RC500_X8007_H_

/* 
 * SCR = smart card reader 
 * /dev/SCR0 <-> MFRC500
 * /dev/SCR1 <-> TDA8007B(DS8007)
 */

/* 
 * 程序主板本号
 * 用于表示程序主体结构和框架
 */
#define DRV_MAJOR_VERSION_NO 2
/* 
 * 程序次版本号
 * 用于表示程序功能的添加或者单元性功能的改变
 */
#define DRV_MINOR_VERSION_NO 1
/* 
 * 程序修正版本号
 * 用于表示程序修正了某些BUG或做了一定优化
 */
#define DRV_PATCH_VERSION_NO 0




#define SC_READER "SCR"

#define SC_READER_MIN 0
#define SC_READER_MAX 2

#endif /* _RC500_X8007_H_ */

