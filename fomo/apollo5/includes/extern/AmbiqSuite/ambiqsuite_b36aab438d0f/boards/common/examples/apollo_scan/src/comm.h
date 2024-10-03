//*****************************************************************************
//
//! @file comm.h
//!
//! @brief header file for apollo scan application
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef __COMM__
#define __COMM__

#include <stdint.h>
#include <stdbool.h>

/* definition communication UART port*/
#define    DONGLE_UART                              0

/*UART TX with interrupt strategy enable*/
//#define UART_TX_INT_ENABLE

/*UART with flow control enable*/
//#define RTS_CTS_ENABLE

/*definition UART baud rate*/
#define DONGLE_COM_BR                               (115200*4)

/* definition message instance*/
#define UART_MSG_INSTANCE                           10


/* definition the Header of PDU*/
#define HEADER_OF_PDU                               0xAA
/* definition the maximum length of PDU*/
#define MAX_LEN_PDU                                 262

/* definition the maximum length of attribute*/
#define MAX_ATT_LEN                                 512

/* definition of maximum bulk data size */
#define MAX_BULK_DATA_LEN     4*1024

#define AMOTA_LENGTH_SIZE_IN_PKT    2
#define AMOTA_CMD_SIZE_IN_PKT       1
#define AMOTA_CRC_SIZE_IN_PKT       4
#define AMOTA_HEADER_SIZE_IN_PKT   (AMOTA_LENGTH_SIZE_IN_PKT + AMOTA_CMD_SIZE_IN_PKT)
#define AMOTA_FW_HEADER_SIZE        48
#define AMOTA_BULK_DATA_UNIT_SIZE   (AMOTA_HEADER_SIZE_IN_PKT + 240 + AMOTA_CRC_SIZE_IN_PKT)
#define AMOTA_PDU_MAX    20

#define ATT_WRITE_TX_COMPLETE_TIME_OUT      1000*1000

#define ISP_FLAG_FALSH_ADDR                 0x4000
#define IAP_FLASH_FLAG                      0x12345678

typedef UCHAR APPL_HANDLE;

/*definition commands*/
enum
{
    RESET_CMD               = 0x02,
    RESET_RSP               = 0x03,
    GET_VER_CMD             = 0x04,
    GET_VER_RSP             = 0x05,
    GET_LOCAL_MAC_CMD       = 0x06,
    GET_LOCAL_MAC_RSP       = 0x07,
    BL_ENTER_ISP_CMD        = 0x08,
    BL_ENTER_ISP_RSP        = 0x09,
    SCAN_CMD                = 0x20,
    SCAN_RSP                = 0x21,
    STOP_SCAN_CMD           = 0x22,
    STOP_SCAN_RSP           = 0x23,
    CONNECT_CMD             = 0x24,
    CONNECT_RSP             = 0x25,
    DIS_SVC_CMD             = 0x26,
    DIS_SVC_RSP             = 0x27,
    DIS_CHAR_CMD            = 0x28,
    DIS_CHAR_RSP            = 0x29,
    WRITE_CHAR_CMD          = 0x2A,
    WRITE_CHAR_RSP          = 0x2B,
    READ_CHAR_CMD           = 0x2C,
    READ_CHAR_RSP           = 0x2D,
    DIS_CONNECT_CMD         = 0x2E,
    DIS_CONNECT_RSP         = 0x2F,
    WRITE_BULK_HEADER_CMD   = 0x30,
    WRITE_BULK_HEADER_RSP   = 0x31,
    WRITE_BULK_DATA_CMD     = 0x32,
    WRITE_BULK_DATA_RSP     = 0x33,
    EXCHANGE_MTU_CMD        = 0x34,
    EXCHANGE_MTU_RSP        = 0x35,
    WRITE_CHAR_NO_RSP       = 0x36,
    BOND_REQ                = 0x40,
    BOND_RSP                = 0x41,
    CMD_IND                 = 0x66,
    HW_RESET                = 0x99
};

/* amota commands */
enum eAmotaCommand
{
    AMOTA_CMD_UNKNOWN,
    AMOTA_CMD_FW_HEADER,
    AMOTA_CMD_FW_DATA,
    AMOTA_CMD_FW_VERIFY,
    AMOTA_CMD_FW_RESET,
    AMOTA_CMD_MAX
};



/*define uart msg type*/
typedef struct uart_msg
{
    uint16_t rxLen;
    uint8_t data[MAX_LEN_PDU];
}uart_msg_t;

typedef struct
{
    uint16_t idx;
    int size;
    uint16_t unit_len;
    uint16_t checksum;
    uint16_t unit_checksum;
    uint16_t sum;
    ATT_WRITE_REQ_PARAM write_req_param;
    ATT_ATTR_HANDLE charHdl;
    ATT_HANDLE hdl;
    ATT_VALUE val;
    UINT8 charData[MAX_BULK_DATA_LEN];
    API_RESULT retval;
}write_bulk_data_t;

extern bool g_stopScanFlag;


typedef void (*CmdHdl)(uint8_t *paramete);

void DongleUartInit(void);

void BleOn(uint8_t *parameter);
void HwReset(uint8_t *parameter);
void GetVersion(uint8_t * parameter);
void CmdInd(uint8_t * parameter);
void GetLocalMac(uint8_t * parameter);
void BootLoaderEnterISPMode(uint8_t *parameter);
void ScanDevice(uint8_t * parameter);
void StopScanDevice(uint8_t *parameter);
void ConnectDevice(uint8_t * parameter);
void DisConnectDevice(uint8_t *parameters);
void DiscoveryService(uint8_t * parameter);
void DiscoveryChar(uint8_t * parameter);
void WriteChar(uint8_t * parameter);
void ReadChar(uint8_t * parameter);
void SendBulkHeader(uint8_t *parameters);
void SendBulkData(uint8_t *parameters);
void WriteCharNoRsp(uint8_t * parameter);
void SendMsgToCmdTask(uart_msg_t * msg);
void CmdResponse(uint8_t * cmdRspData);
void UartHandleTask( void * pvParameters );
void CmdHandleTask(void * pvParameters);
void MTUExchangeReq(uint8_t *parameters);
void BondDevice(uint8_t *parameters);
extern void CreateCommTask(void);


#endif
