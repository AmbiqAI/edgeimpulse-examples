//*****************************************************************************
//
//! @file comm.c
//!
//! @brief uart handling and command handling of apollo scan.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "BT_common.h"
#include "appl_as.h"
#include "am_devices_em9304.h"
#include "BT_hci_api.h"
#include "comm.h"

#if 0
#define UART_HDL_TASK_PRIORITY          (tskIDLE_PRIORITY + 1)
#define CMD_HDL_TASK_PRIORITY           (UART_HDL_TASK_PRIORITY+1)
#else
//#define CMD_HDL_TASK_PRIORITY         (tskIDLE_PRIORITY + 1)
//#define UART_HDL_TASK_PRIORITY            (CMD_HDL_TASK_PRIORITY+1)
#define CMD_HDL_TASK_PRIORITY           (tskIDLE_PRIORITY + 1)
#define UART_HDL_TASK_PRIORITY          (tskIDLE_PRIORITY+1)
#endif

#define AS_TRC(...)

uint8_t g_rxBuffer[MAX_LEN_PDU] = {0};
uint8_t g_txBuffer[MAX_LEN_PDU] = {0};

volatile int write_tx_complete_flag;
volatile int bulk_mode;
volatile UINT16 ota_mtu = AMOTA_PDU_MAX;

QueueHandle_t g_uartMsgHdl;
QueueHandle_t g_cmdMsgHdl;


uint8_t g_txPkgId = 0;
// assume only deal with notification of response wiht size = 5;
volatile uint8_t otaRSPData[5];

#ifdef UART_TX_INT_ENABLE
bool g_sendPkgDoneFlag = true;
#endif

bool g_stopScanFlag = false;

const uint8_t g_firwareVersion[26] =
{
    0x00,   //1 byte status
    0x01,   //1 byte firmware version
    0x00,   //1 byte firmware sub version
    0x01,   //1 byte hardware version
    0x00,   //1 byte hardware sub version
    0x01,   //1 byte HCI version
    0x00,   //1 byte HCI sub version
    'A', 'm', 'b', 'i', 'q', 'M', 'i', 'c', 'r', 'o', '[', '2', '0', '1', '8', ']',
    18, 04, 15 //YY MM DD
};

/* uart commands table*/
const uint8_t g_cmd[] =
{
    RESET_CMD,
    GET_VER_CMD,
    GET_LOCAL_MAC_CMD,
    BL_ENTER_ISP_CMD,
    SCAN_CMD,
    STOP_SCAN_CMD,
    CONNECT_CMD,
    DIS_SVC_CMD,
    DIS_CHAR_CMD,
    WRITE_CHAR_CMD,
    READ_CHAR_CMD,
    DIS_CONNECT_CMD,
    WRITE_BULK_HEADER_CMD,
    WRITE_BULK_DATA_CMD,
    EXCHANGE_MTU_CMD,
    WRITE_CHAR_NO_RSP,
    BOND_REQ,
    HW_RESET
};

/* functions table*/
CmdHdl g_pfunCmdHdl[] =
{
    BleOn,
    GetVersion,
    GetLocalMac,
    BootLoaderEnterISPMode,
    ScanDevice,
    StopScanDevice,
    ConnectDevice,
    DiscoveryService,
    DiscoveryChar,
    WriteChar,
    ReadChar,
    DisConnectDevice,
    SendBulkHeader,
    SendBulkData,
    MTUExchangeReq,
    WriteCharNoRsp,
    BondDevice,
    HwReset
};


//*****************************************************************************
//
// dongle UART configuration settings.
//
//*****************************************************************************
am_hal_uart_config_t g_dongleUartConfig =
{
    .ui32BaudRate = DONGLE_COM_BR,
    .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
    .bTwoStopBits = false,
    .ui32Parity   = AM_HAL_UART_PARITY_NONE,
#ifdef RTS_CTS_ENABLE
    .ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_RTS_CTS | AM_REG_UART_CR_RTS_M,
#else
    .ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE,
#endif
};


//*****************************************************************************
//
// Initialize the UART
//
//*****************************************************************************
void DongleUartInit(void)
{
    int32_t i32Module = DONGLE_UART;
    //
    // Make sure the UART RX and TX pins are enabled.
    //
    am_bsp_pin_enable(COM_UART_TX);
    am_bsp_pin_enable(COM_UART_RX);
    am_bsp_pin_enable(COM_UART_CTS);
    am_bsp_pin_enable(COM_UART_RTS);
    //
    // Power on the selected UART
    //
    am_hal_uart_pwrctrl_enable(i32Module);

    //
    // Start the UART interface, apply the desired configuration settings, and
    // enable the FIFOs.
    //
    am_hal_uart_clock_enable(i32Module);

    //
    // Disable the UART before configuring it.
    //
    am_hal_uart_disable(i32Module);

    //
    // Configure the UART.
    //
    am_hal_uart_config(i32Module, &g_dongleUartConfig);

    //
    // Enable the UART FIFO.
    //
    am_hal_uart_fifo_config(i32Module, AM_HAL_UART_TX_FIFO_7_8 | AM_HAL_UART_RX_FIFO_7_8);
    //
      // Initialize the UART queues.
      //
     am_hal_uart_init_buffered(i32Module, g_rxBuffer, MAX_LEN_PDU,
                                g_txBuffer, MAX_LEN_PDU);

    //
    // Enable the UART.
    //
    am_hal_uart_enable(i32Module);

#ifdef UART_TX_INT_ENABLE

    am_hal_uart_int_enable(i32Module, AM_HAL_UART_INT_RX | AM_HAL_UART_INT_RX_TMOUT |
                                       AM_HAL_UART_INT_TXCMP);
#else
    am_hal_uart_int_enable(i32Module, AM_HAL_UART_INT_RX | AM_HAL_UART_INT_RX_TMOUT
                                       );
#endif
am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_UART + i32Module, configMAX_SYSCALL_INTERRUPT_PRIORITY);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART + i32Module);
}


void BleOn(uint8_t *parameter)
{
    am_util_stdio_printf( "BleOn().\r\n" );
    am_hal_gpio_out_bit_replace(18, 1);
    BT_bluetooth_off();
    BT_bluetooth_on
    (
        appl_hci_callback,
        appl_bluetooth_on_complete_callback,
        "EtherMind-PXM"
    );
}

void HwReset(uint8_t *parameter)
{
    am_util_stdio_printf( "HwReset().\r\n" );
    //am_hal_reset_control(AM_HAL_RESET_CONTROL_SWPOI, 0);

    am_hal_gpio_out_bit_clear(AM_BSP_GPIO_EM9304_RESET);
    am_util_delay_ms(300);
    am_hal_reset_poi();
}

void GetVersion(uint8_t *parameter)
{
    am_util_stdio_printf( "GetVersion().\r\n" );
    uint8_t cmdRspData[MAX_LEN_PDU - 2];

    cmdRspData[0] = GET_VER_RSP;
    cmdRspData[1] = sizeof (g_firwareVersion);
    cmdRspData[2] = 0;
    memcpy (&cmdRspData[3], g_firwareVersion, sizeof (g_firwareVersion));
    CmdResponse(cmdRspData);
}

void CmdInd(uint8_t *parameter)
{
    uint8_t cmdRspData[MAX_LEN_PDU - 2];

    cmdRspData[0] = CMD_IND;
    cmdRspData[1] = 0;
    cmdRspData[2] = 0;
    CmdResponse(cmdRspData);
}

void ReadLocalMacAddr(uint8_t *addr)
{
    am_util_stdio_printf( "ReadLocalMacAddr().\r\n" );
    BT_hci_get_local_bd_addr(addr);
}

void GetLocalMac(uint8_t * parameter)
{
    uint8_t mac_addr[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    am_util_stdio_printf( "GetLocalMac().\r\n" );
    uint8_t cmdRspData[MAX_LEN_PDU - 2];
    uint8_t i;

    i = 0;
    cmdRspData[i++] = GET_LOCAL_MAC_RSP;
    cmdRspData[i++] = 7;
    cmdRspData[i++] = 0;
    cmdRspData[i++] = 0;   // status: success
    ReadLocalMacAddr(&mac_addr[0]);
    memcpy (&cmdRspData[i], mac_addr, 6);
    CmdResponse(cmdRspData);
}

//am_bootloader_image_t *g_psBootImage = (am_bootloader_image_t *) FLAG_PAGE_LOCATION;

void BootLoaderEnterISPMode(uint8_t *parameter)
{
#if 0
    uint32_t iapFlag = IAP_FLASH_FLAG;

    am_util_stdio_printf( "BootLoaderEnterISPMode().\r\n" );
    // Write the flash Page
    am_bootloader_program_flash_page(ISP_FLAG_FALSH_ADDR, &iapFlag, AM_HAL_FLASH_PAGE_SIZE);
    am_util_stdio_printf( "isp flag write falsh success,system will reboot and bootloader will respone 0xAA 0xFF 0x09 0x00 0x00 0x00 \r\n" );
    am_hal_reset_poi();
#endif
}

static BT_timer_handle timer_handle;
APPL_HANDLE   apolloscan_handle;

void apolloscan_timer_handler (void *data, UINT16 datalen)
{
    if (!g_stopScanFlag)
    {
           StopScanDevice(data);
    }
}

void ScanDevice(uint8_t *parameters)
{
    am_util_stdio_printf( "ScanDevice().\r\n" );

    char scanType;
    uint16_t scanInterval;
    uint16_t scanWindow;
    char addrType;
    char scanFilterPolicy;
    API_RESULT result;

    g_stopScanFlag = false;

    scanType = *parameters++;

    scanInterval = *parameters & 0xFF;   //low byte of scan interval;
    parameters++;
    scanInterval |= ((uint16_t)*parameters << 8); //high byte of scan interval;
    parameters++;
    scanWindow = *parameters & 0xFF;   //low byte of scan Window;
    parameters++;
    scanWindow |= ((uint16_t)*parameters << 8); //high byte of scan Window;
    parameters++;

    addrType = *parameters++;

    scanFilterPolicy = *parameters;

    if (scanType == 0)
    {
        am_util_stdio_printf( "passive scanning, " );
    }
    else
    {
        am_util_stdio_printf( "active scanning, " );
    }

    am_util_stdio_printf( "scanning interval %dms, ", scanInterval );
    am_util_stdio_printf( "scanning window %dms, ", scanWindow );

    if (addrType == 0)
    {
        am_util_stdio_printf( "public address, " );
    }
    else
    {
        am_util_stdio_printf( "random address, " );
    }

    if (scanFilterPolicy == 0)
    {
        am_util_stdio_printf( "non white list.\r\n " );
    }
    else
    {
        am_util_stdio_printf( "white list.\r\n" );
    }

    result = BT_hci_le_set_scan_parameters
           (
               scanType,
               scanInterval,
               scanWindow,
               addrType,
               scanFilterPolicy
           );

    if (result != API_SUCCESS)
    {
        am_util_stdio_printf("Set scan parameters failed. Result = 0x%04X \r\n", result);
    }

    timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    BT_start_timer
             (
                 &timer_handle,
                 3,
                 apolloscan_timer_handler,
                 &apolloscan_handle,
                 sizeof (apolloscan_handle)
             );
}


void StopScanDevice(uint8_t *parameters)
{
    am_util_stdio_printf( "StopScanDevice().\r\n" );
    g_stopScanFlag = true;
    BT_hci_le_set_scan_enable (0, 1);

}

void ConnectDevice(uint8_t *parameters)
{
    am_util_stdio_printf( "ConnectDevice().\r\n" );

    UINT16   le_scan_interval;
    UINT16   le_scan_window;
    UCHAR    initiator_filter_policy;
    UCHAR    peer_address_type;
    UCHAR    peer_address[6];
    UCHAR    own_address_type;
    UINT16   conn_interval_min;
    UINT16   conn_interval_max;
    UINT16   conn_latency;
    UINT16   supervision_timeout;
    UINT16   maximum_ce_length;
    UINT16   minimum_ce_length;

    le_scan_interval = *parameters & 0xFF;   //low byte of scan interval;
    parameters++;
    le_scan_interval |= ((uint16_t)*parameters << 8); //high byte of scan interval;

    parameters++;
    le_scan_window = *parameters & 0xFF;   //low byte of scan Window;
    parameters++;
    le_scan_window |= ((uint16_t)*parameters << 8); //high byte of scan Window;

    parameters++;
    initiator_filter_policy = *parameters++;

    peer_address_type = *parameters++;

    memcpy (peer_address, parameters, 6);

    parameters = parameters + 6;
    own_address_type = *parameters;

    parameters++;
    conn_interval_min = *parameters & 0xFF;  //low byte of scan interval;
    parameters++;
    conn_interval_min |= ((uint16_t)*parameters << 8); //high byte of scan interval;

    parameters++;
    conn_interval_max = *parameters & 0xFF;  //low byte of scan interval;
    parameters++;
    conn_interval_max |= ((uint16_t)*parameters << 8); //high byte of scan interval;

    parameters++;
    conn_latency = *parameters & 0xFF;   //low byte of scan interval;
    parameters++;
    conn_latency |= ((uint16_t)*parameters << 8); //high byte of scan interval;

    parameters++;
    supervision_timeout = *parameters & 0xFF;    //low byte of scan interval;
    parameters++;
    supervision_timeout |= ((uint16_t)*parameters << 8); //high byte of scan interval;

    parameters++;
    maximum_ce_length = *parameters & 0xFF;  //low byte of scan interval;
    parameters++;
    maximum_ce_length |= ((uint16_t)*parameters << 8); //high byte of scan interval;

    parameters++;
    minimum_ce_length = *parameters & 0xFF;  //low byte of scan interval;
    parameters++;
    minimum_ce_length |= ((uint16_t)*parameters << 8); //high byte of scan interval;

    BT_hci_le_create_connection
               (
                   le_scan_interval,
                   le_scan_window,
                   initiator_filter_policy,
                   peer_address_type,
                   peer_address,
                   own_address_type,
                   conn_interval_min,
                   conn_interval_max,
                   conn_latency,
                   supervision_timeout,
                   maximum_ce_length,
                   minimum_ce_length
               );


}

void BondDevice(uint8_t *parameters)
{
    SMP_AUTH_INFO auth;
    SMP_BD_ADDR   smp_peer_bd_addr;
    SMP_BD_HANDLE smp_bd_handle;
    API_RESULT retval = API_SUCCESS;
    UCHAR    peer_address[6];
    UCHAR    peer_addr_type;

    am_util_stdio_printf( "BondDevice().\r\n" );
    am_util_stdio_printf("peer addr: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", parameters[0], parameters[1]
                         , parameters[2], parameters[3], parameters[4], parameters[5]);

    memcpy (peer_address, parameters, 6);
    peer_addr_type = *(parameters + 6);

    //APPL_SET_STATE(DEVICE_CONNECTED);

    auth.param = 1;
    auth.bonding = 1;
    auth.ekey_size = 12;
    auth.security = SMP_SEC_LEVEL_1;
    BT_COPY_BD_ADDR(smp_peer_bd_addr.addr, peer_address);
    BT_COPY_TYPE(smp_peer_bd_addr.type, peer_addr_type);

    retval = BT_smp_get_bd_handle
             (
                 &smp_peer_bd_addr,
                 &smp_bd_handle
             );

    if (API_SUCCESS == retval)
    {
        retval = BT_smp_authenticate (&smp_bd_handle, &auth);
    }

    if (API_SUCCESS != retval)
    {
        am_util_stdio_printf (
        "Initiation of Authentication Failed. Reason 0x%04X\n",
        retval);
    }
}


const unsigned int crc32Table[256] =
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

unsigned int calcCrc32(int len, unsigned char* buf)
{
    unsigned int crc = 0xFFFFFFFF;
    unsigned char a, b;
    for (int i = 0; i < len; i++)
    {
        a = (buf[i] & 0xff);
        b = (crc & 0xff);
        crc = crc32Table[a ^ b] ^ (crc >> 8);
    }
    crc = crc ^ 0xFFFFFFFF;
    return crc;
}

write_bulk_data_t  bulk_data;
uint8_t bulk_data_packet[AMOTA_BULK_DATA_UNIT_SIZE];

API_RESULT WriteBulkData(uint8_t *parameters, int len)
{

    ATT_HANDLE bulk_client_handle;
    ATT_WRITE_CMD_PARAM write_cmd_param;
    API_RESULT retval;

    bulk_client_handle.device_id = bulk_data.hdl.device_id;
    bulk_client_handle.att_id = bulk_data.hdl.att_id;

    write_cmd_param.handle = bulk_data.charHdl;
    write_cmd_param.value.val = parameters;
    write_cmd_param.value.len = len;

    retval = BT_att_send_write_cmd
             (
                 &bulk_client_handle,
                 &write_cmd_param
             );

    return retval;
}

int wait_to_print;
int total_send = 0;
int sendPacket(uint8_t *data, int len)
{
    int pack_size = ota_mtu;
    int j;
    int time_out;
    j = 0;
    for ( int i = 0; i < len; i += pack_size )
    {
        int data_len = (i + pack_size) > len ? len % pack_size : pack_size;
        write_tx_complete_flag = 0;
        WriteBulkData(&data[i], data_len);
        total_send += data_len;
        j++;
        //if (j==3) return 3;
        wait_to_print = 1;
        time_out = ATT_WRITE_TX_COMPLETE_TIME_OUT;
        while(!write_tx_complete_flag && time_out--)
        {
        }
#if 0
        vTaskDelay ((portTickType)((3) / (portTICK_RATE_MS)));
//      am_util_delay_ms(10);
#else
#endif
    }
    return len;
}

int sendOtaCmd(int cmd, uint8_t *data, int len)
{
    unsigned int checksum = 0;
    int packetLength = AMOTA_HEADER_SIZE_IN_PKT + len + AMOTA_CRC_SIZE_IN_PKT;

    // fill data + checksum length
    bulk_data_packet[0] = (uint8_t)(len + AMOTA_CRC_SIZE_IN_PKT);
    bulk_data_packet[1] = (uint8_t)((len + AMOTA_CRC_SIZE_IN_PKT) >> 8);
    bulk_data_packet[2] = cmd;

    if (len != 0)
    {
        // calculate CRC
        checksum = calcCrc32(len, data);
        // copy data into packet
        memcpy(&bulk_data_packet[AMOTA_HEADER_SIZE_IN_PKT], data, len);
    }

    // append crc into packet
    // crc is always 0 if there is no data only command
    bulk_data_packet[AMOTA_HEADER_SIZE_IN_PKT + len] = ((uint8_t)(checksum));
    bulk_data_packet[AMOTA_HEADER_SIZE_IN_PKT + len + 1] = ((uint8_t)(checksum >> 8));
    bulk_data_packet[AMOTA_HEADER_SIZE_IN_PKT + len + 2] = ((uint8_t)(checksum >> 16));
    bulk_data_packet[AMOTA_HEADER_SIZE_IN_PKT + len + 3] = ((uint8_t)(checksum >> 24));

    if (sendPacket(bulk_data_packet, packetLength))
    {
        return 1;
    }
    else
    {
        //mAmotaCallback.otaFaild("sendPacket failed");
        return 0;
    }
}


void ReceiveBulkData(uint8_t *parameters)
{
    int i, len;
//  am_util_stdio_printf( "ReceiveBulkData(): %d\r\n",  bulk_data.idx);

       // special in for bulk data length only
    len = parameters[0] | parameters[1] << 8;
       bulk_data.unit_len = len;
    am_util_stdio_printf( "bulk_data.unit_len: %d\r\n", bulk_data.unit_len);

    // payload data from [1]
     //  memcpy(&bulk_data_temp[0], &parameters[1], len);
    bulk_data.idx += len;
    bulk_data.unit_checksum = 0;
    for (i = 0; i < len; i++)
    {
        bulk_data.sum += parameters[i + 2];
        bulk_data.unit_checksum += parameters[i + 2];
    }

    if (bulk_data.idx == bulk_data.size)
    {
        // bulk data pkt finished
        if (bulk_data.sum != bulk_data.checksum)
        {
            am_util_stdio_printf( "bulk_data.checksum error: %x\r\n", bulk_data.sum);
        }
        else
        {
            am_util_stdio_printf( "bulk_data.checksum ok: %x\r\n", bulk_data.sum);
        }
    }
}

void SendBulkData(uint8_t *parameters)
{
       int i;
    uint8_t cmdRspData[MAX_LEN_PDU - 2];
    am_util_stdio_printf( "SendBulkData().\r\n" );

    ReceiveBulkData(parameters);
    sendOtaCmd(AMOTA_CMD_FW_DATA, &parameters[2], bulk_data.unit_len);

       i = 0;
    cmdRspData[i++] = WRITE_BULK_DATA_RSP;
    cmdRspData[i++] = 1 + 2 + 2 + 5;
    cmdRspData[i++] = 0;
    cmdRspData[i++] = 0x00;   // 0: success, n: failed with something wrong, TBD (to be defined)
    cmdRspData[i++] = bulk_data.idx;
    cmdRspData[i++] = bulk_data.idx >> 8;
    cmdRspData[i++] = bulk_data.unit_checksum;
    cmdRspData[i++] = bulk_data.unit_checksum >> 8;
    cmdRspData[i++] = otaRSPData[0];
    cmdRspData[i++] = otaRSPData[1];
    cmdRspData[i++] = otaRSPData[2];
    cmdRspData[i++] = otaRSPData[3];
    cmdRspData[i++] = otaRSPData[4];
    CmdResponse(cmdRspData);
}

void SendBulkHeader(uint8_t *parameters)
{
    uint8_t cmdRspData[MAX_LEN_PDU - 2];

    am_util_stdio_printf( "SendBulkHeader().\r\n" );

    bulk_data.hdl.device_id = *parameters++;
    bulk_data.hdl.att_id = *parameters++;

    bulk_data.charHdl = *parameters & 0xFF;  //low byte of char handle;
    parameters++;
    bulk_data.charHdl |= ((ATT_ATTR_HANDLE)*parameters << 8); //high byte of char handle;

    parameters++;
    bulk_data.val.len = *parameters & 0xFF;  //low byte of char data;
    parameters++;
    bulk_data.val.len |= ((UINT16)*parameters << 8); //high byte of char data;

    parameters++;
    bulk_data.checksum = *parameters & 0xFF;
    parameters++;
    bulk_data.checksum |= ((UINT16)*parameters << 8); //high byte of char data;

    // reset received data checksum to 0
       bulk_data.sum = 0;
    bulk_data.idx = 0;

    cmdRspData[0] = WRITE_BULK_HEADER_RSP;
    cmdRspData[1] = 1;
    cmdRspData[2] = 0;
    cmdRspData[3] = 0x00;     // 0: success, n: failed with something wrong, TBD (to be defined)

    CmdResponse(cmdRspData);
    if (bulk_data.checksum)
    {
       bulk_mode = 1;
    }
    else
    {
        bulk_mode = 0;
    }
}

void MTUExchangeReq(uint8_t *parameters)
{
    am_util_stdio_printf ("Enter the MTU for the ATT connection\n");
//  scanf ("%x", &data);
  uint16_t mtu;
  ATT_HANDLE hdl;
    API_RESULT retval;

    hdl.device_id = *parameters++;
    hdl.att_id = *parameters++;

    mtu = *parameters & 0xFF;   //low byte of char handle;
    parameters++;
    mtu |= *parameters << 8;         //high byte of char handle;

    retval = BT_att_send_mtu_xcnhg_req
             (
                 &hdl,
                 &mtu
             );

    am_util_stdio_printf("Exchange MTU REQ retval - 0x%04X\n", retval);
}



void DisConnectDevice(uint8_t *parameters)
{
    am_util_stdio_printf( "DisConnectDevice().\r\n" );

    UINT16   connHandle;
    UCHAR    reason;

    /*get connetion handle of disconenction*/
    connHandle = *parameters & 0xFF;     //low byte of scan interval;
    parameters++;
    connHandle |= ((UINT16)*parameters << 8); //high byte of scan interval;

    /*get reason code of disconneciton*/
    parameters++;
    //reason = *parameters;
    reason = 0x13;
    BT_hci_disconnect
               (
                   connHandle,
                   reason
               );


}

void DiscoveryService(uint8_t *parameters)
{
    am_util_stdio_printf( "DiscoveryService().\r\n" );
    UINT8 uuid_type;
    UINT16 start_handle, end_handle, uuid16;
    int i;

    i = 2;
    start_handle = parameters[i] | (parameters[i + 1] << 8);
    i += 2;
    end_handle = parameters[i] | (parameters[i + 1] << 8);
    i += 2;
    uuid_type = parameters[i++];
    uuid16 = parameters[i] | (parameters[i + 1] << 8);

#if 0
    if (uuid_type == ATT_16_BIT_UUID_FORMAT)
    {
        uuid.uuid_16 = *parameters & 0xFF;
        parameters++;
        uuid.uuid_16 |= (UINT16)*parameters << 8;
        am_util_stdio_printf( "discovery service UUID tpye is 16-bit, UUID is 0x%04x.\r\n", uuid.uuid_16 );
    }
    else if (uuid_type == ATT_128_BIT_UUID_FORMAT)
    {
        am_util_stdio_printf( "discovery service UUID tpye is 128-bit, UUID is:" );
        for (UINT8 i = 0; i < 16; i++)
        {
            uuid.uuid_128.value[i] = *parameters;
            am_util_stdio_printf( "0x%02x, ",*parameters );
            parameters++;
        }
        am_util_stdio_printf( "\r\n " );
    }
    else
    {
        am_util_stdio_printf( "discovery service UUID tpye is error!!!!!!!!:" );
    }
#endif

    appl_discover_all_service(start_handle, end_handle, uuid16, uuid_type);
}


void DiscoveryChar(uint8_t *parameters)
{
    am_util_stdio_printf( "DiscoveryChar().\r\n" );
    UINT16 start_handle, end_handle, uuid16;
    UINT8 uuid_type;

    int i;

    i = 2;
    start_handle = parameters[i] | (parameters[i + 1] << 8);
    i += 2;
    end_handle = parameters[i] | (parameters[i + 1] << 8);
    i += 2;
    uuid_type = parameters[i++];
    uuid16 = parameters[i] | (parameters[i + 1] << 8);

    appl_discover_all_characteristics(start_handle, end_handle, uuid16, uuid_type);
}

void WriteChar(uint8_t *parameters)
{
    am_util_stdio_printf( "WriteChar().\r\n" );
    ATT_WRITE_REQ_PARAM write_req_param;
    ATT_ATTR_HANDLE charHdl;
    ATT_HANDLE hdl;
    ATT_VALUE val;
    UINT8 charData[MAX_ATT_LEN];
    API_RESULT retval;


    hdl.device_id = *parameters++;
    hdl.att_id = *parameters++;

    charHdl = *parameters & 0xFF;    //low byte of char handle;
    parameters++;
    charHdl |= ((ATT_ATTR_HANDLE) * parameters << 8); //high byte of char handle;

    parameters++;
    val.len = *parameters & 0xFF;    //low byte of char data;
    parameters++;
    val.len |= ((UINT16) * parameters << 8); //high byte of char data;

    parameters++;
    memcpy(charData, parameters, val.len);

    val.val = charData;

    write_req_param.handle = charHdl;
    write_req_param.value = val;

    retval = BT_att_send_write_req
             (
                 &hdl,
                 &write_req_param
             );

    am_util_stdio_printf( "BT_att_send_write_req retval: %d\r\n", retval);
    // bulk mode will never use write request
           bulk_mode = 0;
}

void WriteCharNoRsp(uint8_t *parameters)
{
    //am_util_stdio_printf( "WriteCharNoRsp().\r\n" );
    ATT_WRITE_REQ_PARAM write_req_param;
    ATT_ATTR_HANDLE charHdl;
    ATT_HANDLE hdl;
    ATT_VALUE val;
    UINT8 charData[MAX_ATT_LEN];
    API_RESULT retval;

    hdl.device_id = *parameters++;
    hdl.att_id = *parameters++;

    charHdl = *parameters & 0xFF;    //low byte of char handle;
    parameters++;
    charHdl |= ((ATT_ATTR_HANDLE) * parameters << 8); //high byte of char handle;

    parameters++;
    val.len = *parameters & 0xFF;    //low byte of char data;
    parameters++;
    val.len |= ((UINT16) * parameters << 8); //high byte of char data;

    parameters++;
    memcpy(charData, parameters, val.len);

    val.val = charData;

    write_req_param.handle = charHdl;
    write_req_param.value = val;

    retval = BT_att_send_write_cmd(&hdl, &write_req_param);
    if (retval)
    {
        am_util_stdio_printf( "BT_att_send_write_cmd retval: %d\r\n", retval);
    }
}

void ReadChar(uint8_t *parameters)
{
    am_util_stdio_printf( "ReadChar().\r\n" );

    ATT_ATTR_HANDLE charHdl;
    ATT_HANDLE hdl;

    hdl.device_id = *parameters++;
    hdl.att_id = *parameters++;

    charHdl = *parameters & 0xFF;    //low byte of char handle;
    parameters++;
    charHdl |= ((ATT_ATTR_HANDLE) * parameters << 8); //high byte of char handle;

    BT_att_send_read_req(&hdl, &charHdl);
}

void ExchangeMtu(uint8_t *parameters)
{
    ATT_HANDLE hdl;
    UINT16 mtu;

    am_util_stdio_printf( "ExchangeMtu().\r\n" );

    hdl.device_id = *parameters++;
    hdl.att_id = *parameters++;

    mtu = *parameters & 0xFF;    //low byte of char handle;
    parameters++;
    mtu |= ((ATT_ATTR_HANDLE) * parameters << 8); //high byte of char handle;

    BT_att_send_mtu_xcnhg_req(&hdl, &mtu);
}
//*****************************************************************************
//
// UART1 Interrupt Service Routine
//
//*****************************************************************************
void am_uart_isr(void)
{
    uint32_t status;
    uint32_t rxSize, txSize;
    BaseType_t xHigherPriorityTaskWokenByPost;
    uart_msg_t uart_msg;

    /* get the status of Dongle UART interrupt*/
    status = am_hal_uart_int_status_get(DONGLE_UART, false);

    /*clear this time dongle UART interrupt status*/
    am_hal_uart_int_clear(DONGLE_UART, status);

    if (status & (AM_HAL_UART_INT_RX | AM_HAL_UART_INT_RX_TMOUT | AM_HAL_UART_INT_TXCMP))
    {
        am_hal_uart_service_buffered_timeout_save(DONGLE_UART, status);
    }

    if (status & (AM_HAL_UART_INT_RX_TMOUT))
    {
        am_hal_uart_get_status_buffered(DONGLE_UART, &rxSize, &txSize);
        am_hal_uart_char_receive_buffered(DONGLE_UART, (char *)&uart_msg.data[0], rxSize);
        // We have not woken a task at the start of the ISR.
        xHigherPriorityTaskWokenByPost = pdFALSE;
        /*send queue from isr, queue includes rx len and original data*/
        uart_msg.rxLen = rxSize;

        if ((uart_msg.data[0] == 0xAA) &&
            (uart_msg.data[2] == 0x99) &&
            (uart_msg.data[3] == 0x00))
        {
            HwReset(NULL);
        }

        xQueueSendFromISR(g_uartMsgHdl, (void *)&uart_msg, &xHigherPriorityTaskWokenByPost);

        // Now the buffer is empty we can switch context if necessary.  Note that the
        // name of the yield function required is port specific.
        if ( xHigherPriorityTaskWokenByPost )
        {
            //portYIELD_FROM_ISR();

            taskYIELD ();
        }
    }

    if (status & (AM_HAL_UART_INT_RX))
    {
       // am_hal_uart_get_status_buffered(DONGLE_UART, &rxSize, &txSize);

    }
#ifdef UART_TX_INT_ENABLE
    if (status & (AM_HAL_UART_INT_TXCMP))
    {
        g_sendPkgDoneFlag = true;
    }
#endif
}

void SendMsgToCmdTask(uart_msg_t * msg)
{

    if (msg->data[0] == HEADER_OF_PDU)
    {
        /*    */
        xQueueSend(g_cmdMsgHdl, &msg->data[2], 0xFF);
    }
}

void CmdResponse(uint8_t * cmdRspData)
{
    uint8_t rsp_pkg[MAX_LEN_PDU];
    uint16_t len_cmd_data;
    uint16_t len_pkg;
    uint16_t i;

    rsp_pkg[0] = HEADER_OF_PDU;
    rsp_pkg[1] = g_txPkgId++;


    /* length = 1byte cmd + 1byte data length + data*/
    len_cmd_data = 1 + 2 + (cmdRspData[1] | cmdRspData[2] << 8); //0 is cmd, 2 is length of data
    memcpy(&rsp_pkg[2], cmdRspData, len_cmd_data);
    len_pkg = 1 + 1 + len_cmd_data;
    /*start transmiting a package from UART*/
#ifdef UART_TX_INT_ENABLE
    while (!g_sendPkgDoneFlag);  //waiting for last package is sent done;
    g_sendPkgDoneFlag = false;
#endif
#if 0
    am_util_stdio_printf( "UART TX %d, cmd: 0x%02x\n", len_pkg, cmdRspData[0]);

    for (i=0; i<len_pkg; i++)
    {
        am_util_stdio_printf( "0x%02x ,", rsp_pkg[i] );
        am_hal_uart_char_transmit_buffered(DONGLE_UART, rsp_pkg[i]);
    }
    am_util_stdio_printf( "\r\n" );
#else
    for (i=0; i<len_pkg; i++)
    {
        am_hal_uart_char_transmit_polled(DONGLE_UART, rsp_pkg[i]);
    }
#endif
}

void UartHandleTask(void * pvParameters)
{
    portBASE_TYPE xStatus;
    uart_msg_t data;

    /*enable 32768HZ cyrstal clock and use it to calibrate HFRC*/
    am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_XT);
    am_hal_clkgen_hfrc_adjust_enable(AM_REG_CLKGEN_HFADJ_HFWARMUP_1SEC, AM_REG_CLKGEN_HFADJ_HFADJCK_4SEC);

    DongleUartInit();
    g_uartMsgHdl = xQueueCreate(UART_MSG_INSTANCE, sizeof(uart_msg_t *) + MAX_LEN_PDU);
    if (g_uartMsgHdl == NULL)
    {
        am_util_stdio_printf("Creating uart msg is failure");
        while(1);
    }
    for ( ; ; )
    {
        xStatus = xQueueReceive(g_uartMsgHdl, &data, 0xFF);
         if ( xStatus == pdPASS )
        {
            static int totalLen = 0;
            totalLen += data.rxLen;

            am_util_stdio_printf( "UART RX %d, ", totalLen);
#if 0
            for (uint8_t i = 0; i< data.rxLen; i++)
            {
                am_util_stdio_printf( "%02x ", data.data[i] );
            }
            am_util_stdio_printf( "\r\n" );
#endif
         SendMsgToCmdTask(&data);
        }
        else
        {
            //am_util_stdio_printf( "Could not receive from the queue.\r\n" );
        }
    }
}

void CmdHandleTask(void * pvParameters)
{
    portBASE_TYPE xStatus;
    #define  msgLen (MAX_LEN_PDU - 2) //remove 1 byte hear+1 byte id of package
    uint8_t cmdData[msgLen];
    uint8_t cmd;
    uint8_t idx;

    g_cmdMsgHdl = xQueueCreate(UART_MSG_INSTANCE, msgLen);

    for ( ; ; )
    {
        //vTaskDelay(100);
        xStatus = xQueueReceive(g_cmdMsgHdl, cmdData, 10);
        if ( xStatus == pdPASS )
        {
            /* comData[0]: cmd,  comData[1]: length of PDU, others: parameters of PDU */
            cmd = cmdData[0];

            //am_util_stdio_printf( "0x%02x - %02x%02x", cmd, cmdData[9], cmdData[10]);
            am_util_stdio_printf( "%02x%02x, ", cmdData[9], cmdData[10]);
#if 0
//          for (uint8_t i = 7; i< ((1 + cmdData[1]) > 15)?15:(1 + cmdData[1]); i++)
//          {
//              am_util_stdio_printf( "%02x ", cmdData[i + 1] );
//          }
//          am_util_stdio_printf( "\n", cmd);
#endif
            /* get index from commands table*/
            for (idx = 0; idx < sizeof(g_cmd); idx++)
            {
                if (g_cmd[idx] == cmd)
                {
                    break;
                }
            }

            /* excution command if uart received a comand is active*/
            if (idx < sizeof(g_cmd))
            {
#if 1
                if ((bulk_mode == 1) && (cmd == WRITE_CHAR_NO_RSP))
                {
                    // indicate the PC to send more data
                    CmdInd(&cmd);
                }
#endif
                am_util_stdio_printf("%04X\r\n", g_pfunCmdHdl[idx]);
                /* only transmiting parameters of PDU to sub rountine*/
                if (cmd == WRITE_BULK_DATA_CMD)
                {
                    g_pfunCmdHdl[idx](&cmdData[1]);
                }
                else
                {
                    g_pfunCmdHdl[idx](&cmdData[3]);
                }
            }
            else
            {
                /* uart received a unkown command*/
                am_util_stdio_printf( "an unknown cmd !!!!!!!!\r\n" );
                while(1);
            }

        }
        else
        {
            //am_util_stdio_printf( "Could not receive from the queue.\r\n" );
        }
    }
}


/* Function that creates a task. */
void CreateCommTask(void)
{
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;
    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    UartHandleTask,           /* Function that implements the task. */
                    "UartHandleTask",          /* Text name for the task. */
                    512*16,                /* Stack size in words, not bytes. */
                    ( void * ) 1,       /* Parameter passed into the task. */
                    UART_HDL_TASK_PRIORITY,   /* Priority at which the task is created. */
                    &xHandle );         /* Used to pass out the created task's handle. */

    if ( xReturned == pdPASS )
    {
        /* The task was created.  Use the task's handle to delete the task. */
        //vTaskDelete( xHandle );
    }

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    CmdHandleTask,           /* Function that implements the task. */
                    "CmdHandleTask",          /* Text name for the task. */
                    512*16,                /* Stack size in words, not bytes. */
                    ( void * ) 1,       /* Parameter passed into the task. */
                    CMD_HDL_TASK_PRIORITY,   /* Priority at which the task is created. */
                    &xHandle );         /* Used to pass out the created task's handle. */

    if ( xReturned == pdPASS )
    {
        /* The task was created.  Use the task's handle to delete the task. */
        //vTaskDelete( xHandle );
    }

}

