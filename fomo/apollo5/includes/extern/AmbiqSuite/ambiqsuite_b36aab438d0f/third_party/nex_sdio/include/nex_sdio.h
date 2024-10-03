//____________________________________________________________________
// Copyright Info : All Rights Reserved - (c) Arasan Chip Systems Inc.,
//
//Name            : nex_sdio.h
//
//Creation Date   : 21-Feb-2007
//
//Description     : Header file containing defines for SDIO
//
//Changed History :
//<Date>           <Author>        <Version>          < Description >       
//____________________________________________________________________

#ifndef __SD_SDIO_H
#define __SD_SDIO_H

#include "nex_porting.h"

// Tuple Hash Defines
#define CIS_TUPLE_LINK          0x01
#define CIS_MANFID_TUPLE        0x20
#define CIS_FUNCID              0x21
#define CIS_FUNCE               0x22
#define CIS_END_OF_TUPLE        0xFF
#define CIS_NULL_TUPLE          0x00

// HighSpeed Hash Defines
#define HIGHSPEED_SUPPORT 		0x1
#define FULLSPEED_ENABLE  		(0x0 << 1)
#define HIGHSPEED_ENABLE  		0x2
#define SDIO_SSDR12_ENABLE 		(0x0 << 1)
#define SDIO_SSDR24_ENABLE 		(0x1 << 1)
#define SDIO_SSDR50_ENABLE 		(0x2 << 1)
#define SDIO_SSDR104_ENABLE 	(0x3 << 1)
#define SDIO_SDDR50_ENABLE 		(0x4 << 1)

#define HIGHSPEED_CHECK   	    0x3
#define SDIO_SSDR12_CHECK 	    0x0
#define SDIO_SSDR24_CHECK 	    0x2
#define SDIO_SSDR50_CHECK 	    0x4
#define SDIO_SSDR104_CHECK 	    0x6
#define SDIO_SDDR50_CHECK 	    0x8


#define FULLSPEED_SUPPORT 		0x40
#define READWAIT_SUPPORT		0x04
#define SUSPEND_RESUME_SUPPORT 	0x08

//UHS defines
#define SDIO_SSDR50		        0x01
#define SDIO_SSDR104	        0x02
#define SDIO_SDDR50		        0x04

//Driver Strength defines
#define SDIO_SDTA		        0x01
#define SDIO_SDTC	 	        0x02
#define SDIO_SDTD		        0x04


// This the max byte count for cmd53 i.e. 2^9
#define MAX_BYTE_COUNT          512

// SDIO CCCR Registers
#define SDIO_CCCR_REV     		0x00
#define SDIO_SD_REV       		0x01
#define SDIO_IO_ENABLE    		0x02
#define SDIO_IO_RDY       		0x03
#define SDIO_INT_ENABLE   		0x04
#define SDIO_INT_RDY      		0x05
#define SDIO_IO_ABORT     		0x06
#define SDIO_BUS_CNTRL    		0x07
#define SDIO_CARD_CAPAB   		0x08
#define SDIO_CIS_PTR1     		0x09
#define SDIO_CIS_PTR2     		0x0A
#define SDIO_CIS_PTR3     		0x0B
#define SDIO_BUS_SUSPEND  		0x0C
#define SDIO_FN_SELECT    		0x0D
#define SDIO_EXEC_FLAGS   		0x0E
#define SDIO_RDY_FLAGS    		0x0F
#define SDIO_FN_BLK_SIZE1	 	0x10
#define SDIO_FN_BLK_SIZE2 		0x11
#define SDIO_PWR_CNTRL    		0x12
#define SDIO_BUS_SPEED_SELECT   0x13
#define SDIO_UHS_SUPPORT		0x14
#define SDIO_DRIVER_STRENGTH	0x15

struct sd_card;
struct sd_host;
struct sd_card_function;
struct nex_host;

typedef struct __sdio_parameters
{
    UINT32 incradrs;
} SDIO_PARAM;

extern VOID  sdio_discover_functions(struct sd_host *host, struct sd_card *card);
extern INT32 sdio_write_multiple_bytes(struct sd_host* host, UCHAR* buffer, UINT32 length, UINT32 address, SDIO_PARAM *sdio_param);
extern INT32 sdio_read_multiple_bytes(struct sd_host* host, UCHAR* buffer, UINT32 length, UINT32 address, SDIO_PARAM *sdio_param);
extern INT32 sdio_write_multiple_bytes_dma(struct sd_host *host, SD_NEX_ADMA_TABLE *adma_table, UINT32 length, UINT32 address, SDIO_PARAM *sdio_param);
extern INT32 sdio_read_multiple_bytes_dma(struct sd_host *host, SD_NEX_ADMA_TABLE *adma_table, UINT32 length, UINT32 address, SDIO_PARAM *sdio_param);

UCHAR  sdio_readb(const struct sd_card_function *func, UINT32 addr);
VOID   sdio_writeb(const struct sd_card_function *func, UINT32 addr, UCHAR data);

UCHAR  sdio_fn_readb(UINT32 function, const struct sd_card_function *func, UINT32 addr);
VOID   sdio_fn_writeb(UINT32 function, const struct sd_card_function *func, UINT32 addr, UCHAR data);

VOID   sd_sdio_set_bus_width(struct sd_host *host, struct sd_card *card,INT32 bus_width);
VOID   sd_sdio_set_blocklen(struct sd_host *host, struct sd_card* card, UINT32 functionNumber, UINT32 blocklen);
INT32  sdio_set_highspeed(struct sd_host *host);
INT32  sdio_SetUHSMode(struct sd_host *host, INT32 drive_strength, INT32 uhs_mode);
INT32  sdio_suspend(struct nex_host *host,struct sd_card *card,UINT32 functionNumber);
UINT32 sdio_suspendFunction(struct nex_host *host,struct sd_card *card);
INT32  sdio_readAfterWriteb(const struct sd_card *card, UINT32 func, UINT32 addr, UINT32 data, UINT32 flag);
UINT32 sdio_resume(struct nex_host *host,struct sd_card *card,UINT32 functionNumber);
INT32  sdio_set_fullspeed(struct sd_host *host);

#endif
