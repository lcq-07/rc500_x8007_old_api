/*
 * Filename      : ISO14443_4A.h
 * Author        : nuc0707#163#com
 * Create time   : 2015-07-22 11:56
 * Modified      : append in the following formate
 * year-month-day description
 */
#ifndef _ISO14443_4A_H_
#define _ISO14443_4A_H_

int PICC_rats(unsigned char *pATS, int *ATS_len);
int PICC_deselect(void);
int PICC_tcl(unsigned char *psend, unsigned char *precv, int *len);

#endif /* _ISO14443_4A_H_ */
