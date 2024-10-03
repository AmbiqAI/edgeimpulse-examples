//____________________________________________________________________
// Copyright Info : All Rights Reserved - (c) Arasan Chip Systems Inc.,
//
// Name           :  nex_thread.h
//
// Creation Date  :  12-March-2007
//
// Description    :  This file contains the code for linked list implementation and structure for making
//                   GetData And SendData nonblocking and the request queue.
//
// Changed History:
//<Date>             <Author>        <Version>        <Description>
// 14-Feb-2007        VS              0.1              base code
// 23-Jul-2020        Ambiq           0.9              only keep the interfacce functions
//____________________________________________________________________

#ifndef __NEX_THREAD_H
#define __NEX_THREAD_H

#include "nex_api.h"
#include "nex_debug.h"
#include "nex_porting.h"
#include "nex_sdio.h"

typedef enum _sd_function_type
{
    SD_GETDATA               = 0,
    SD_SENDDATA              = 1,
    SD_SENDCOMMAND           = 2,
    SD_GETDATA_ADMA          = 3,
    SD_SENDDATA_ADMA         = 4,
    SD_NOCOMMAND             = 5,
    SD_SETIBD                = 6,
    SD_GETIBD                = 7,
    eSD_MANAGEPARTITION      = 8,
    eSD_MANAGEPARTITION_ADMA = 9,
    eSD_QUERYPARTITION       = 10,
    eSD_QUERYPARTITION_ADMA  = 11
} SD_FUNCTION_TYPE;

struct function_info
{
    VOID *            buffer;
    UINT32            length;
    UINT32            address;
    UINT32            sdhcNum;
    UINT32            slotNum;
    SDIO_PARAM        sdio_param;
    SD_COMMAND        cmd;
    SD_NEX_ADMA_TABLE adma_table;
};

VOID up_the_semaphore(struct function_info *, SD_FUNCTION_TYPE);
INT32 nex_thread_setup_nonblock(VOID);
VOID nex_cleanup_nonblock(VOID);

//
// These functions are from 'nex_api.c'
//
INT32 __SendData(VOID *buffer, UINT32 length, UINT32 address, INT32 sdhcNum, INT32 slotNum, SDIO_PARAM *sdio_param);
INT32 __GetData(VOID *buffer, UINT32 length, UINT32 address, INT32 sdhcNum, INT32 slotNum, SDIO_PARAM *sdio_param);
INT32 __SendDataDMA(VOID *buffer, UINT32 length, UINT32 address, INT32 sdhcNum, INT32 slotNum, SDIO_PARAM *sdio_param);
INT32 __GetDataDMA(VOID *buffer, UINT32 length, UINT32 address, INT32 sdhcNum, INT32 slotNum, SDIO_PARAM *sdio_param);

INT32 __SendCommand(SD_COMMAND *cmd, INT32 sdhcNum, INT32 slotNum);

INT32 __Setibd(VOID *buffer, UINT32 length, UINT32 entry_no, INT32 sdhcNum, INT32 slotNum, SDIO_PARAM *sdio_param);
INT32 __Getibd(VOID *buffer, UINT32 length, UINT32 entry_no, INT32 sdhcNum, INT32 slotNum, SDIO_PARAM *sdio_param);

INT32 __eSDManagePartition(VOID *buffer, UINT32 length, UINT32 opcode, INT32 sdhcNum, INT32 slotNum);
INT32 __eSDManagePartitionDMA(VOID *buffer, UINT32 length, UINT32 opcode, INT32 sdhcNum, INT32 slotNum);
INT32 __eSDQueryPartition(VOID *buffer, UINT32 length, UINT32 opcode, INT32 sdhcNum, INT32 slotNum);
INT32 __eSDQueryPartitionDMA(VOID *buffer, UINT32 length, UINT32 opcode, INT32 sdhcNum, INT32 slotNum);

#endif
