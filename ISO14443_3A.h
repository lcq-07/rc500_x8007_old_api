/*
 * Filename      : ISO14443_3A.h
 * Author        : nuc0707#163#com
 * Create time   : 2015-07-21 09:37
 * Modified      : append in the following formate
 * year-month-day description
 */

/* ISO14443A 命令*/
#define ISO14443_REQA	0x26 /* request idle */
#define ISO14443_WUPA	0x52 /* request all */
#define ISO14443_SEL1	0x93 /* anticollision level 1 */
#define ISO14443_SEL2	0x95 /* anticollision level 2 */
#define ISO14443_SEL3	0x97 /* anticollision level 3 */
#define ISO14443_HLTA	0x50 /* halt */

/* PICC Mifare卡命令 */
/* Each tag command is written to the reader IC and transfered via RF */
#define PICC_MFAUTHENT1A	0x60 /* authentication step 1 */
#define PICC_MFAUTHENT1B	0x61 /* authentication step 2 */
#define PICC_MFREAD		0x30 /* read block */
#define PICC_MFWRITE		0xA0 /* write block */
#define PICC_MFDECREMENT	0xC0 /* decrement value */
#define PICC_MFINCREMENT	0xC1 /* increment value */
#define PICC_MFRESTORE		0xC2 /* restore command code */
#define PICC_MFTRANSFER		0xB0 /* transfer command code */


extern unsigned char PICC_UID[16];

extern int PICC_request(unsigned char req_code, unsigned char *ATQ);
extern int PICC_anticoll(unsigned char *PICC_UID);
extern int PICC_select(unsigned char *PICC_UID, unsigned char *SAK);
extern int PICC_halt(void);
extern int PICC_MFauth(unsigned char key_type, unsigned char block, unsigned char *key);
extern int PICC_MFread(unsigned block, unsigned char *buf);
extern int PICC_MFwrite(unsigned block, unsigned char *buf);
