//____________________________________________________________________
// Copyright Info     :  All Rights Reserved - (c) Arasan Chip Systems Inc.,
//
//Name                  :  sduart.h
//
//Creation Date      :  12-April-2007
//
//Description          : This file contains the header file code of UART serial driver for Arasan SDIO board Changed History 
//
//Changed History  :
//<Date>                   <Author>        <Version>        < Description >        
//12-April-2007              AG                   0.1                  base code
//____________________________________________________________________


#ifndef __SDUART_H__
#define __SDUART_H__

#define BOARD_CLK_FREQ                147456
#define DEFAULT_BLOCK_SIZE            64
#define DEFAULT_TRIGGER_LEVEL         64

#define NR_PORTS	(sizeof(state_table)/sizeof(struct serial_state))

// UART Register - Function 1 Registers
#define SDUART_RECEIVE_REG            0x00 // Accessed when DLAB = 0; R - only
#define SDUART_TRANS_HOLD_REG         0x00 // Accessed when DLAB = 0; W - only
#define SDUART_INTR_ENABLE_REG        0x01    
#define SDUART_INTR_IDENT_REG         0x02
#define SDUART_FIFO_CONTROL_REG       0x02
#define SDUART_LINE_CONTROL_REG       0x03
#define SDUART_LINE_STATUS_REG        0x05
#define SDUART_MODEM_CONTROL_REG      0x04
#define SDUART_MODEM_STATUS_REG       0x06
#define SDUART_DIVISOR_LATCH_LS_REG   0x00
#define SDUART_DIVISOR_LATCH_MS_REG   0x01
#define SDUART_EXT_FIFO_ENABLE_REG    0x08
#define SDUART_TRANS_COUNT_REG        0x09
#define SDUART_RECEIVE_COUNT_REG      0x0A

/*
 * Interrupt Enable Register - OffSet 0x01
 *
 * This Register is accessed when DLAB = 0
 */
#define IER_ENABLE_MODEM_STATUS      (1 << 3)
#define IER_ENABLE_LINE_STATUS       (1 << 2)
#define IER_ENABLE_TRANS_HOLD        (1 << 1)
#define IER_ENABLE_RECEIVE_DATA      (1 << 0)
#define IER_ENABLE_ALL_INTERRUPT      0x0F
#define IER_DISABLE_ALL_INTERRUPT     0x00


/*
 * Interrupt Identification Register - OffSet 0x02
 */
#define IIR_INTR_PENDING              (1 << 0)
#define IIR_TIMEOUT_INTR_PENDING      (1 << 3)
#define IIR_INTR_PENDING_STATUS       0x06
#define IIR_IS_FIFO_ENABLED           0xC0
#define IIR_MODEM_STATUS              0x00
#define IIR_TRANS_HOLD_REG_EMPTY      0x02
#define IIR_CHAR_TIMEOUT              0x0C
#define IIR_RECEIVED_DATA_AVAIL       0x04
#define IIR_RECEIVE_LINE_STATUS       0x06
#define IIR_TOTAL_INTERRUPT_STATUS    0x0F


/*
 * FIFO Control Register - OffSet 0x03      ;   This is a write only Register at the same location as the IIR This 
    register is used to enableFIFOs/clearFIFOs/Set RCVR FIFO Trigger level
 */
#define FCR_IER_MODEM_STATUS_INTR     (1 << 0)
#define FCR_IER_CLEAR_RCVR_FIFO       (1 << 1)
#define FCR_IER_CLEAR_XMIT_FIFO       (1 << 2)
#define FCR_DMA_MODE                  (1 << 3)
#define FCR_TRIGGER_LEVEL_01          0x00 
#define FCR_TRIGGER_LEVEL_04          0x40
#define FCR_TRIGGER_LEVEL_08          0x80
#define FCR_TRIGGER_LEVEL_14          0xC0

// Line Control Register - OffSet 0x3
#define LCR_SET_DLAB_BIT              (1 << 7)
#define LCR_SET_BREAK_CONTROL         (1 << 6)
#define LCR_SET_STICKY_PARITY         (1 << 5)
#define LCR_SET_EVEN_PARITY           (1 << 4)
#define LCR_SET_PARITY_ENABLE         (1 << 3)
#define LCR_SET_1_5_STOP_BIT          (1 << 2)
#define LCR_SET_2_STOP_BIT            (1 << 2)
#define LCR_SET_1_STOP_BIT            (0 << 2)
#define LCR_STOP_BIT_MASK             (1 << 2)
#define LCR_PARITY_MASK               (0x38)

#define LCR_RESET_DLAB_BIT            ~(LCR_SET_DLAB_BIT)
#define LCR_RESET_BREAK_CONTROL       ~(LCR_SET_BREAK_CONTROL)
#define LCR_RESET_STICKY_PARITY       ~(LCR_SET_STICKY_PARITY)
#define LCR_SET_ODD_PARITY            ~(LCR_SET_EVEN_PARITY)
#define LCR_RESET_PARITY_ENABLE       ~(LCR_SET_PARITY_ENABLE)
#define LCR_SET_ONE_AND_HALF_STOP_BIT ~(LCR_SET_1_STOP_BIT)       //hanged from LCR_SET_ONE_STOP_BIT

#define LCR_CHAR_LEN_5BITS            (0x00)
#define LCR_CHAR_LEN_6BITS            (0x01)
#define LCR_CHAR_LEN_7BITS            (0x02)
#define LCR_CHAR_LEN_8BITS            (0x03)
#define LCR_CHAR_LEN_MASK             (0x03)

// MODEM Control Register - Offset 0x4
#define MCR_SET_DTR_CONTROL           (1 << 0)
#define MCR_SET_RTS_CONTROL           (1 << 1)    
#define MCR_SET_DATA_CARRIER_DETECT   (1 << 2)
#define MCR_SET_RING_INDICATOR        (1 << 3)
#define MCR_SET_LOOP_BACK             (1 << 4)

#define MCR_RESET_DTR_CONTROL         ~(MCR_SET_DTR_CONTROL)
#define MCR_RESET_RTS_CONTROL         ~(MCR_SET_RTS_CONTROL)
#define MCR_RESET_DATA_CARRIER_DETECT ~(MCR_SET_DATA_CARRIER_DETECT)
#define MCR_RESET_RING_INDICATOR      ~(MCR_SET_RING_INDICATOR)
#define MCR_RESET_LOOP_BACK           ~(MCR_SET_LOOP_BACK)

// Line Status Register - OffSet 0x5
#define LSR_GET_DATA_READY            (1 << 0)
#define LSR_GET_OVERRUN_ERROR         (1 << 1)    
#define LSR_GET_PARITY_ERROR          (1 << 2)
#define LSR_GET_FRAMING_ERROR         (1 << 3)
#define LSR_GET_BREAK_INTERRUPT       (1 << 4)
#define LSR_GET_THR_EMPTY             (1 << 5)
#define LSR_GET_TRANSMIT_EMPTY        (1 << 6)
#define LSR_GET_ERROR_INDICATOR       (1 << 7)


// MODEM Status Register - Offset 0x6
#define MSR_GET_DCTS                  (1 << 0)
#define MSR_GET_DDSR                  (1 << 1)
#define MSR_GET_TERI                  (1 << 2)
#define MSR_GET_DDCD                  (1 << 3)
#define MSR_GET_CTS                   (1 << 4)
#define MSR_GET_DSR                   (1 << 5)
#define MSR_GET_RI                    (1 << 6)
#define MSR_GET_DCD                   (1 << 7)


// * Extended FIFO Enable Register - OffSet 0x8
#define EXTENDED_FIFO_ENABLE          (1 << 0)
#define EXTENDED_FIFO_DISABLE        ~(EXTENDED_FIFO_ENABLE)

typedef unsigned int     SD_UINT32;
typedef unsigned int     SD_UINT;
typedef unsigned long    SD_ULONG;
typedef unsigned char    SD_UCHAR;
typedef int              SD_INT;
typedef int              SD_INT32;
typedef unsigned short   SD_UINT16; 
typedef void             SD_VOID;
typedef struct semaphore SD_SEMAPHORE;

#define SD_STATIC        static
#define SD_INLINE        inline
#define SD_SUCCESS       0
#define SD_ERROR_NODEV   -1
#define SD_ERROR_INVALID -1
#define SD_LOCK          spinlock_t

struct sd_interface 
{
unsigned int interfaceCode;
unsigned int interfaceID;
unsigned int BlockSize;
unsigned int MaxBlockSize;
unsigned int Enabled;
unsigned int WriteProtected;
//unsigned void *CISPointer;
//unsigned void *CSAPointer;
//unsigned int tupleData[SD_CISTPL_FUNCEX_LENGTH];
unsigned int SizeInSectors;
unsigned int pDevice;
spinlock_t command;
};

typedef struct sd_interface SD_INTERFACE;
typedef struct sd_interface * PSD_INTERFACE;

//struct _SD_BLOCKDEV;
struct sd_srb;

typedef struct sd_srb SD_SRB;
typedef struct sd_srb * PSD_SRB;
typedef struct _SD_BLOCKDEV
{
PSD_INTERFACE        pInterface;
SD_UINT32            SizeInSectors;
SD_UINT32            blockLength;
SD_UINT32            blkDevNo;
SD_UINT16            UseCount;
SD_UINT16            StatusChange;
SD_UINT16            DevConnected;
SD_LOCK              devLock;
struct block_device *pBdev;
PSD_SRB              pDataSrb;
PSD_SRB              pCmdSrb;
spinlock_t           queue_lock;
}SD_BLOCKDEV;

struct sd_srb 
{
int          srbStatus;
SD_BLOCKDEV *srbCompArg;
};

#define sd_Verbose(format, args...) printk(KERN_ALERT   "" format, ##args)
#define sd_Error(format, args...)   printk(KERN_ERR     "" format, ##args)
#define sd_Warn(format, args...)    printk(KERN_WARNING "" format, ##args)
#define sd_Debug(format, args...)   printk(KERN_DEBUG   "" format, ##args)

#define VERIFY_READ  0
#define VERIFY_WRITE 1
//#define ASYNC_CALLOUT_ACTIVE 0

#endif /* !__SDUART_H__ */
