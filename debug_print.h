/*
 * Filename      : debug_print.h
 * Author        : nuc0707#163#com
 * Create time   : 2015-08-06 09:05
 * Modified      : append in the following formate
 * year-month-day description
 */

#ifndef _DEBUG_PRINT_H_
#define _DEBUG_PRINT_H_

//#define _DEBUG_PRINT_EN_
#ifdef _DEBUG_PRINT_EN_
//#define DEBUG_PRINT(fmt, ...) printk(KERN_DEBUG "%s[%d]"fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#define DEBUG_PRINT(...) printk(KERN_DEBUG __VA_ARGS__);
#else
#define DEBUG_PRINT(...)
#endif

#endif /* _DEBUG_PRINT_H_ */
