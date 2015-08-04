/*
 * Filename      : rc500.h
 * Author        : nuc0707#163#com
 * Create time   : 2015-06-26 15:05
 * Modified      : append in the following formate
 * year-month-day description
 */

#ifndef _RC500_H_
#define _RC500_H_
/* RC500 FIFO长度，处理帧长度 */
/* FSDI   0   1   2   3   4   5   6   7   8   9-F
 * FSD   16  24  32  40  48  64  96 128 256   RFU
 */
#define FSDI 5
#define FSD 64
/* RC500寄存器定义 */
#define RegPage               0x00 /* Page Select Register */
#define RegCommand            0x01 /* Command Register */
#define RegFIFOData           0x02 /* FiFo Register */
#define RegPrimaryStatus      0x03 /* Modem State/IRQ/ERR/LoHiAlert Reg */
#define RegFIFOLength         0x04 /* Buffer length Register */
#define RegSecondaryStatus    0x05 /* diverse status flags */
#define RegInterruptEn        0x06 /* IRQ enable Register */
#define RegInterruptRq        0x07 /* IRQ bits Register */
#define RegControl            0x09 /* processor control */
#define RegErrorFlag          0x0A /* error flags showing the error status of the last command executed */
#define RegCollPos            0x0B /* bit position of the first bit collision detected on the RF-interface */
#define RegTimerValue         0x0C /* preload value of the timer */
#define RegCRCResultLSB       0x0D /* LSB of the CRC Coprocessor register */
#define RegCRCResultMSB       0x0E /* MSB of the CRC Coprocessor register */
#define RegBitFraming         0x0F /* Adjustments for bit oriented frames */
#define RegTxControl          0x11 /* controls the logical behaviour of the antenna driver pins TX1 and TX2 */
#define RegCwConductance      0x12 /* selects the conductance of the antenna driver pins TX1 and TX2 */
#define RFU13                 0x13 /* RFU */
#define RegCoderControl       0x14 /* selects coder rate */
#define RegModWidth           0x15 /* selects the width of the modulation pulse */
#define RFU16                 0x16 /* RFU */
#define RFU17                 0x17 /* RFU */
#define RegRxControl1         0x19 /* controls receiver behaviour */
#define RegDecoderControl     0x1A /* controls decoder behaviour */
#define RegBitPhase           0x1B /* selets the bit phase between transmitter and receiver clock */
#define RegRxThreshold        0x1C /* selects thresholds for the bit decoder */
#define RFU1D                 0x1D /* RFU */
#define RegRxControl2         0x1E /* controls decoder behaviour and defines the input source for the receiver */
#define RegClockQControl      0x1F /* controls clock generation for the 90?phase shifted Q-channel clock */
#define RegRxWait             0x21 /* selects the time interval after transmission, before receiver starts */
#define RegChannelRedundancy  0x22 /* selects the kind and mode of checking the data integrity on the RF-channel */
#define RegCRCPresetLSB       0x23 /* LSB of the pre-set value for the CRC register */
#define RegCRCPresetMSB       0x24 /* MSB of the pre-set value for the CRC register */
#define RFU25                 0x25 /* RFU */
#define RegMfOutSelect        0x26 /* selects internal signal applied to pin MfOut */
#define RFU27                 0x27 /* RFU */
#define RegFIFOLevel          0x29 /* Defines level for FIFO over- and underflow warning */
#define RegTimerClock         0x2A /* selects the divider for the timer clock */
#define RegTimerControl       0x2B /* selects start and stop conditions for the timer */
#define RegTimerReload        0x2C /* defines the pre-set value for the timer */
#define RegIRqPinConfig       0x2D /* configures the output stage of pin IRq */
#define RFU2E                 0x2E /* RFU */
#define RFU2F                 0x2F /* RFU */
#define RFU31                 0x31 /* RFU */
#define RFU32                 0x32 /* RFU */
#define RFU33                 0x33 /* RFU */
#define RFU34                 0x34 /* RFU */
#define RFU35                 0x35 /* RFU */
#define RFU36                 0x36 /* RFU */
#define RFU37                 0x37 /* RFU */
#define RFU39                 0x39 /* RFU */
#define RegTestAnaSelect      0x3A /* selects analog test mode */
#define RFU3B                 0x3B /* RFU */
#define RFU3C                 0x3C /* RFU */
#define RegTestDigiSelect     0x3D /* selects digital test mode */
#define RFU3E                 0x3E /* RFU */
#define RegTestDigiAccess     0x3F

#define PCD_IRQ_TIMER	0x20
#define PCD_IRQ_TX	0x10
#define PCD_IRQ_RX	0x08
#define PCD_IRQ_IDLE	0x04
#define PCD_IRQ_HIALERT	0x02
#define PCD_IRQ_LOALERT	0x01

/* RC500命令定义 */
#define PCD_STARTUP	0x3F /* Runs the Reset- and Initialisation Phase. */
#define PCD_IDLE	0x00 /* No action; cancels current command execution. */
#define PCD_TRANSMIT	0x1A /* Transmits data from the FIFO buffer to the card. */
#define PCD_RECEIVE	0x16 /* Activates receiver circuitry. */
#define PCD_TRANSCEIVE	0x1E /* Transmits data from FIFO buffer to the card and activates automatically the receiver after transmission. */
#define PCD_WRITEE2	0x01 /* Gets data from FIFO buffer and writes it to the internal E2PROM. */
#define PCD_READE2	0x03 /* Reads data from the internal E2PROM Start Address LSB and puts it into the FIFO buffer. */
#define PCD_LOADKEYE2	0x0B /* Copies a key from the E2PROM into Start Address LSB the key buffer. */
#define PCD_LOADKEY	0x19 /* Reads a key from the FIFO buffer and puts it into the key buffer. */
#define PCD_AUTHENT1	0x0C /* Performs the first part of the Crypto1 card authentication. */
#define PCD_AUTHENT2	0x14 /* Performs the second part of the card authentication using the Crypto1 algorithm. */
#define PCD_LOADCONFIG	0x07 /* Reads data from E2PROM and initialises the MF RC500 registers. */
#define PCD_CALCCRC	0x12 /* Activates the CRC-Coprocessor. */

struct MF_CMD_INFO {
	unsigned char cmd; /* RC500命令 */
	unsigned char len; /* 数据长度 */
	unsigned char recv_bit; /* 接收位数 */
	unsigned char coll_pos; /* 冲突位置 */
};

typedef struct {
	unsigned char MfCommand;   
	unsigned char MfCtlFlag;  
	unsigned char MfSector;    
	unsigned int MfLength;
	unsigned char MfData[256];  
	int MfStatus;      
}TranSciveBuffer;

extern TranSciveBuffer command;
extern unsigned char PCD_INFO[16];
extern unsigned char PCD_BUF[FSD];
extern struct MF_CMD_INFO PCD_CMD;

extern unsigned char PCD_read(unsigned char addr);
extern void PCD_write(unsigned char addr, unsigned char data);
extern void PCD_bitset(unsigned char addr, unsigned char bits);
extern void PCD_bitclr(unsigned char addr, unsigned char bits);
extern void PCD_set_timeout(int tmo);
extern void PCD_antenna(unsigned char ctl);
extern int PCD_init(void);
extern int PCD_get_info(void);
extern int PCD_cmd(struct MF_CMD_INFO *cmd_info, unsigned char *buf);
extern int sendtoRC500(TranSciveBuffer *send, TranSciveBuffer *receive);

#endif
