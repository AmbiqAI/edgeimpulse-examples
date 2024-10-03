//____________________________________________________________________
// Copyright Info      : All Rights Reserved - (c) Arasan Chip Systems Inc.,
//
// Name                : nex_api.h
//
// Creation Date       : 12-Feb-2007
//
// Description         : This file contains the code for interfacing with the MMC and SD driver
//
// Changed History     :
//
//<Date>             <Author>      <Version>           <Description>
// 12 Feb 2007          VS            0.01                 base header
//
//____________________________________________________________________

#ifndef _NEX_API_H
#define _NEX_API_H

//--------------+
// OS Header Files |
//--------------+
#include "nex_porting.h"

#define SOFTWARE_LOOPBACK 0
#define SD_FALSE          0
#define SD_TRUE           1
#define MSPRO_READ_ONLY   2

#define SD_SUCCESS 0
#define SD_FAILURE -1

#define SD_MAX_SLOTS 2

#define MEMPRO_READ  0x0 // MEMPRO Read
#define MEMPRO_WRITE 0x1 // MEMPRO Write

#define eBOOT_TOT_SIZE 0x2000
#if 0
typedef enum sd_power_mode
{

    SD_1_8_VDD = 0,
    SD_3_0_VDD = 1,
    SD_3_3_VDD = 2,
    SD_MAX_POWER_MODE = SD_3_3_VDD

} SD_POWER_MODE;
#endif
typedef enum _sd_power_enable_disable
{
    SD_POWER_DISABLE = 0,
    SD_POWER_ENABLE = 1,
    SD_MAX_POWER_ENABLE_DISABLE = SD_POWER_ENABLE
} SD_POWER_ENABLE_DISABLE;

typedef enum _sd_dma_enable
{
    SD_DMA_DISABLE = 0,
    SD_DMA_ENABLE = 1,
    SD_MAX_DMA_MODE = SD_DMA_ENABLE
} SD_DMA_MODE;

typedef enum _sd_dma_type
{
    SD_SINGLE_DMA = 0,
    SD_ADMA = 1,
    SD_MAX_DMA_TYPE = SD_ADMA
} SD_DMA_TYPE;

typedef enum _sd_clock_divider
{
    SD_DIVIDER_256 = 1, /*  It has to be 1 b'coz the value can
                            used for shifting the clock register
                            0x2ch value by the same number.   */
    SD_DIVIDER_128,
    SD_DIVIDER_64,
    SD_DIVIDER_32,
    SD_DIVIDER_16,
    SD_DIVIDER_8,
    SD_DIVIDER_4,
    SD_DIVIDER_2,
    SD_DIVIDER_1,
} SD_CLOCK_DIVIDER;

typedef enum _sd_autocmd
{
    SD_AUTOCMD_DISABLE = 0,
    SD_AUTOCMD12,
    SD_AUTOCMD23,
} SD_AUTOCMD;

#define SD_VALID_DIVIDER 9

//--------------------+     	 Added on March 1 for Error feteching in
//  Error Code for the API |     all the files of nex_pci.Error interrupt return status
//--------------------+
#define SD_INVALID_POWER_MODE           (-1)
#define SD_INCORRECT_SLOT_NUM           (-2)
#define SD_BUFFER_NOT_ALLOCATED         (-3)
#define SD_COMMAND_TIMEOUT_ERROR        (-4)
#define SD_COMMAND_CRC_ERROR            (-5)
#define SD_COMMAND_ENDBIT_ERROR         (-6)
#define SD_COMMAND_INDEX_ERROR          (-7)
#define SD_DATA_TIMEOUT_ERROR           (-8)
#define SD_DATA_CRC_ERROR               (-9)
#define SD_DATA_ENDBIT_ERROR            (-10)
#define SD_CURRENT_LIMIT_ERROR          (-11)
#define SD_AUTO_CMD12_ERROR             (-12)
#define SD_CARD_BUSY_ERROR              (-13)
#define SD_CARD_INVALID_ERROR           (-14)
#define SD_FAIL_ERROR                   (-15)
#define SD_UNDEFINED_ERROR              (-16)
#define SD_LENGTH_ZERO_ERROR            (-17)
#define SD_CARD_NOT_PRESENT             (-18)
#define SD_INVALID_CLOCK_DIVIDER        (-19)
#define SD_CARD_READ_ONLY               (-20)
#define SD_NOT_SUPPORTED                (-22)
#define eSD_OUT_OF_RANGE                (-23)
#define eSD_NEG_PARTITON                (-24)
#define eSD_UNKNOWN_ERROR               (-25)
#define SD_ADMA_ERROR                   (-26)
#define SDXC_DRIV_STRE_CONFIG_ERROR     (-27)
#define SDXC_DRIV_STRE_HOST_ERROR       (-28)
#define SDXC_TUNING_ERROR               (-29)
#define SDXC_CURRENT_LIMIT_CONFIG_ERROR (-30)
#define SDXC_CMD_SYS_CONFIG_ERROR       (-31)
#define SDXC_ACCESS_MODE_CONFIG_ERROR   (-32)
#define SD_ERR_DRV_TIMEOUT              (-33)

//---------------------+
//   End of All Error Status  |
//---------------------+

//--------------------+
//   MMC Response Types |
//--------------------+
#define MMC_RSP_PRESENT   (1 << 0)
#define MMC_RSP_136       (1 << 1) // 136 bit response
#define MMC_RSP_CRC       (1 << 2) // expect valid crc
#define MMC_RSP_BUSY      (1 << 3) // card may send busy
#define MMC_RSP_OPCODE    (1 << 4) // response contains opcode
#define MMC_CMD_MASK      (3 << 5) // command type
#define MMC_CMD_AC        (0 << 5) // Addressed (point-to-point) commands
#define MMC_CMD_ADTC      (1 << 5) // Addressed (point-to-point) data transfer commands
#define MMC_CMD_BC        (2 << 5) // Broadcast commands
#define MMC_CMD_BCR       (3 << 5) // Broadcast commands with response
#define MSPRO_RSP_PRESENT (1 << 6)
#define MMC_CMD_SUSPEND   (1 << 6)
#define MMC_CMD_RESUME    (1 << 7)

#define MMC_RSP_NONE (0)
#define MMC_RSP_R1   (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)
#define MMC_RSP_R1B  (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE | MMC_RSP_BUSY)
#define MMC_RSP_R2   (MMC_RSP_PRESENT | MMC_RSP_136 | MMC_RSP_CRC)

#define MMC_RSP_R4 (MMC_RSP_PRESENT)
#define MMC_RSP_R5 (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE | MMC_RSP_BUSY)

#define MMC_RSP_R3 (MMC_RSP_PRESENT)
#define MMC_RSP_R6 (MMC_RSP_PRESENT | MMC_RSP_CRC)
#define MMC_RSP_R7 (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)

//------------------------+
// Response Processing R4b     |
//------------------------+
#define SDIO_MEM_PRES 0x08000000
// SDIO3.0
#define SDIO_1_8V_SUPPORT  0x01000000
#define R4_NUM_OF_FUNCTION 0x70000000
#define INVALID_RESP       5
#define R1_APP_CMD         (1 << 5)

//--------------------------------+
// Response R5 - used in case of SDIO   |
//--------------------------------+
#define R5_COM_CRC_ERROR       (1 << 15)                // er, b
#define R5_ILLEGAL_COMMAND     (1 << 15)                // er, b */
#define R5_IO_CURRENT_STATE(x) ((x & 0x00003000) >> 12) // s, b (2 bits)
#define R5_ERROR               (1 << 11)                // erx, c
#define R5_FUNCTION_NUMBER     (1 << 9)                 // er, c
#define R5_OUT_OF_RANGE        (1 << 8)                 // er, c
//-------------------------+
// End of MMC Response Types. |
//-------------------------+

#define ADMA2_ATTRIB_VALID 1
#define ADMA2_ATTRIB_END   1 << 1
#define ADMA2_ATTRIB_INT   1 << 2

#define ADMA2_ATTRIB_NOP  0 << 4
#define ADMA2_ATTRIB_RSV  1 << 4
#define ADMA2_ATTRIB_TRAN 2 << 4
#define ADMA2_ATTRIB_LINK 3 << 4

typedef struct _sd_adma_list
{
    UCHAR *buffer;
    UCHAR attrib;
    INT32 buffer_len;
    INT32 link;
} SD_NEX_ADMA_LIST;

typedef struct _sd_nex_adma_desc
{
    SD_NEX_ADMA_LIST *adma_list;
    UINT32 num_entries;
} SD_NEX_ADMA_DESC;

typedef struct _sd_nex_adma_table
{
    SD_NEX_ADMA_DESC *desc;
    INT32 num_tables;
} SD_NEX_ADMA_TABLE;

//------------------------+
// Supported UHS Modes.   |
//------------------------+
#define SET_SDR12  0
#define SET_SDR25  1
#define SET_SDR50  2
#define SET_SDR104 3
#define SET_DDR50  4

//---------------------------+
// Supported Bus Width Types.|
//---------------------------+
#define SD_BUS_WIDTH1     0
#define SD_BUS_WIDTH4     1
#define SD_BUS_WIDTH8     2
#define SD_BUS_WIDTH4_DDR 5
#define SD_BUS_WIDTH8_DDR 6

//---------------------------+
// Supported clock Frequency.|
//---------------------------+
#define FULL_SPEED  25000000
#define HIGH_SPEED  50000000
#define CLOCK_20MHZ 20000000
#define CLOCK_52MHZ 52000000

// For UHS_I specifications
#define SDR_50  100000000 // 100 MHz
#define SDR_104 208000000 // 208 MHz
#define DDR_50  50000000  // 50 MHz

#define SDR12  0x32 // default speed
#define SDR25  0x5A // This is common for high speed and SDR25
#define UHS50  0x0B // This is common for SDR50 and DDR50
#define UHS104 0x2B

// User values for SDIO UHS setting
#define SDIO_SDR12  0
#define SDIO_SDR24  1
#define SDIO_SDR50  2
#define SDIO_SDR104 3
#define SDIO_DDR50  4

// User values for SDIO driver strength setting
#define SDIO_TYPE_A 1
#define SDIO_TYPE_B 0
#define SDIO_TYPE_C 2
#define SDIO_TYPE_D 3

// For UHS_II specifications(TBD)
#define DDR_208 208000000 // 208 MHz
#define SDR_150 300000000 // 300 MHz
#define DDR_150 150000000 // 150 MHz
#define DDR_300 300000000 // 300 MHz

#define UHS300 0x3B // Will be created for supporting 300MHz in future specifications.

// For Access Mode
#define AM_DDR12  0x1
#define AM_SDR25  0x2
#define AM_SDR50  0x4
#define AM_SDR104 0x8
#define AM_DDR50  0x10

// For Current Limit
#define CL_200MA 0x1
#define CL_400MA 0x2
#define CL_600MA 0x4
#define CL_800MA 0x8

// For Command System
#define Default    0x1
#define eCommerce  0x2
#define OneTimePgm 0x4
#define Adv_Sec_SD 0x8

// For Drive Strength
#define TYPE_B 0x1
#define TYPE_A 0x2
#define TYPE_C 0x4
#define TYPE_D 0x8

#define SDIO_TYPEB_SUPPORT 0x0
#define SDIO_TYPEA_SUPPORT 0x1
#define SDIO_TYPEC_SUPPORT 0x2
#define SDIO_TYPED_SUPPORT 0x4

#define SDIO_TYPEB_ENABLE 0x00
#define SDIO_TYPEA_ENABLE 0x10
#define SDIO_TYPEC_ENABLE 0x20
#define SDIO_TYPED_ENABLE 0x30

#define SDIO_TYPEB_CHECK 0x00
#define SDIO_TYPEA_CHECK 0x10
#define SDIO_TYPEC_CHECK 0x20
#define SDIO_TYPED_CHECK 0x30

// For HOST_CONTROL2   *** Added for SD 3.0
#define PRESET_VAL_EN  0x8000
#define ASYNC_INT_EN   0x4000
#define TUNED_CLOCK    0X0080
#define EXECUTE_TUNING 0x0040
#define DRIVER_TYPE_D  0x0030
#define DRIVER_TYPE_C  0x0020
#define DRIVER_TYPE_B  0x0000
#define DRIVER_TYPE_A  0x0010
#define SIGNALING_1V8  0x0008
#define UHS_DDR50      0x0004
#define UHS_SDR104     0x0003
#define UHS_SDR50      0x0002
#define UHS_SDR25      0x0001
#define UHS_SDR12      0x0000

//----------------- +
// VOLTAGE SETTINGS |
//------------------+
#define MMC_VDD_17_18 0x00000020
#define MMC_VDD_18_19 0x00000040
#define MMC_VDD_19_20 0x00000080

#define MMC_VDD_20_21 0x00000100
#define MMC_VDD_21_22 0x00000200
#define MMC_VDD_22_23 0x00000400
#define MMC_VDD_23_24 0x00000800

#define MMC_VDD_24_25 0x00001000
#define MMC_VDD_25_26 0x00002000
#define MMC_VDD_26_27 0x00004000
#define MMC_VDD_27_28 0x00008000

#define MMC_VDD_28_29 0x00010000
#define MMC_VDD_29_30 0x00020000
#define MMC_VDD_30_31 0x00040000
#define MMC_VDD_31_32 0x00080000

#define MMC_VDD_32_33 0x00100000
#define MMC_VDD_33_34 0x00200000
#define MMC_VDD_34_35 0x00400000
#define MMC_VDD_35_36 0x00800000

//--------------+
// TYPE OF CARD |
//--------------+
#define INVALID_CARD        -1
#define MMC_MODE_MMC        0
#define MMC_MODE_SD         1
#define MMC_MODE_SDIO       2
#define MMC_MODE_SDHC       3
#define MMC_MODE_SDIO_COMBO 10
#define MMC_MODE_MMCHC      4
#define MMC_MODE_eMMC       5
#define MMC_MODE_eMMCHC     6
#define MMC_MODE_CEATA      7
#define MMC_MODE_MEMPRO     8
#define MMC_MODE_SDXC       9

//---------------------+
// COMMAND OPCODES     |
//---------------------+

// 1 class 1
#define MMC_GO_IDLE_STATE       0 // bc
#define MMC_SEND_OP_COND        1 // bcr [31:0] OCR         R3
#define MMC_ALL_SEND_CID        2 // bcr                    R2
#define MMC_SET_RELATIVE_ADDR   3 // ac  [31:16] RCA        R1
#define MMC_SET_DSR             4 // bc  [31:16] RCA
#define MMC_SLEEP_AWAKE         5 // ac [31:16] RCA	R1B
#define MMC_SELECT_CARD         7 // ac  [31:16] RCA        R1
#define MMC_SEND_IF_COND        8 // bcr  [31:0 ] OCR        R7
#define MMC_READ_EXT_CSD        8
#define MMC_SEND_CSD            9  // ac  [31:16] RCA        R2
#define MMC_SEND_CID            10 // ac  [31:16] RCA        R2
#define MMC_READ_DAT_UNTIL_STOP 11 // adtc[31:0 ] addr       R1  *** For MMC
#define SD_VOLTAGE_SWITCH       11 // ac [31:0]              R1  *** For SDXC
#define MMC_STOP_TRANSMISSION   12 // ac                     R1b
#define MMC_SEND_STATUS         13 // ac   [31:16] RCA       R1
#define MMC_GO_INACTIVE_STATE   15 // ac   [31:16] RCA

/* class 2 */
#define MMC_SET_BLOCKLEN         16 // ac    [31:0] block len  R1
#define MMC_READ_SINGLE_BLOCK    17 // adtc [31:0] data addr   R1
#define MMC_READ_MULTIPLE_BLOCK  18 // adtc [31:0] data addr   R1
#define SDXC_SEND_TUNING_PATTERN 19 // adtc [31:0] data addr   R1 *** For SDXC
#define SDXC_SPEED_CLASS_CONTROL 20 // ac [31:0] data addr     R1b *** For SDXC
#define MMC_SEND_TUNING_BLOCK    21

/* class 3 */
#define MMC_WRITE_DAT_UNTIL_STOP 20 // adtc [31:0] data addr   R1 *** For MMC

/* class 4 */
#define MMC_SET_BLOCK_COUNT    23 // adtc [31:0] data addr   R1 *** For MMC & SDXC
#define SDXC_STOP_TRANSMISSION 23 //

#define MMC_WRITE_BLOCK          24 // adtc [31:0] data addr   R1
#define MMC_WRITE_MULTIPLE_BLOCK 25 // adtc                    R1
#define MMC_PROGRAM_CID          26 // adtc                    R1
#define MMC_PROGRAM_CSD          27 // adtc                    R1

/* class 6 */
#define MMC_SET_WRITE_PROT       28 // ac   [31:0] data addr   R1b
#define MMC_CLR_WRITE_PROT       29 // ac   [31:0] data addr   R1b
#define MMC_SEND_WRITE_PROT      30 // adtc [31:0] wpdata addr R1
#define MMC_SEND_WRITE_PROT_TYPE 31 // adtc [31:0] wpdata addr R1

/* class 5 */
#define MMC_ERASE_GROUP_START 35 // ac   [31:0] data addr   R1
#define MMC_ERASE_GROUP_END   36 // ac   [31:0] data addr   R1
#define MMC_ERASE             38 // ac                      R1b

/* class 9 */
#define MMC_FAST_IO      39 // ac   <Complex>          R4
#define MMC_GO_IRQ_STATE 40 // bcr                     R5

/* class 7 */
#define MMC_LOCK_UNLOCK 42 // adtc                    R1b

#define SELECT_PARTITION 43
#define MANAGE_PARTITION 44
#define QUERY_PARTITION  45
#define MMC_SET_TIME     49
/* class 8 */
#define MMC_APP_CMD 55 // ac   [31:16] RCA        R1
#define MMC_GEN_CMD 56 // adtc [0] RD/WR          R1

// SD commands                           type      argument       response
/* class 8 */
#define SD_SEND_RELATIVE_ADDR 3 // bcr                       R6
#define SD_IO_SEND_OP_COND    5 // bcr [23:0] OCR            R4

#define SD_IO_RW_DIRECT      52 // ac <Complex>              R5

/* Application commands */
#define SD_APP_SET_BUS_WIDTH  6 // ac   [1:0] bus width      R1
#define MMC_SWITCH            6 // ac                        R1b  =>   for setting ext_csd -> high speed and 8 bit bus for MMC-HS cards
#define SD_APP_OP_COND       41 // bcr  [31:0] OCR           R3
#define SD_APP_SEND_SCR      51 // adtc                      R1
#define SD_IO_RW_EXTENDED    53

/* CE ATA Commands */
#if defined(ENABLE_CEATA)
#define CEATA_CMD_REGISTER 60
#define CEATA_CMD_BLOCK    61
#endif

// eMMC 4.51 commands

#define MMC_PROTOCOL_RD 53
#define MMC_PROTOCOL_WR 54

//-----------+
// Data Flags|
//-----------+

#define MMC_READ_DATA  (1 << 9)
#define MMC_WRITE_DATA (1 << 8)
#define CMD_RETRIES    3

//---------------------------------+
// defines for EXT_CSD register in  |
// MMC HIGH SPEED cards             |
//---------------------------------+
#define MMC_EXT_MODE_COMMAND_SET (0 << 24)
#define MMC_EXT_MODE_SET_BIT     (1 << 24)
#define MMC_EXT_MODE_CLEAR_BITS  (2 << 24)
// if writing then cmd set is ignored
#define MMC_EXT_MODE_WRITE_BYTE (3 << 24)

// Added for MMC4.3
#define eMMC_BGA_TYPE 0x01

#define MMC_EXT_REGS_CMD_SET             (191 << 16)
#define MMC_EXT_REGS_POWER_CLASS         (187 << 16)
#define MMC_EXT_REGS_HIGH_SPEED          (185 << 16)
#define MMC_EXT_REGS_BUS_WIDTH           (183 << 16)
#define MMC_EXT_REGS_BOOT_CONFIG         (179 << 16)
#define MMC_EXT_REGS_BOOT_GONFIGPROT     (178 << 16)
#define MMC_EXT_REGS_BOOT_BUSWIDTH       (177 << 16)
#define MMC_EXT_REGS_ERASEGRP_DEFN       (175 << 16)
#define MMC_EXT_REGS_BOOT_WP             (173 << 16)
#define MMC_EXT_REGS_USR_WP              (171 << 16)
#define MMC_EXT_REGS_FW_CONFIG           (169 << 16)
#define MMC_EXT_REGS_SANITIZE_START      165
#define MMC_EXT_REGS_BKOPS_START         164
#define MMC_EXT_REGS_BKOPS_ENABLE        163
#define MMC_EXT_REGS_RST_FUNC            (162 << 16)
#define MMC_EXT_REGS_HPI_MGMT            161
#define MMC_EXT_REGS_PART_ATTRB          (156 << 16)
#define MMC_EXT_REGS_PART_COMPLETE       (155 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_4_2    (154 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_4_1    (153 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_4_0    (152 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_3_2    (151 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_3_1    (150 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_3_0    (149 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_2_2    (148 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_2_1    (147 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_2_0    (146 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_1_2    (145 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_1_1    (144 << 16)
#define MMC_EXT_REGS_GP_SIZE_MULT_1_0    (143 << 16)
#define MMC_EXT_REGS_ENH_SIZE_MULT2      (142 << 16)
#define MMC_EXT_REGS_ENH_SIZE_MULT1      (141 << 16)
#define MMC_EXT_REGS_ENH_SIZE_MULT0      (140 << 16)
#define MMC_EXT_REGS_ENH_START_ADDR      (136 << 16)
#define MMC_EXT_REGS_EXCEPTION_EVTS_CTRL (56 << 16)
#define MMC_EXT_REGS_CONTEXT_CONF        37
#define MMC_EXT_REGS_PWR_OFF_NOTIFY      34
#define MMC_EXT_REGS_CACHE_CTRL          33
#define MMC_EXT_REGS_FLUSH_CACHE         32
#define MMC_EXT_REGS_HS_TIMING           185

#define MMC_EXP_PACKED_EVENT_EN  0x8
#define MMC_EXP_SYSPOOL_EVENT_EN 0x4
#define MMC_EXP_DYN_CAP_EVENT_EN 0x2
#define MMC_EXT_SEC_SANITIZE     (0x1 << 6)

#define MMC_EXT_SET_BUS_WIDTH1     (SD_BUS_WIDTH1 << 8)
#define MMC_EXT_SET_BUS_WIDTH4     (SD_BUS_WIDTH4 << 8)
#define MMC_EXT_SET_BUS_WIDTH8     (SD_BUS_WIDTH8 << 8)
#define MMC_EXT_SET_BUS_WIDTH4_DDR (SD_BUS_WIDTH4_DDR << 8)
#define MMC_EXT_SET_BUS_WIDTH8_DDR (SD_BUS_WIDTH8_DDR << 8)

#define MMC_EXT_SET_HIGH_SPEED (1 << 8)
#define MMC_EXT_SET_CMD_ATA    (1 << 4)
#if defined(ENABLE_CEATA)
#define MMC_EXT_SET_CEATA (8 << 8)
#endif

// Added for MMC4.3

// CARD TYPE FLAGS
#define HS_26MHZ           0x1
#define HS_52MHZ           0x2
#define HS_DDR_52MHZ_18_3V 0x4
#define HS_DDR_52MHZ_12V   0x4
#define HS200_200MHZ_18V   0x10
#define HS200_200MHZ_12V   0x20
#define HS200_TIMING       0x02

// BOOT ACK FLAGS
#define eMMC_BOOT_ACK_EN 0x01
#define eMMC_NO_BOOT_ACK 0x00

// BOOT PARTITION ENABLE FLAGS
#define eMMC_NO_BOOT_EN    0x00
#define eMMC_BOOT_PART1_EN 0x01
#define eMMC_BOOT_PART2_EN 0x02
#define eMMC_BOOT_USER_EN  0x07

// BOOT PARTITION ACCESS FLAGS
#define eMMC_NO_BOOT_ACCESS 0x00
#define eMMC_BOOT1_ACCESS   0x01
#define eMMC_BOOT2_ACCESS   0x02
#define eMMC_RPMB_ACCESS    0x03
#define eMMC_GPP1_ACCESS    0x04
#define eMMC_GPP2_ACCESS    0x05
#define eMMC_GPP3_ACCESS    0x06
#define eMMC_GPP4_ACCESS    0x07

// BOOT BUS WIDTH FLAGS
#define eMMC_RESET_BUSWDITH  0x00
#define eMMC_RETAIN_BUSWDITH 0x01

// BOOT BUS WIDTH MODES
#define SDR_BCKWARD_COMP_BOOT_MOD 0x00
#define SDR_HS_TIMING_BOOT_MODE   0x01
#define DDR_BOOT_MODE             0x02

// BOOT BUS WIDTHS
#define eMMC_BOOT_BUSWIDTH1 0x00
#define eMMC_BOOT_BUSWIDTH4 0x01
#define eMMC_BOOT_BUSWIDTH8 0x02

// Boot Config Protection Flags
#define PERM_BOOT_CONFIG_PROT_EN  0x01
#define PERM_BOOT_CONFIG_PROT_DIS 0x00
#define PWR_BOOT_CONFIG_PROT_EN   0x01
#define PWR_BOOT_CONFIG_PROT_DIS  0x00

// Boot Write Protection Flags
#define BOOT_PWR_WP_DIS  0x01
#define BOOT_PWR_WP_USE  0x00
#define BOOT_PERM_WP_DIS 0x01
#define BOOT_PERM_WP_USE 0x00
#define BOOT_PERM_WP_EN  0x01
#define BOOT_PERM_NO_WP  0x00
#define BOOT_PWR_WP_EN   0x01
#define BOOT_PWR_NO_WP   0x00

// User Write Protection Flags
#define PERM_PSWD_DIS           0x1
#define PERM_PSWD_EN            0x0
#define CD_PERM_WP_DIS          0x1
#define CD_PERM_WP_EN           0x0
#define US_PERM_WP_GRP_DIS      0x1
#define US_PERM_WP_GRP_EN       0x0
#define US_PWR_WP_GRP_DIS       0x1
#define US_PWR_WP_GRP_EN        0x0
#define US_PERM_WP_CMD28GRP_EN  0x1
#define US_PERM_WP_CMD28GRP_DIS 0x0
#define US_PWR_WP_CMD28GRP_EN   0x1
#define US_PWR_WP_CMD28GRP_DIS  0x0

// FW update Flags
#define FW_UPDATE_EN  0x0
#define FW_UPDATE_DIS 0x1

// Reset Function Enable
#define RST_N_TEMP_DIS 0x0
#define RST_N_PERM_EN  0x1
#define RST_N_PERM_DIS 0x2

// Partition Support Flags
#define ENH_ATTRIBUTE_EN  0x1
#define ENH_ATTRIBUTE_DIS 0x0
#define PARTITIONING_EN   0x1
#define PARTITIONING_DIS  0x0

// Partition Atrribute Features
#define SET_ENH_USR            0x1
#define SET_ENH_GPP1           0x2
#define SET_ENH_GPP2           0x4
#define SET_ENH_GPP3           0x8
#define SET_ENH_GPP4           0x10
#define SET_ALL_PART_ATTRB     0x1F
#define SET_ALL_GPP_PART_ATTRB 0x1E

// Partition Completion Flags
#define PARTITION_COMPLETE 0x1

// Ext CSD Read Flags
#define HS_BOOT_MODE_SUPPORT  0x1
#define DDR_BOOT_MODE_SUPPORT (1 << 1)
#define ALT_BOOT_MODE_SUPPORT (1 << 2)

// eMMC44 RPMB Flags

#define AUTH_KEY_PGM_REQ 0x0001
#define WR_CNTR_VAL_REQ  0x0002
#define AUTH_DAT_WR_REQ  0x0003
#define AUTH_DAT_RD_REQ  0x0004
#define RESULT_RD_REQ    0x0005

#define AUTH_KEY_PGM_RESP 0x0100
#define WR_CNTR_VAL_RESP  0x0200
#define AUTH_DAT_WR_RESP  0x0300
#define AUTH_DAT_RD_RESP  0x0400

#define OPR_OK                    0x00
#define WRCNT_EXP_OPR_OK          (0x80)
#define GEN_FAILURE               0x01
#define WRCNT_EXP_GEN_FAILURE     (0x81)
#define AUTH_FAILURE              0x02
#define WRCNT_EXP_AUTH_FAILURE    (0x82)
#define COUNTER_FAILURE           0x03
#define WRCNT_EXP_COUNTER_FAILURE (0x83)
#define ADDR_FAILURE              0x04
#define WRCNT_EXP_ADDR_FAILURE    (0x84)
#define WRITE_FAILURE             0x05
#define WRCNT_EXP_WRITE_FAILURE   (0x85)
#define READ_FAILURE              0x06
#define WRCNT_EXP_READ_FAILURE    (0x86)
#define AUTH_KEY_NOT_PGMMED       0x07

#define RPMB_MAC_KEY_LENGTH        32
#define RPMB_NONCE_LENGTH          16
#define RPMB_DATA_LENGTH           256
#define RPMB_STUFF_LENGTH          196
#define RPMB_COUNTER_LENGTH        4
#define RPMB_ADDR_LENGTH           2
#define RPMB_BLKCNT_LENGTH         2
#define RPMB_RESULT_LENGTH         2
#define RPMB_RSPREQ_LENGTH         2
#define TOTAL_MAC_LEN_PER_FRAME    284
#define MACLEN_EXCL_DATA_PER_FRAME 28

// EMMC ERROR CODES
#define EMMC_INVALID_BOOT_ACK          -26
#define EMMC_INVALID_BOOT_PART         -27
#define EMMC_INVALID_BOOT_ACCESS       -28
#define EMMC_INVALID_BOOT_BUSWIDTH     -29
#define EMMC_INVALID_BOOT_BUSWIDTHFLAG -30
#define EMMC_INVALID_BOOT_BUSMODE      -31
#define EMMC_INVALID_BOOT_CONFIGPROT   -32
#define EMMC_INVALID_BOOT_WPENABLE     -33
#define EMMC_INVALID_USR_WPENABLE      -34
#define EMMC_INVALID_ERASEGRPDEFN      -35
#define EMMC_INVALID_GPP_NO            -36

// Added for eMMC4.51
#define EMMC_CMD38_ERASE        0x00000000
#define EMMC_CMD38_SECURE_ERASE 0x80000000
#define EMMC_CMD38_SECURE_TRIM1 0x80000001
#define EMMC_CMD38_SECURE_TRIM2 0x80008000
#define EMMC_CMD38_TRIM         0x00000001
#define EMMC_CMD38_DISCARD      0x00000003

// eSD physical partition attribute structure.

struct esd_phy_partition
{
    INT32 partition_attr : 8;
    INT32 reserved1[3];
    INT32 reserved2 : 24;
    INT32 data_robust_en : 1;
    INT32 fast_boot : 1;
    INT32 speed_class_en : 1;
    INT32 reserved3 : 29;
    INT32 phy_partition_type : 4;
    INT32 reserved4 : 4;
    INT32 permanent_wp : 1;
    INT32 temp_wp : 1;
    INT32 volatile_wp : 1;
    INT32 sticky_wp : 1;
    INT32 sticky_rp : 1;
    INT32 reserved5 : 3;
    INT32 system_wp : 1;
    INT32 reserved6[10];
    INT32 reserved7 : 15;
    INT32 esd_vendor_spec[8];
    INT32 host_vendor_spec[8];
    INT32 password[32];
};

// eSD device attribute structure
struct esd_device_attribute
{
    INT32 dev_attr_ver : 8;
    INT32 reserved1 : 24;
    INT32 dis_config_cmd : 1;
    INT32 reserved2 : 31;
    INT32 data_robust_support : 1;
    INT32 reserved3 : 1;
    INT32 speed_class_support : 1;
    INT32 reserved4 : 5;
    INT32 config_cmd_support : 1;
    INT32 ex_part_support : 1;
    INT32 imd_resp_support : 1;
    INT32 reserved5 : 21;
    INT32 lock_reset : 2;
    INT32 prod_mode_prof : 1;
    INT32 reserved6 : 29;
};

// eMMC4.4 data frame for RPMB

struct emmc44_data_frame
{
    UINT16 req_resp;
    UINT16 opr_result;
    UINT16 blk_count;
    UINT16 sec_addr;
    INT32 wr_counter;
    UCHAR *Nonce;       // 16 bytes
    UCHAR *data;        // 256 bytes
    UCHAR *key_mac;     // 32 bytes
    UCHAR *stuff_bytes; // 196 bytes
};
//-----------------+
// End of all Defines    |
//-----------------+
struct sd_mmc_cid
{
    INT32 manfid;      // 120-127 bits=8
    UINT16 card_bga;   // 113-112  =16 bits
    UINT16 oemid;      // 104-119 bits=16
    CHAR prod_name[5]; // 64-103 bits=40  5-character ASCII string
                       // 56-63 bits=8   product revision(n.m)
    CHAR prod_name6;
    UCHAR hwrev : 4;  // 60-63 bits=4     n-most significant
    UCHAR fwrev : 4;  // 56-59 bits=4     m-least significant
    INT32 serial;     // 24-55 bits=32
                      // 20-23             reserved
                      // 8-19              manufacture date
    INT32 year;       // 12-19 bits=8-    mfg.year
    UINT32 month : 4; // 8-11 bits=4-     mfg.month
                      // 1-7 bits=7      crc
                      // 0-0 bits=1      not used always 1
};
typedef struct sd_mmc_cid NEX_SD_CID;

struct sd_mmc_csd
{
    UCHAR csd_struct;         // 126-127 bits=2
    UCHAR mmca_vsn;           // 122-125 bits=4
                              // 119-121 bits=3 missing bits--reserved
    UINT32 m1;                // 115-118 bits=4
    UINT32 e1;                // 112-114 bits=3
    INT32 tacc_ns;            // m1 	e1
    UINT16 tacc_clks;         // 104-111 bits=8   (* 100)
    UINT32 m2;                // 99-102 bits=4
    UINT32 e2;                // 96-98  bits=3
    INT32 max_dtr;            // m2 e2
    UINT16 cmdclass;          // 84-95 bits=12
    INT32 read_blkbits;       // 80-83 bits=4
    INT32 read_partial : 1,   // 79-79 bits=1
        write_misalign : 1,   // 78-78 bits=1
        read_misalign : 1;    // 77-77 bits=1
                              // 74-76 bits=3 missing bits  reserved
    UINT32 m3;                // 62-73 bits=12
                              // 50-61 bits=12 reserved
    UINT32 e3;                // 47-49 bits=3
    INT32 capacity;           // m3 e3
                              // 29-46 bits=18 reserved
    INT32 r2w_factor;         // 26-28 bits=3
    INT32 write_blkbits;      // 22-26 bits=4
    UINT32 write_partial : 1; // 21-21 bits=1
};
typedef struct sd_mmc_csd NEX_MMC_CSD;

struct sd_mmc_csd_v4
{
    UCHAR csd_struct : 2;       // 127-126 bits=4
    INT32 specs_ver : 4;        // 125-122 bits=4
    INT32 reserve3 : 2;         // 121-120 bits=2
                                // 121-120 reserved
    UCHAR taac;                 // 119-112 bits=8
    UCHAR nsac;                 // 111-104 bits=8
    UCHAR tran_speed;           // 103-96  bits-8
    UINT16 ccc;                 // 95-84 bits=12
    UCHAR read_bl_len;          // 83-80 bits=4
    INT32 read_partial : 1,     // 79-79 bits=1
        write_misalign : 1,     // 78-78 bits=1
        read_misalign : 1;      // 77-77 bits=1
    INT32 dsr_imp : 1;          // 75-75 bits=1
    INT32 reserve2 : 2;         // 75-74 bits=2
                                // 75-74 bits=3  reserved
    INT32 c_size : 12;          // 73-62 bits=12
    INT32 vdd_r_curr_min : 3;   // 61-59 bits=3
    INT32 vdd_r_curr_max : 3;   // 58-56 bits=3
    INT32 vdd_w_curr_min : 3;   // 55-53 bits=3
    INT32 vdd_w_curr_max : 3;   // 52-50 bits=3
                                // 50-61 bits=12 reserved previously
    UINT32 c_size_mult : 3;     // 49-47 bits=3
    INT32 erase_grp_size : 5;   // 46-42 bits=5
    INT32 erase_grp_mult : 5;   // 41-32 bits=5
    INT32 wp_grp_size : 5;      // 36-32 bits=5
    INT32 wp_grp_enable : 1;    // 31-31 bits=2
    INT32 default_ecc : 2;      // 30-29 bits=2
                                // 29-46 bits=18 reserved prevoiously
    INT32 r2w_factor : 3;       // 28-26 bits=3
    INT32 write_bl_len : 4;     // 25-22 bits=4
    UINT32 write_partial : 1;   // 21-21 bits=1
    INT32 reserve1 : 4;         // 20-17 reserved
    INT32 content_prot_app : 1, // 16-16 bits=1
        file_format_grp : 1,    // 15-15 bits=1
        copy : 1,               // 14-14 bits=1
        perm_write_protect : 1, // 13-13 bits=1
        tmp_write_prot : 1;     // 12-12 bits=1
    INT32 file_format : 2;      // 11-10 bits=2
    INT32 ecc : 2;              // 09-08 bits=2
};

typedef struct sd_mmc_csd_v4 NEX_MMC_CSD_V4;

struct sd_csd_v1
{
    UINT32 csd_struct : 2;         // 126-127 bits=2  csd structure
    UINT32 reserve1 : 6;           // 125-120 bits=6  reserved
    CHAR taac;                     // 112-119 bits=8  data read access time
    CHAR nsac;                     // 104-111 bits=8  data read access time in clk cycles
    CHAR tran_speed;               // 96-103 bits=8  max data transfer rate
    UINT16 ccc : 12;               // 84-95  bits=12 card command classes
    UINT32 read_bl_len : 4;        // 80-83  bits=4  max read data block length
    UINT32 read_bl_partial : 1;    // 79-79  bits=1  partial blocks for read allowed
    UINT32 write_blk_misalign : 1; // 78-78  bits=1  write block misalignment
    UINT32 read_blk_misalign : 1;  // 77-77  bits=1  read block misalignment
    UINT32 dsr_imp : 1;            // 76-76  bits=1  dsr implented
    UINT32 reserve2 : 2;           // 74-75  bits=2  reserved
    INT32 c_size : 12;             // 62-73  bits=12 device size
    UINT32 vdd_r_cur_min : 3;      // 59-61  bits=3  max read current@vdd min
    UINT32 vdd_r_cur_max : 3;      // 56-58  bits=3  max read current@vdd max
    UINT32 vdd_w_cur_min : 3;      // 53-55  bits=3  max write current @vdd min
    UINT32 vdd_w_cur_max : 3;      // 50-52  bits=3  max write current @ vdd max
    UINT32 c_size_mult : 3;        // 47-49  bits=3  device size multiplier
    UINT32 erase_blk_en : 1;       // 46-46  bits=1  erase single block enable
    UINT32 sector_size : 7;        // 39-45  bits=7  erase sector size
    UINT32 wp_grp_size : 7;        // 32-38  bits=7  write protect group size
    UINT32 wp_grp_enable : 1;      // 31-31  bits=1  write protect group enable
    UINT32 reserve3 : 2;           // 29-30  bits=2  reserved
    UINT32 r2w_factor : 3;         // 26-28  bits=3  write speed factor
    UINT32 write_bl_len : 4;       // 22-25  bits=4  max write data block length
    UINT32 write_bl_partial : 1;   // 21-21  bits=1  partial blocks for write allowed
    UINT32 reserve4 : 5;           // 16-20  bits=5  reserved
    UINT32 file_format_grp : 1;    // 15-15  bits=1  file format group
    UINT32 copy : 1;               // 14-14  bits=1  copy flag (OTP)
    UINT32 perm_write_protect : 1; // 13-13  bits=1  permanent write protection
    UINT32 tmp_write_protect : 1;  // 12-12  bits=1  temporary write protection
    UINT32 file_format : 2;        // 10-11  bits=2  file format
    UINT32 reserve5 : 2;           // 08-09  bits=2  reserved
    UINT32 crc : 7;                // 01-07  bits=7  crc code
                                   // 0-00  bits=1  not used, always 1
};
typedef struct sd_csd_v1 NEX_SD_CSD_V1;

// this is sd_csd version 2 structure
struct sd_csd_v2
{
    UINT32 csd_struct : 2;         // 126-127 bits=2 csd structure
    UINT32 reserve1 : 6;           // 125-120 bits=6 reserved
    CHAR taac;                     // 112-119 bits=8 data read access time
    CHAR nsac;                     // 104-111 bits=8 data read access time in clk cycles
    CHAR tran_speed;               // 96-103 bits=8  max data transfer rate
    UINT16 ccc : 12;               // 84-95  bits=12 card command classes
    UINT32 read_bl_len : 4;        // 80-83  bits=4  max read data block length
    UINT32 read_bl_partial : 1;    // 79-79  bits=1  partial blocks for read allowed
    UINT32 write_blk_misalign : 1; // 78-78  bits=1  write block misalignment
    UINT32 read_blk_misalign : 1;  // 77-77  bits=1  read block misalignment
    UINT32 dsr_imp : 1;            // 76-76  bits=1  dsr impleented
    UINT32 reserve2 : 6;           // 70-75  bits=6  reserved
    UINT32 c_size : 22;            // 48-69  bits=22 device size
    UINT32 reserve3 : 1;           // 47-47  bits=1  reserved
    UINT32 erase_blk_en : 1;       // 46-46  bits=1  erase single block enable
    UINT32 sector_size : 7;        // 39-45  bits=7  erase sector size
    UINT32 wp_grp_size : 7;        // 32-38  bits=7  write protect group size
    UINT32 wp_grp_enable : 1;      // 31-31  bits=1  write protect group enable
    UINT32 reserve4 : 2;           // 29-30  bits=2  reserved
    UINT32 r2w_factor : 3;         // 26-28  bits=3  write speed factor
    UINT32 write_bl_len : 4;       // 22-25  bits=4  max write data block length
    UINT32 write_bl_partial : 1;   // 21-21  bits=1  partial blocks for write allowed
    UINT32 reserve5 : 5;           // 16-20  bits=5  reserved
    UINT32 file_format_grp : 1;    // 15-15  bits=1  file format group
    UINT32 copy : 1;               // 14-14  bits=1  copy flag (OTP)
    UINT32 perm_write_protect : 1; // 13-13  bits=1  permanent write protection
    UINT32 tmp_write_protect : 1;  // 12-12  bits=1  temporary write proection
    UINT32 file_format : 2;        // 10-11  bits=2  file format
    UINT32 reserve6 : 2;           // 08-09  bits=2  reserved
    UINT32 crc : 7;                // 01-07  bits=7  crc code
                                   // 0-0    bits=1  not used, always 1
};
typedef struct sd_csd_v2 NEX_SD_CSD_V2;

struct sd_scr
{
    UINT32 scr_structur : 4;        // 63-60 bits=4 SCR Strurcture
    UINT32 sd_spec : 4;             // 59-56 bits=4 SD Memory Card - Spec. Version
    UINT32 dat_stat_aftr_erase : 1; // 55-55 bits=1 data_status_after erases
    UINT32 sd_security : 3;         // 54-52 bits=3 SD Security Support
    UINT32 sd_bus_widths : 4;       // 51-48 bits=4 DAT Bus widths supported
    UINT32 sd_spec3 : 1;            // 47 Added for SD 3.0
    UINT32 ex_security : 4;         // added for SD3.0
    UINT16 reserved : 9;            // 46-34 bits=16 Reserved
    UINT32 cmd_support : 2;         // 33-32 bits=2 command support bits
    INT32 mfg_reserverd : 32;       // 31-0  bits=32 Reserved for manufacturer usage
};
typedef struct sd_scr NEX_SD_SCR;

struct sd_mmc_ext_csd
{
    UCHAR buffer[512];
    UCHAR ext_sec_err;           // 1byte 505
    UCHAR s_cmd_set;             // 1byte 504
    UCHAR hpi_features;          // 1byte 503
    UCHAR bkops_support;         // 1byte 502
    UCHAR max_packed_reads;      // 1byte 501
    UCHAR max_packed_writes;     // 1byte 500
    UCHAR data_tag_support;      // 1byte 499
    UCHAR tag_unit_size;         // 1byte 498
    UCHAR tag_res_size;          // 1byte 497
    UCHAR context_capab;         // 1byte 496
    UCHAR large_unit_size_m1;    // 1byte 495
    UCHAR ext_support;           // 1byte 494
    INT32 cache_size;            // 4byte 252-249
    UCHAR generic_cmd6_time;     // 1 byte 248
    UCHAR pwr_off_long_time;     // 1byte 247
    UCHAR bkops_status;          // 1byte 246
    INT32 correctly_prg_sectors; // 4bytes 245-242
    UCHAR ini_timeout_ap;        // 1byte 241
    UCHAR pwr_cl_ddr_52_360;     // 1byte 239
    UCHAR pwr_cl_ddr_52_195;     // 1byte 238
    UCHAR pwr_cl_200_360;        // 1byte 237
    UCHAR pwr_cl_200_195;        // 1byte 236
    UCHAR min_perf_ddr_w8_52;    // 1byte 235
    UCHAR min_perf_ddr_r8_52;    // 1byte 234
    UCHAR trim_mult;             // 1byte 232
    UCHAR sec_support;           // 1byte 231
    UCHAR sec_erase_mult;        // 1byte 230
    UCHAR sec_trim_mult;         // 1byte 229
    UCHAR boot_info;             // 1byte 228
    UCHAR boot_size_mult;        // 1byte 226
    UCHAR access_size;           // 1byte 225
    UCHAR hc_erase_grp_size;     // 1byte 224
    UCHAR erase_timeout_multi;   // 1byte 223
    UCHAR rel_wr_sec_cnt;        // 1byte 222
    UCHAR hc_wp_grp_size;        // 1byte 221
    UCHAR sleep_cur_vcc;         // 1byte 220
    UCHAR sleep_cur_vccq;        // 1byte 219
    UCHAR slp_awk_timeout;       // 1byte 217
    UINT32 sec_count;            // 4byte 215-212
    UCHAR min_pref_w_8_52;       // 1byte 210
    UCHAR min_pref_r_8_52;       // 1byte 209
    UCHAR min_pref_w_8_26_4_52;  // 1byte 208
    UCHAR min_pref_r_8_26_4_52;  // 1byte 207
    UCHAR min_pref_w_4_26;       // 1byte 206
    UCHAR min_pref_r_4_26;       // 1byte 205
    UCHAR pwr_cl_26_360;         // 1byte 203
    UCHAR pwr_cl_52_360;         // 1byte 202
    UCHAR pwr_cl_26_195;         // 1byte 201
    UCHAR pwr_cl_52_195;         // 1byte 200
    UCHAR part_switch_time;      // 1byte 199
    UCHAR out_of_intr_time;      // 1byte 198
    UCHAR driver_strength;       // 1byte 197
    UCHAR card_type;             // 1byte 196
    UCHAR csd_structure;         // 1byte 194
    UCHAR ext_csd_rev;           // 1byte 192

    // Modes segment

    UCHAR cmd_Set;             // 1byte 191  R/W
    UCHAR cmd_set_rev;         // 1byte 189
    UCHAR power_class;         // 1byte 187
    UCHAR hs_timing;           // 1byte 185
    UCHAR bus_width;           // 1byte 183
    UCHAR erased_mem_cont;     // 1byte 181
    UCHAR boot_config;         // 1byte 179
    UCHAR boot_config_prot;    // 1byte 178
    UCHAR boot_bus_width;      // 1byte 177
    UCHAR erase_grp_defn;      // 1byte 175
    UCHAR boot_wp_status;      // 1byte 174
    UCHAR boot_wp;             // 1byte 173
    UCHAR user_wp;             // 1byte 171
    UCHAR fw_config;           // 1byte 169
    UCHAR rpmb_size_mult;      // 1byte 168
    UCHAR wr_rel_set;          // 1byte 167
    UCHAR wr_rel_param;        // 1byte 166
    UCHAR sanitize_start;      // 1byte 165
    UCHAR bkops_start;         // 1byte 164
    UCHAR bkops_en;            // 1byte 163
    UCHAR rst_n_func;          // 1byte 162
    UCHAR hpi_mgmt;            // 1byte 161
    UCHAR part_support;        // 1byte 160
    INT32 max_enh_size_mult;   // 3bytes 159-157
    UCHAR part_attrb;          // 1byte 156
    UCHAR part_set_complete;   // 1byte 155
    INT32 GP_size_mult0;       // 3byte 143-145
    INT32 GP_size_mult1;       // 3byte 146-148
    INT32 GP_size_mult2;       // 3byte 149-151
    INT32 GP_size_mult3;       // 3byte 152-154
    INT32 enh_size_mult;       // 3byte 142-140
    INT32 enh_start_addr;      // 4byte 139-136
    UCHAR sec_bad_blk_mngt;    // 1byte 134
    UCHAR tcase_support;       // 1byte 132
    UCHAR periodic_wakeup;     // 1byte 131
    UCHAR prg_cid_csd_ddr;     // 1byte 130
    UCHAR vendor_specific[64]; // 64bytes 127-64
    UCHAR native_sector_size;  // 1byte 63
    UCHAR use_native_sector;   // 1byte 62
    UCHAR data_sector_size;    // 1byte 61
    UCHAR ini_timeout_emu;     // 1byte 60
    UCHAR class_6_ctrl;        // 1byte 59
    UCHAR dyncap_needed;       // 1byte 58
    UINT16 expt_evts_ctrl;     // 2bytes 57-56
    UINT16 expt_evts_status;   // 2bytes 55-54
    UINT16 ext_part_attrib;    // 2bytes 53-52
    UCHAR context_conf[15];    // 15bytes 51-37
    UCHAR packed_cmd_status;   // 1byte 36
    UCHAR packed_fail_index;   // 1byte 35
    UCHAR pwr_off_notify;      // 1byte 34
    UCHAR cache_ctrl;          // 1byte 33
    UCHAR flush_cache;         // 1 byte 32
};

typedef struct sd_mmc_ext_csd NEX_MMC_EXT_CSD;

typedef enum _sd_command_type
{
    SD_CMD_DATA = 0,
    SD_CMD_NON_DATA = 1
} SD_COMMAND_TYPE;

typedef struct _sd_command_data
{
    VOID *buffer;
    INT32 length;
    INT32 blocksize;
    INT32 no_of_blocks;
} SD_COMMAND_DATA;

typedef struct _sd_command
{
    INT32 sdhcNum;
    INT32 slotNum;
    INT32 command_opcode;
    INT32 command_arg;
    INT32 response_type;
    INT32 dir_flag;
    SD_COMMAND_TYPE cmd_type;
    SD_COMMAND_DATA *cmd_data;
    VOID (*cmd_complete_handler)(INT32);
} SD_COMMAND;

struct sd_sdio_cis
{
    UCHAR sdio_rev : 4;
    UCHAR cccr_rev : 4;
    UCHAR sd_format_rev : 8;
    UCHAR sdio_bus_width : 2;
    INT32 reserved : 14;
    UCHAR manf_code[2];
    UCHAR manf_info[2];
    INT32 max_block_size;
    INT32 fn_max_block_size;
};
typedef struct sd_sdio_cis NEX_SDIO_CIS;

struct sdio_bus_specific_info
{
    INT32 host_ver;     // added for SD3.0
    INT32 host_max_clk; // added for SD3.0
    INT32 clock;
    INT32 voltage_range;
    INT32 block_length;
    INT32 low_volt;
    INT32 sdr12_support;  // added for SD3.0
    INT32 sdr25_support;  // added for SD3.0
    INT32 sdr50_support;  // added for SD3.0
    INT32 sdr104_support; // added for SD3.0
    INT32 ddr50_support;  // added for SD3.0
    INT32 mA200_support;
    INT32 mA400_support;
    INT32 mA600_support;
    INT32 mA800_support;
    INT32 hs_support; // added for SD3.0
    INT32 card_ver;
    INT32 functionSelect; // used to select a sdio function 1-7
    // INT32 scattergather;  // scattergather = 0 when req->buffer is used and 1 when scatterlist is used
    UCHAR bus_width;
    UCHAR dma_enable_disable; // 0 for disable, 1 for enable
    UCHAR dma_type;
    UCHAR ifcard_present; // 0 for absent, 1 for present
    UCHAR uhs_mode;
};

struct sd_scatterlist
{
    SCATTERLIST *sg; /* I/O scatter list */
    INT32 sg_len;    /* size of scatter list */
};

struct emmc_cmd23_arg
{
    UCHAR rel_wr_req;
    UCHAR packedcmd;
    UCHAR tag_req;
    UCHAR context_id;
    UCHAR forced_prg;
    UINT16 no_of_blks;
};

struct mmc_lock_data
{
    UCHAR erase;       // 1 Forced Erase
    UCHAR lock_unlock; // 1- Lock 0-unlock
    UCHAR clr_pwd;     // 1 -clear password
    UCHAR set_pwd;     // 1 - set password
    UCHAR pwd_len;     // pwd length
    UCHAR pwd[32];     // password
};

struct packedcmd_entry
{
    struct emmc_cmd23_arg cmd23arg; // cmd23 arg of each entry
    INT32 addr;                     // addr to read/write
    UCHAR *data;                    // data to read/write
};

struct packedcmd
{
    INT32 num_entries;                    // number of entries in packed cmd
    UCHAR rw;                             // 1 for packed write and 0 for packed read cmd
    struct packedcmd_entry *packed_entry; // individual entry in packed cmd
};

/*************************************************
Added for MSPRO cards
**************************************************/
//--------------------------------------+
// Supported clock Frequency for MemPro cards   |
//--------------------------------------+

#define SERIAL_MAX_CLOCK    20000000
#define PARALLEL4_MAX_CLOCK 40000000
#define PARALLEL8_MAX_CLOCK 60000000
#define HOST_MAX_CLK        48000000

//--------------------------------+
//  Interface mode bits of MSPRO card.    |
//--------------------------------+
#define SERIAL_ACCESS    0x80
#define PARALLEL4_ACCESS 0x00
#define PARALLEL8_ACCESS 0x40

//-------------------------+
//  Registers of MSPRO card.  |
//-------------------------+
#define RSVD0_REG     0x0
#define INT_REG       0x1
#define STATUS_REG    0x2
#define TYPE_REG      0x4
#define IFMODE_REG    0x5
#define CATEGORY_REG  0x6
#define CLASS_REG     0x7
#define SYS_PARAM_REG 0x10
#define DATA_CNT_REG  0x11
#define DATA_ADDR_REG 0x13
#define TPC_PARAM_REG 0x17
#define CMD_PARAM_REG 0x18

// Memory Stick Pro TPCs

#define MEM_PRO_SET_RW_REG_ADDR  0x87
#define MEM_PRO_READ_LONG_DATA   0x2D
#define MEM_PRO_READ_SHORT_DATA  0x3C
#define MEM_PRO_READ_REG         0x4B
#define MEM_PRO_GET_INT          0x78
#define MEM_PRO_WRITE_LONG_DATA  0xD2
#define MEM_PRO_WRITE_SHORT_DATA 0xC3
#define MEM_PRO_WRITE_REG        0xB4
#define MEM_PRO_SET_CMD          0xE1
#define MEM_PRO_EX_SET_CMD       0x96
#define MEM_PRO_READ_QUAD_DATA   0x5A
#define MEM_PRO_WRITE_QUAD_DATA  0xA5

// Memory PRO Memory access commands
#define MEMACC_READ_DATA  0x20
#define MEMACC_WRITE_DATA 0x21
#define MEMACC_READ_ATRB  0x24
#define MEMACC_STOP       0x25
#define MEMACC_ERASE      0x26
#define MEMACC_GETIBD     0x47
#define MEMACC_SETIBD     0x46
#define MEMACC_FORMAT     0x10
#define MEMACC_SLEEP      0x11

// MSPRO Format options
#define MSPRO_SHORT_FORMAT 1
#define MSPRO_LONG_FORMAT  2
#define MSPRO_QUICK_FORMAT 0
#define MSPRO_FULL_FORMAT  1

// MSPRO sleep status
#define MSPRO_SLEEP_STATUS   0x02
#define MSPRO_INVALID_FORMAT -1
#define LONG_DATA_SIZE       512

#define MSPRO_MAX_RW_SIZE 8

// MemStick PRO STATUS Reigsters (RO)

// Structure for MSPRO status registers
struct MemStkStatus
{
    UCHAR Int;      // INT Register
    UCHAR Status;   // Status Register
    UCHAR Type;     // Type Register
    UCHAR IFMode;   // IFmode Register
    UCHAR Category; // Category Register
    UCHAR Class;    // Classs Register
};

// Structure for MSPRO attribute info
typedef struct _mspro_sys_info
{
    UCHAR memstickclass;
    UINT16 blksize;
    UINT16 totaluserarea;
    UINT16 unitsize;
    UINT16 totalblk;
    UINT16 pagesize;
    UINT16 cntlfunction;
    UCHAR interfacetype;
    UINT32 mempro_id[4];
    UCHAR mem_subclass;

} MSPRO_SYS_INFO;

typedef struct _mspro_model
{
    ULONG v00_0Fh[4];
    ULONG v10_1Fh[4];
    ULONG v20_2Fh[4];

} MSPRO_MODEL;

typedef struct _mspro_mbr
{
    UCHAR boot_partition;
    UCHAR start_head;
    UCHAR start_sector;
    UCHAR start_cylinder;
    UCHAR partition_type;
    UCHAR end_head;
    UCHAR end_sector;
    UCHAR end_cylinder;
    UINT32 start_sectors;
    UINT32 sectors_per_partition;

} MSPRO_MBR;

typedef struct _mspro_pbr_fat
{
    CHAR jmp_ins[3];
    LONG OEM_name_version;
    CHAR no_of_FATS;
    UINT16 no_of_heads;
    CHAR phy_driverno;
    LONG filesystemtype;

} MSPRO_PBR_FAT;

typedef struct _mspro_pbr_fat32
{
    CHAR jmp_ins[3];
    LONG OEM_name_version;
    CHAR no_of_FATS;
    UINT16 no_of_heads;
    CHAR phy_driverno;
    LONG filesystemtype;

} MSPRO_PBR_FAT32;

typedef struct _mspro_specfile
{
    CHAR filename[8];
    CHAR ext[3];
    UCHAR attr;
    UCHAR reserved[10];
    UINT16 time;
    UINT16 date;
    UINT16 cluster;
    UINT32 filesize;

} MSPRO_SPECFILE;

typedef struct _mspro_devinfo
{
    UINT16 cylinders;
    UINT16 heads;
    UINT16 bytes_per_track;
    UINT16 bytes_per_sector;
    UINT16 sectors_per_track;
    UCHAR reserved[6];

} MSPRO_DEVINFO;

// Structure of Device Information Entry
struct mspro_info
{

    MSPRO_SYS_INFO mspro_sys_info;    // System Information
    MSPRO_MODEL mspro_model;          // Model Name
    MSPRO_MBR mspro_mbr;              // MBR Values
    MSPRO_PBR_FAT mspro_pbr_fat;      // PBR Values - (FAT12/FAT16)
    MSPRO_PBR_FAT32 mspro_pbr_fat32;  // PBR Values - (FAT32)
    MSPRO_SPECFILE mspro_specfile[2]; // Special File Values
    MSPRO_DEVINFO mspro_devinfo;      // Idenfify Device Info;
};

typedef struct mspro_info MSPRO_INFO;

/*
 *    All extern declaration here of all functions which will be exported to
 *     the user driver programmer all the function defination are given in
 *     PWD/nex_api.c.
 */

/*******************************************************************************
Name           	: SendData

Argument    	: VOID *buffer - This is the pointer to buffer for writing the data to FIFO/DMA buffer.
                  UINT32 length - It is the length of the buffer in bytes to write to FIFO/DMA buffer.
                  UINT32 address - It is the address of the location where data buffer will be written.
              INT32 sdhcNum - SD Host Controller Number.
                  INT32 slotNum - This is the slot number on which SendData is to be operated on.

Returns        	: Succes/Failure.
                  Success : SD_SUCCESS.
                  Failure : Negative Error number.

Description     : The SendData(...) function will be a Nonblockable function to send buffer to device and is used to
                  write the buffer to the device in conjunction with ReadRegister & WriteRegister bus driver API.
                  This function internally implements the CMD53 multi block write command.
NOTE            : The buffer allocation and deallocation will be responsibility of the caller function.
********************************************************************************/
extern INT32 SendData(VOID *Buffer, UINT32 Length, UINT32 address, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************************
Name             : GetData

Argument         : VOID *buffer - This is the pointer to buffer for reading the data to FIFO/DMA buffer.
                   UINT32 length - It is the length of the buffer in bytes to read from FIFO/DMA buffer.
           UINT32 address - address of the target device to read.
           INT32 sdhcNum - SD Host Controller Number.
                   INT32 slotNum - This is the slot number on which the GetData is to be operated on.

Returns        	 : Succes/Failure.
                   Success : SD_SUCCESS.
                   Failure : Negative Error number.

Description      : The GetData(...) function will be a Nonblockable function for buffer reading command to device and
                   is used to read the   buffer in conjunction with ReadRegister & WriteRegister bus driver API.
                   This function internally implements the CMD53 multi block read command.

NOTE             : The buffer allocation and deallocation will be responsibility of the caller function.
********************************************************************************/
extern INT32 GetData(VOID *Buffer, UINT32 Length, UINT32 address, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           :  ReadPartialBlock

 Argument       :  UINT32 address - This is address to read from target device using CMD17.
                   UCHAR  *buffer - The content of this pointer will hold the data to read from the address,
                                  memory allocation has to be done by caller function.
                   UINT32 length - no. of bytes to read from the device.
           INT32 sdhcNum - SD Host Controller Number.
                   INT32 SlotNum - This is the slot number of the card to be read.

 Returns        :  Succes/Failure.
                   Success : SD_SUCCESS.
                   Failure   : Negative Error number.

 Description   : The ReadPartialBlock(...) function will be used to read a partial block (<512 bytes) from the target
 device. This function can be used only if the card supports partial block read. The address offset is provided by the
 upper layer. The function will implement the CMD17 command and send to the device to read the bytes from the given
 address.
 ******************************************************************/
extern INT32 ReadPartialBlock(UINT32 address, UCHAR *buffer, UINT32 length, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : WritePartialBlock

 Argument       : UINT32 address - This is address to write to the target device using CMD24.
                  UCHAR  buffer - This is the buffer to hold data to be written on RegisterAddress.
                  UINT32 length - no. of bytes to read from the device.
          INT32 sdhcNum - SD Host Controller Number.
                  INT32 SlotNum - This is the slot number of the card to be written.

 Returns        : Succes/Failure.
                      Success : SD_SUCCESS.
                      Failure   : Negative Error number.

 Description    : The WritePartialBlock(...) function will be used to write a partial block (<512 bytes) on the target
device. This function can be used only if the card supports partial block write. The address offset is provided by the
upper layer. The function will implement the CMD24 command and send to the device to write the byte on given address.
********************************************************************/
extern INT32 WritePartialBlock(UINT32 address, UCHAR buffer, UINT32 length, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : ReadHWRegister

 Argument       : ULONG registerAddress - This is address to read from host controller register.
                  UINT16 *value - The content of this pointer will hold the byte read from RegisterAddress,
                                 memory allocation has to be done by caller function.
          INT32 sdhcNum - SD Host Controller Number.
                  INT32 slotNum - This is the slot number whose register are to be read.

 Returns        : Succes/Failure.
                       Success : SD_SUCCESS.
                       Failure   : Negative Error number.

 Description   : The ReadHWRegister(...) function will be used to read a single word from the host controller
                 register. The register actual address offset is provided by the upper layer. The function will
                 flat read the word from host controller register address.
********************************************************************/
extern INT32 ReadHWRegister(ULONG RegisterAddress, UINT16 *Value, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name        : ReadHWRegister_32

 Argument    : ULONG registerAddress - This is address to read from host controller register.
               UINT32 *value - The content of this pointer will hold the byte read from RegisterAddress,
                              memory allocation has to be done by caller function.
           INT32 sdhcNum - SD Host Controller Number.
               INT32 slotNum -This is the slot number whose register are to be read.


 Returns     : Succes/Failure.
               Success : 0.
               Failure : Negative Error number.

 Description : The ReadHWRegister_32(...) function will be used to read a single word from the host controller
               register. The register actual address offset is provided by the upper layer. The function will
               flat read the word from host controller register address.
********************************************************************/
extern INT32 ReadHWRegister_32(ULONG registerAddress, UINT32 *value, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : WriteHWRegister

 Argument       : ULONG registerAddress - This is address to write to the host controller register.
                  UINT16 value - This is the word value to be written on RegisterAddress.
          INT32 sdhcNum - SD Host Controller Number
                  INT32 slotNum - This is the slot number whose register are to be written.

 Returns        : Succes/Failure.
                       Success : SD_SUCCESS.
                       Failure : Negative Error number.

 Description    : The WriteHWRegister(...) function will be used to write a single word on the host controller
                  register. The register actual address offset is provided by the upper layer. The function will
                  flat write the word on the host controller register address.
********************************************************************/
extern INT32 WriteHWRegister(ULONG RegisterAddress, UINT16 Value, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name        : WriteHWRegister_32

 Argument    : ULONG registerAddress - This is address to write to the host controller register.
               UINT32 value - This is the word value to be written on RegisterAddress.
           INT32 sdhcNum - SD Host Controller Number.
               INT32 slotNum - This is the slot number whose register are to be written.

 Returns     : Succes/Failure.
               Success : 0.
               Failure : Negative Error number.

 Description : The WriteHWRegister_32(...) function will be used to write a single word on the host controller
               register. The register actual address offset is provided by the upper layer. The function will
               flat write the word on the host controller register address.
********************************************************************/
extern INT32 WriteHWRegister_32(ULONG registerAddress, UINT32 value, INT32 sdhcNum, INT32 slotNum);

typedef enum _sd_type_of_intr
{
    SD_TARGET_INTR = 2,
    SD_CARD_REMOVED = 3,
    SD_CARD_INSERTED = 4,
    SD_DATA_COMPLETE = 5,
    SD_CMD_COMPLETE = 6,
    SD_MAX_TYPE = SD_CMD_COMPLETE
} SD_TYPE_OF_INTR;

typedef struct _sd_call_back_info
{
    SD_TYPE_OF_INTR intr_type; /* Type of Interrupt as in the above enum */
    INT32 status;
    INT32 slot; /* Slot number for which the call back has been called */
    INT32 sdhcNum;
    UINT32 cmd_resp[4];
    UCHAR card_status;
} SD_CALL_BACK_INFO;

typedef VOID (*functionPointer)(SD_CALL_BACK_INFO *callBackInfo);

/*******************************************************************
 Name           : RegisterISRCallBack

 Argument       : VOID (*functionPointer)(VOID *) - This is a function pointer to the interrupt handler which gets
executed on interrupt from the target device. INT32 sdhcNum - SD Host Controller Number INT32 slotNum - the slot number

 Returns        : Succes/Failure.
                       Success : SD_SUCCESS.
                       Failure   : Negative Error number.

 Description    : This function registers handler to the bus driver for reporting interrupts to the upper
                  layer. Now the Register handler function is to be called when the interrupts from target
                  device arrived in the bus driver.
********************************************************************/
extern INT32 RegisterISRCallBack(functionPointer fptr, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : IsCardInitialized

 Argument       : INT32 sdhcNum - SD Host Controller Number
          INT32 slotNum - This is the slot number for which card detection is to be checked.

 Returns        : SD_TRUE - If card is present.
                  SD_FALSE - If card is not present in this slot.

 Description    : This function will check whether the card is present on the current host initialized.
********************************************************************/
extern INT32 IsCardInitialized(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : GetBusSpecificInformation

 Argument       : struct sdio_bus_specific_info *busInfo - Will hold all the bus specific information for the specific
slot. INT32 sdhcNum - SD Host Controller Number. INT32 slotNum - The slot number for which bus specific information is
to be retrived.

 Returns        : VOID.

 Description    : The function will retrive bus specific  information for the current slot whose host is initialised.
********************************************************************/
extern VOID GetBusSpecificInformation(struct sdio_bus_specific_info *BusInfo, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : SetBusSpecificInformation

 Argument       : struct sdio_bus_specific_info *busInfo - Holds all the Bus specific information for the specific slot
to be set. INT32 sdhcNum - SD Host Controller Number. INT32 slotNum - The slot number for which bus specific information
is to be retrived.

 Returns        : Success/Failure.
                       Success : SD_SUCCESS.
                       Failure : Negative Error number.

 Description    : The function will set bus specific  information for the current slot whose host is initialised.
********************************************************************/
extern INT32 SetBusSpecificInformation(struct sdio_bus_specific_info *BusInfo, INT32 sdhcNum, INT32 slotNum);

/* ___ NOTE : The CIDStructure is still not initialised in des_docv1.5 ___*/
/*******************************************************************
 Name           : GetCardSpecificInfo_CID

 Argument       : struct NEX_SD_CID *cidStructure - Holds all the card specific information CID for the slot number
provided. INT32 sdhcNum - SD Host Controller Number. INT32 slotNum - Gives the Slot number for which CID structure is to
be found out.

 Returns        : VOID

 Description    : The following function will get card specific information CID in the structure passed to it. The call
to this API is valid only if card is initialised properly.
********************************************************************/
extern VOID GetCardSpecificInfo_CID(NEX_SD_CID *cidStructure, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : GetCardSpecificInfo_CSD

 Argument       : struct NEX_SD_CSD  *csdStructure - Holds all the card specific information CSD for the slot number
provided. INT32 sdhcNum - SD Host Controller Number. INT32 slotNum - Gives the Slot number for which CSD structure is to
be found out.

 Returns        : VOID

 Description    : The following function will get card specific information CSD in the structure passed to it.
                  The call to this API is valid only if card is initialised properly.
*******************************************************************/
extern VOID GetCardSpecificInfo_CSD(VOID *csdStructure, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : GetCardSpecificInfo_UpdatedCSD

 Argument       : struct NEX_SD_CSD  *csdStruct - Holds all the card specific information CSD for the slot number
provided. INT32 sdhcNum - SD Host Controller Number. INT32 slotNum - Gives the Slot number for which CSD structure is to
be found out.

 Returns        : VOID

 Description    : The following function will get card specific information CSD in the structure passed to it.
                       The call to this API is valid only if card is initialised properly.
*******************************************************************/
extern VOID GetCardSpecificInfo_UpdatedCSD(VOID *csdStruct, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : GetCardSpecificInfo_ExtCSD

 Argument       : struct NEX_MMC_EXT_CSD  *extcsdStructure - Holds all the Extended card specific information CSD for
the slot number provided. INT32 sdhcNum - SD Host Controller Number. INT32 slotNum - Gives the Slot number for which CSD
structure is to be found out.

 Returns        : VOID

 Description    : The following function will get card specific information CSD in the structure passed to it.
                  The call to this API is valid only if card is initialised properly.
*******************************************************************/
extern VOID GetCardSpecificInfo_ExtCSD(VOID *extcsdStructure, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : GetCardSpecificInfo_CIS

 Argument       : struct NEX_SDIO_CIS *cisStructure - Holds all the card specific information for the slot number
provided. INT32 sdhcNum - SD Host Controller Number. INT32 slotNum - Gives the Slot number for which CIS structure is to
be found out.

 Returns        : VOID

 Description    : The following function will get card specific information CIS in the structure passed to it.
                  The call to this API is valid only if card is initialised properly.
*******************************************************************/
extern VOID GetCardSpecificInfo_CIS(NEX_SDIO_CIS *CISStructure, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : SendCommand

 Argument       : SD_COMMAND *cmd - Holds all the command specific information.

 Returns        : int
                       Success : 0
                       Failure : -

 Description   : The following function will put the command from the application driver in the structure passed to it.
                 The call to this API is valid only if card is initialised properly.
*******************************************************************/
INT32 SendCommand(SD_COMMAND *cmd);

/*******************************************************************
 Name           : EnableFunctionInterrupt

 Argument       : INT32 sdhcNum - SD Host Controller Number.
          INT32 SlotNum - Gives the Slot number for which

 Returns        : VOID

 Description    : Following function is used for enabling function function interrupts.
*******************************************************************/
VOID EnableFunctionInterrupt(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : CardType

 Argument       : INT32 sdhcNum - SD Host Controller Number.
          INT32 slotNum - Gives the Slot number.

 Returns        : Succes/Failure.
                       Success : 0,1,2,3 as card types
                       Failure   : -1 is returned

 Description   : Following function will retuen the type of card that is currently present in slot.
*******************************************************************/
INT32 CardType(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************************
Name          : ReadSDIORegister

Argument      : UINT32 function - This is the function number for register address in SDIO card to read.
                UINT32 registerAddress -This is address to read from target device using CMD52.
                UCHAR *value - The content of this pointer will hold the byte read from RegisterAddress;
                               memory allocation has to be done by caller function.
                INT32 sdhcNum - SD Host Controller Number.
                INT32 slotNum - This is the slot number whose register are to be read.

Returns       : Succes/Failure.
                Success : SD_SUCCESS.
                Failure : Negative Error number.

Description   : The ReadSDIORegister(...) function will be used to read a single byte from the target device
                register. Function number will decide which SDIO function to use and the  register actual
                address offset is provided by the upper layer. The function will implement the CMD52
                command and send to the device to read the byte from register address.
********************************************************************************/
INT32 ReadSDIORegister(UINT32 function, UINT32 registerAddress, UCHAR *value, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************************
Name           : WriteSDIORegister

Argument       : UINT32 function - This is the function number for register address in SDIO card to read.
                 UINT32 registerAddress - This is address to write to the target device using CMD52.
                 UCHAR value - This is the byte value to be written on RegisterAddress.
         INT32 sdhcNum - SD Host Controller Number.
                 INT32 slotNum - This is the slot number whose register are to be written.

Returns        : Succes/Failure.
                 Success : SD_SUCCESS.
                 Failure : Negative Error number.

Description    : The WriteSDIORegister(...) function will be used to write a single byte on the target device
register.function number will decide which SDIO function to use and the register actual address offset is provided by
the upper layer. The function will implement the CMD52 command and send to the device to write the byte on register
address.
********************************************************************************/
INT32 WriteSDIORegister(UINT32 function, UINT32 registerAddress, UCHAR value, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************************
Name           	: SetSDIOFullSpeed()

Argument     	: sdhcNum : SD Host Controller Number
          slotNum : slot number of the card


Returns        	: Succes/Failure.
                  Success : SD_SUCCESS.
                  Failure : Negative Error number.

Description   	: The SetSDIOFullSpeed function will be used to enable the full speed mode for SDIO cards

********************************************************************************/

INT32 SetSDIOFullSpeed(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************************
Name           	: SetSDIOHighSpeed()

Argument     	: sdhcNum : SD Host Controller Number
          slotNum : slot number of the card


Returns        	: Succes/Failure.
                  Success : SD_SUCCESS.
                  Failure : Negative Error number.

Description   	: The SetSDIOHighSpeed function will be used to enable the high speed mode for SDIO cards,
                  if the card supports high speed.
********************************************************************************/
INT32 SetSDIOHighSpeed(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************************
Name           	: SetMMCSDHighSpeed()

Argument      	: sdhcNum : SD Host Controller Number
          slotNum : slot number of the card

Returns        	: Succes/Failure.
                  Success : 1.
                  Failure : 0 or Negative Error number.

Description	: The SetMMCSDHighSpeed function will be used to enable the high speed mode for MMC or SD cards,
                  currently present in the slot if only the card supports high speed.
*******************************************************************************/
INT32 SetMMCSDHighSpeed(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************************
 Name           : SetMMCSDFullSpeed()

 Argument       : sdhcNum : SD Host Controller Number
          slotNum : slot number of the card

 Returns        : Succes/Failure.
                  Success : 1.
                  Failure : 0 or Negative Error number.

 Description    : The SetMMCSDFullSpeed function will be used to enable the full speed mode for MMC or SD cards,
                  currently present in the slot if only the card supports full speed.
*******************************************************************************/
INT32 SetMMCSDFullSpeed(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************************
Name        : GetDataSDIOIncrAdrs

Argument    : VOID *buffer - This is the pointer to buffer for reading the data to FIFO/DMA buffer.
              UINT32 length - It is the length of the buffer in bytes to read from FIFO/DMA buffer.
              UINT32 address - address to start reading the data from.
          INT32 sdhcNum - SD Host Controller Number.
              INT32 slotNum - This is the slot number on which SendData is to be operated on.

Returns     : Succes/Failure.
              Success : 0.
              Failure : Negative Error number.

Description : The GetDataSDIOIncrAdrs(...) function will be a Nonblockable function for buffer reading
              command to SDIO device and is used to read the buffer at specified address. This function
              internally implements the CMD53 and uses the Incrementing address for cmd 53 operation.

NOTE        : The buffer allocation and deallocation will be responsibility of the caller function.
********************************************************************************/
INT32 GetDataSDIOIncrAdrs(VOID *buffer, UINT32 length, UINT32 address, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************************
Name        : SendDataSDIOIncrAdrs

Argument    : VOID *buffer - This is the pointer to buffer for writing the data to FIFO/DMA buffer.
              UINT32 length - It is the length of the buffer in bytes to write to FIFO/DMA buffer.
              UINT32 address - It is the address of the location where data buffer will be written.
          INT32 sdhcNum - SD Host Controller Number.
              INT32 slotNum - This is the slot number on which SendData is to be operated on.

Returns     : Succes/Failure.
              Success : 0.
              Failure : Negative Error number.

Description : The SendDataSDIO(...) function will be a Nonblockable function to send buffer to
              SDIO device and is used to write the buffer to    the SDIO device. This Api internally
              uses the Cmd53 block/byte modes to  transfer data using incrementing addresses in cmd 53.

NOTE:       : The buffer allocation and deallocation will be responsibility of the caller function.
********************************************************************************/
INT32 SendDataSDIOIncrAdrs(VOID *buffer, UINT32 length, UINT32 address, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name          	:  MemproReadWriteAddress

Argument        :  UCHAR rwtype - It will be either read or write.
           UINT32 address - Address to be read/write.
                   UCHAR *value - Value will be updated at this address.
                   UINT32 size - No of bytes to read/write.
           INT32 sdhcNum - SD Host Controller Number.
           INT32 slotNum - the slot number.

Return vlaue    : INT - SUCCESS The Data read/write is valid
                  FAILURE The Data read/write is not valid

 Description	: The EXPORTED API is used to read/write some address from/into the MemStickPro card.
 ******************************************************************/
#if defined(ENABLE_MSPRO)
INT32 MemproReadWriteAddress(UCHAR rwtype, UINT32 address, UCHAR *value, UINT32 size, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name		:  GetCardAttribute

Argument 	:  MSPRO_INFO  *dinfoStructure - The structure to store the attribute values.
           INT32 sdhcNum - SD Host Controller Number.
                   INT32 slotNum - The slot number.

Return vlaue	:  None

Description	: The EXPORTED API is used to read the attribute info of the MemStickPro card.
 ******************************************************************/
VOID GetCardAttribute(MSPRO_INFO *dinfoStructure, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name            :  MSPROFormat

Argument	:  UCHAR format_type - It will be either short or long format
                   UINT32 size - Format w.r.t No of bytes
           UCHAR format_mode - Given a mode number, it returns the appopriate
                       ten-character mode string as it would have been generated by ls -l.
           INT32 sdhcNum - SD Host Controller Number
                   INT slotNum - the slot number

Return vlaue 	: INT - SUCCESS The Data read/write is valid
                        FAILURE The Data read/write is not valid

Description	: This API is used to format the MSPRO card.
 ******************************************************************/
INT32 MSPROFormat(UCHAR format_type, UINT32 size, UCHAR format_mode, INT32 sdhcNum, INT32 slotNum);
INT32 MSPROSetIBD(VOID *buffer, INT32 length, UINT32 entry_no, INT32 sdhcNum, INT32 slotNum);

INT32 MSPROGetIBD(VOID *buffer, INT32 length, UINT32 entry_no, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name		:  MSPROErase

Argument	:  INT32 length - No.of blocks to be erased
                   UINT32 address - The address from which ERASE starts
           INT32 sdhcNum - SD Host Controller Number
                   INT slotNum -  the slot number

Return vlaue	: INT - SUCCESS The Data read/write is valid
                        FAILURE The Data read/write is not valid

Description	: This API is used to format the MSPRO card.
 ******************************************************************/
INT32 MSPROErase(INT32 length, UINT32 address, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name 		:  MSPROSleep

Argument	:  INT32 sdhcNum - SD Host Controller Number
                   INT slotNum -  the slot number

Return vlaue 	:  INT - SUCCESS The Data read/write is valid
                        FAILURE The Data read/write is not valid

Description 	:  This API is used to format the MSPRO card.
 ******************************************************************/
INT32 MSPROSleep(INT32 sdhcNum, INT32 slotNum);

#endif

/*******************************************************************
 Name         :  eMMCSetBootPartition

 Agrument     :  boot_ack		- The boot acknowledgement enable for the card
         boot_part		- The boot partition to be enabled for the next boot read
         boot_access   		-  The boot partition to be accessed
         sdhcNum 		- SD Host Controller Number
         slotNum		-  Slot number

 Returns      :	 Succes/Failure.
                 Success : 0.
                 Failure : Negative Error number.

 Description  : This API is used to set the boot partition enable and boot partition access fields in the extended CSD
 register. The boot partition enable indicates the boot operation need to be done at the next boot time and the boot
 partition access indicates the read/write on boot area. To access user area for normal operation, the boot access
 fields are set to 000b.
 ******************************************************************/
INT32 eMMCSetBootPartition(UCHAR boot_ack, UCHAR boot_part, UCHAR boot_access, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name         :    eMMCSetBootBusWidth

Argument    	:  boot_mode - The boot bus width modes.
           boot_ buswidth - The bus width for the boot operation.
           boot_buswidth_flag - The bus width flags.
           sdhcNum - SD Host Controller Number.
           slotNum - Slot number.

 Returns      :	   Succes/Failure.
                   Success : 0.
                   Failure : Negative Error number.
 Description  :    This API is used to set the boot bus width. The possible bus widths for boot operation are 1, 4
and 8. The bus width can be reset or retained for the next boot operation.
 ******************************************************************/
INT32 eMMCSetBootBusWidth(UCHAR boot_mode, UCHAR boot_buswidth, UCHAR boot_buswidth_flag, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name            :  eMMCSetBootConfigProt

Argument        :  UCHAR  power_config_prot - To Disable the change of boot configuration register until next power
cycle or next H/W reset operation. UCHAR  perm_config_prot  - To Disable the change of boot configuration register
permanently. INT32 sdhcNum - SD Host Controller Number. INT32 slotNum -  Slot number.

Returns         :  Status

Description	:  This API is used to set the boot config protection
******************************************************************/
INT32 eMMCSetBootConfigProt(UCHAR power_config_prot, UCHAR perm_config_prot, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name 		: eMMCSetBootWPEnable

Argument	: UCHAR  boot_pwr_en  - To enable power on WP to boot area
                  UCHAR  boot_perm_en - To enable permanent WP to boot area
                  UCHAR  boot_pwr_dis  - To disable power on WP to boot area
                  UCHAR  boot_perm_dis - To disable permanent WP to boot area
          INT32 sdhcNum - SD Host Controller Number
                  INT32 slotNum -  Slot number

Returns         : Status

Description	: This API is used to WriteProtect the boot area
 ******************************************************************/
INT32 eMMCSetBootWPEnable(UCHAR boot_pwr_en, UCHAR boot_perm_en, UCHAR boot_pwr_dis, UCHAR boot_perm_dis, INT32 sdhcNum,
                          INT32 slotNum);

/*******************************************************************
Name            : eMMCSetUserWPEnable

Argument 	: usr_pwr_en - To  enable/disable power on WP to user area
                  usr_perm_en - To  enable/disable permanent WP to user area
                  usr_pwr_dis  - To  enable/disable power on WP to user area
                  usr_perm_dis - To  enable/disable permanent WP to user area
                  CD_perm_dis - To enable/disable the use of PERM_WP in CSD[13]
                  perm_pswd_dis - To enable/disable pswd setting features
          INT32 sdhcNum - SD Host Controller Number
                  INT32 slotNum -  Slot number

Returns         : Status

Description 	: This API is used to WriteProtect the user area
 ******************************************************************/
INT32 eMMCSetUserWPEnable(UCHAR usr_pwr_en, UCHAR usr_perm_en, UCHAR usr_pwr_dis, UCHAR usr_perm_dis, UCHAR CD_perm_dis,
                          UCHAR perm_pswd_dis, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name            : eMMCSetEraseGrpDefn

Argument    	: erase_grp_en - To  enable/disable erase grp definition
          sdhcNum - SD Host Controller Number
                  slotNum -  Slot number

Returns         : Status

Description	: This API is used to  enable/disable erase group definition
 ******************************************************************/
INT32 eMMCSetEraseGrpDefn(UCHAR erase_grp_en, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name         	: eMMCFinishPartComplete

Argument    	: part_complete - To  enable partition complete
          sdhcNum - SD Host Controller Number
                  slotNum -  Slot number

Returns 	: Status

Description 	: This API is used to configure partition completion
 ******************************************************************/
INT32 eMMCFinishPartComplete(UCHAR part_complete, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name      	: eMMCConfigureEUDA

Argument  	: enh_area_size - Size of EUDA
                  enh_start_address - Start address for EUDA
          sdhcNum - SD Host Controller Number
                  slotNum -  Slot number

Returns   	: Status

Description  	: This API is used to configure 'Enhanced User Data Area'
 ******************************************************************/
INT32 eMMCConfigureEUDA(INT32 enh_area_size, INT32 enh_start_address, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name 		: eMMCConfigureGPP

Argument	: gpp_no - General Purpose Partition number
                  GP_SIZE_MULT_X_0 - Factor0 for GPP
                  GP_SIZE_MULT_X_1- Factor1 for GPP
                  GP_SIZE_MULT_X_2- Factor2 for GPP
          sdhcNum - SD Host Controller Number
                  slotNum -  Slot number

Returns   	: Status

Description	: This API is used to configure GPP('General Purpose Partition' number)
******************************************************************/
INT32 eMMCConfigureGPP(UINT32 gpp_no, UCHAR GP_SIZE_MULT_X_0, UCHAR GP_SIZE_MULT_X_1, UCHAR GP_SIZE_MULT_X_2,
                       INT32 sdhcNum, INT32 slotNum);


/*******************************************************************
Name 		: eMMCSetWriteProt

Argument	: wp_address - The WriteProtect group address
          sdhcNum - SD Host Controller Number
                  slotNum -  Slot number

Returns 	: Status

Description	: This API is used to set WriteProtect in user area
 ******************************************************************/
INT32 eMMCSetWriteProt(INT32 wp_address, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name		: eMMCClrWriteProt

Argument 	: wp_address - The WriteProtect group address
          sdhcNum - SD Host Controller Number
                  slotNum -  Slot number

Returns 	: Status

Description     : This API is used to clear WriteProtect in user area
******************************************************************/
INT32 eMMCClrWriteProt(INT32 wp_address, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name 		: eMMCSendWriteProt

Argument 	: Buffer - the buffer to collect WriteProtect bits
                  Length - Number of WriteProtect bytes
                  address - WriteProtect group address
          sdhcNum - SD Host Controller Number
                  slotNum -  Slot number

Returns 	: Status

Description     : This API is used to send status of WriteProtect bits
******************************************************************/
INT32 eMMCSendWriteProt(VOID *Buffer, UINT32 Length, INT32 address, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name		: eMMCSendWriteProtType

Argument	: Buffer - the buffer to collect WriteProtect bits
                  Length - Number of WriteProtect bytes
                  address - WriteProtect group address
          sdhcNum - SD Host Controller Number
                  slotNum -  Slot number

Returns   	: Status

Description     : This API is used to send type of WriteProtect bits
 ******************************************************************/
INT32 eMMCSendWriteProtType(VOID *Buffer, UINT32 Length, INT32 address, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name		: MMCSetBlockCount

Argument	: arg - structure to emmc_cmd23_arg
          sdhcNum - SD Host Controller Number
                  slotNum -  Slot number

Returns  	: Status

Description     : This API is used to set number of blocks for MMC.
 ******************************************************************/
INT32 MMCSetBlockCount(struct emmc_cmd23_arg arg, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : eSDSelectPartition

 Argument       :	INT32 partition_id        -  Patition ID number to be selected.
                INT32 enable_imd_resp     -  Enable Immediate Response flag.
            INT32 sdhcNum 		  -  SD Host Controller Number.
                INT32 slotnumber          -  The Slot Number.

 Returns        : int
                     0              - Success
                    Error Code      -  Failure.

 Description   : This API is used to select a new active physical partition.
         If the requested physical partition does not exist, an OUT_OF_RANGE error is returned.
         This command can only be used after the card has been initialized.
*******************************************************************/
INT32 eSDSelectPartition(INT32 partition_id, INT32 enable_imd_resp, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : eSDManagePartition

 Parameters	: opcode          -  Opcode of the subcommand to be sent.
          buffer          -  Pointer to a buffer to be sent.
              length          -  Length of the data to be sent.
          sdhcNum 	  -  SD Host Controller Number.
              slotnumber      -  Slot number.

 Return Value   : Succes/Failure.
            0            - Success
            Error Code   -  Failure.

 Description   : This API is used to manage the physical partition.
         Based on the opcode, one of the management sub-command will be executed.
         This command can only be used after the card has been initialized.
*******************************************************************/
INT32 eSDManagePartition(UINT32 opcode, VOID *buffer, UINT32 length, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name             : eSDQueryPartition

 Argument         : UINT32 opcode - the sub command to be sent.
            INT32 partition_id - Partition ID to be queried. This field is valid only for the opcode
                             eSD_QUERY_PARTITION_ATTRIBUTES(0XB1).

            VOID *buffer - This is the pointer to buffer for reading the data to FIFO/DMA buffer.
                    UINT32 length - It is the length of the buffer in bytes to read from FIFO/DMA buffer.
            INT32 sdhcNum - SD Host Controller Number.
                    INT32 slotnumber - This is the slot number on which the QueryPartition is to be operated on.

 Returns          : Succes/Failure.
                       Success : SD_SUCCESS.
                       Failure   : Negative Error number.

 Description      : The QueryPartition(...) function will be a Nonblockable function for reading partition and device
attributes from the device.

 NOTE             : The buffer allocation and deallocation will be responsibility of the caller function.
*******************************************************************/
INT32 eSDQueryPartition(UINT32 opcode, INT32 partition_id, VOID *buffer, UINT32 length, INT32 sdhcNum, INT32 slotNum);

// APIs added for SDXC cards

/*******************************************************************
 Name           : SDXC_Switch_Voltage

 Argument       : INT32 sdhcNum - SD Host Controller Number.
          INT32 slotNum - Gives the Slot number.

 Returns        : Success : 0
                  Failure : returns error code

 Description    : This API will switch the card operating voltage from 3P3V to 1P8V for the card in given slot.
*******************************************************************/
extern INT32 SDXC_Switch_Voltage(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : SDXC_UHS_Setting

 Argument       : INT32 sdhcNum - SD Host Controller Number.
          INT32 slotNum - Gives the Slot number.
                  INT32 Current_limit - Selection to limit the maximum current of the card in UHS-1 mode.
                  INT32 drive_strength - Selection of suitable output driver strength in UHS-1 modes depends on the host
environment. INT32 cmd_sys - A specific function can be extended and controlled by a set of shared commands. INT32
access_mode - Selection of SD bus interface speed modes.

 Returns        : Success   : 0
                  Failure   : returns error code

 Description    : This API will switch the card operating voltage from 3P3V to 1P8V for the card in given slot.
*******************************************************************/
extern INT32 SDXC_UHS_Setting(INT32 sdhcNum, INT32 slotNum, INT32 Current_limit, INT32 drive_strength, INT32 cmd_sys,
                              INT32 access_mode);

/*******************************************************************
 Name           : SDIOSelectUHSMode

 Argument       : INT32 sdhcNum - SD Host Controller Number.
          INT32 slotNum - The Slot number.
                  INT32 drive_strength - Selection of suitable output driver strength in UHS-1 modes depending on the
host environment. INT32 uhs_mode - Selection of UHS speed modes.

 Returns        : Success   : 0
                  Failure   : returns error code

 Description    : This API will select the UHS mode for the SDIO card
          and switch the card operating voltage from 3P3V to 1P8V for the card in given slot.
*******************************************************************/
extern INT32 SDIOSelectUHSMode(INT32 sdhcNum, INT32 slotNum, INT32 drive_strength, INT32 uhs_mode);

/*******************************************************************
 Name           : EnableUHSTuning

 Argument       : INT32 uhs_mode - Selection of UHS speed modes.
          INT32 sdhcNum - SD Host Controller Number.
          INT32 slotNum - The Slot number.

 Returns        : Success : 0
                  Failure : returns error code

 Description    : This API will enable Tuning for SDR50 and SDR104
*******************************************************************/
INT32 EnableUHSTuning(INT32 uhs_mode, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : SDIOSuspend

 Argument       : UINT32 functionNumber - function number to suspend.
          INT32 sdhcNum - SD Host Controller Number.
          INT32 slotNum - The Slot number.

 Returns        : Success   : 0
                  Failure   : returns error code

 Description   : This API will suspend a specified function.
*******************************************************************/
INT32 SDIOSuspend(UINT32 functionNumber, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : SDIOResume

 Argument       : UINT32 functionNumber - function number to resume.
          INT32 sdhcNum - SD Host Controller Number.
          INT32 SlotNum - The Slot number.

 Returns        : Success : 0
                  Failure : returns error code

 Description    : This API will resume a suspended function.
*******************************************************************/
INT32 SDIOResume(UINT32 functionNumber, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : StopAtBlock

 Argument       : INT32 sdhcNum - SD Host Controller Number.
          INT32 slotNum - The slot Number.
          UINT32 blkNum - Block number after which to issue stop transmission command (CMD12) for infinite transfer.

 Returns        : Success : 0
                  Failure : returns error code

 Description    : This API will determine after which block to issue CMD12 to stop infinite transfer.
*******************************************************************/
INT32 StopAtBlock(INT32 sdhcNum, INT32 slotNum, UINT32 blkNum);

/*******************************************************************
 Name           :  eMMCSetHS200

 Argument       :  clkfreq - clock frequency in Hz
           drvStrength - driver strength; The possible driver strength values to be used are 0,1,2 and 3
           buswidth - bus width
           sdhcNum - SD Host Controller Number
           slotNum - slot number

 Returns        :  Success   : 0
                   Failure   : returns error code

 Description    :  This API is used to set HS200 mode in the card. Supported in eMMC4.5
*******************************************************************/
INT32 eMMCSetHS200(UINT32 clkfreq, UCHAR drvStrength, UCHAR buswidth, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : eMMCPackedWrite

 Argument       : pcmd - Pointer to the structure, packedcmd
          sdhcNum - SD Host Controller Number
          slotNum - slot number

 Returns        : Success   : 0
                  Failure   : returns error code

 Description    : This API is used to send Packed Write Command.
*******************************************************************/
INT32 eMMCPackedWrite(struct packedcmd *pcmd, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name           : eMMCPackedRead

 Argument       : pcmd - Pointer to the structure, packedcmd
          sdhcNum - SD Host Controller Number
          slotNum - slot number

 Returns        : Success : 0
                  Failure : returns error code

 Description    : This API is used to send Packed Read Command.
*******************************************************************/
INT32 eMMCPackedRead(struct packedcmd *pcmd, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	 	 : eMMCEnableHPI

 Argument 	 : sdhcNum - SD Host Controller Number
           slotNum - slot number
           hpi - 1 to enable hpi and 0 to disable it

 Returns	 : Success : 0
           Failure : returns error code

 Description	 : This API is used to Enable High Priority Interrupt Mechanism
*******************************************************************/
INT32 eMMCEnableHPI(INT32 sdhcNum, INT32 slotNum, UCHAR hpi);

/*******************************************************************
 Name 	  	  : eMMCSetHPI

 Argument 	  : sdhcNum - SD Host Controller Number
            slotNum - slot Number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to set High Priority Interrupt Mechanism.
*******************************************************************/
INT32 eMMCSetHPI(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : eMMCSetTime

 Argument 	  : sdhcNum - SD Host Controller Number
            slotNum - slot Number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to issue CMD49 to send the Real Time Clock information to the card
*******************************************************************/
INT32 eMMCSetTime(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : eMMCPowerNotify

 Argument 	  : pwr - power on or off
            sdhcNum - SD Host Controller Number
            slotNum - slot Number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to issue power-on or power-off notification to the card
*******************************************************************/
INT32 eMMCPowerNotify(UCHAR pwr, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : eMMCEnableBKOPS

 Argument 	  : sdhcNum - SD Host Controller Number
            slotNum - slot Number
            bkops - 1 to enable bkops and 0 to disable it

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to enable Background Operations
*******************************************************************/
INT32 eMMCEnableBKOPS(INT32 sdhcNum, INT32 slotNum, UCHAR bkops);

/*******************************************************************
 Name 	  	  : eMMCStartBKOPS

 Argument 	  : sdhcNum - SD Host Controller Number
            slotNum - slot Number

 Returns	  : Success : 0
            Failure   : returns error code

 Description	  : This API is used to inform the card to start Background Operations
*******************************************************************/
INT32 eMMCStartBKOPS(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : MMCWriteExtCSD

 Argument 	  : offset - ext csd offset
            value - value to be written to the ext csd
            sdhcNum - SD Host Controller Number
            slotNum - slot Number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to issue cmd6 to update any offset value in ext csd register
*******************************************************************/
INT32 MMCWriteExtCSD(UINT32 offset, UCHAR value, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : GetDeviceStatus_CMD13

 Argument 	  : status - device status
            sdhcNum - SD Host Controller Number
            slotNum - slot number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to get the device status by issuing cmd13
*******************************************************************/
INT32 GetDeviceStatus_CMD13(UINT32 *status, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : eMMCSanitize

 Argument 	  : sdhcNum - SD Host Controller Number
            slotNum - slot Number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to start Sanitize operation
*******************************************************************/
INT32 eMMCSanitize(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : MMCErase

 Argument 	  : start_addr - start address to erase
            end_addr - end address
                erasetype - can be any of the below values
                    0x00000000 - Erase
                    0x00000003 - Discard
                    0x80000000 - Secure Erase
                    0x00000001 - Trim
                    0x80000001 - Secure Trim Step 1
                    0x80008000 - Secure Trim Step 2
            sdhcNum - SD Host Controller Number
            slotNum - slot Number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to perform erase/trim/secure erase/secure trim/discard operation
*******************************************************************/
INT32 MMCErase(INT32 start_addr, INT32 end_addr, INT32 erasetype, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : MMCLockUnlock

 Argument 	  : lockdata - it is a mmc_lock_data structure
            sdhcNum - SD Host Controller Number
            slotNum - slot number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to lock/unlock the device or set/clear password for the device
*******************************************************************/
INT32 MMCLockUnlock(struct mmc_lock_data lockdata, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : MMCSleepAwake

 Argument 	  : slp_awk - 1 for sleep and 0 for awake
            sdhcNum - SD Host Controller Number
            slotNum - slot number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to switch the card from standby state to sleep state
*******************************************************************/
INT32 MMCSleepAwake(UINT32 slp_awk, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : eMMCCacheONOFF

 Argument 	  : onoff - 1 to On Cache usage and 0 to Off the cache usage
            sdhcNum - SD Host Controller Number
            slotNum - slot number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to ON/OFF the Cache.
*******************************************************************/
INT32 eMMCCacheONOFF(UINT32 onoff, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : eMMCFlushCache

 Argument 	  : sdhcNum - SD Host Controller Number
            slotNum - slot number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to flush the data from Cache to non-volatile memory
*******************************************************************/
INT32 eMMCFlushCache(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : eMMCContextConfig

 Argument 	  : id - context_id starting from 1
                rel_mode - Reliability mode selection
            lu_mult - large unit multiplier
            lu_context - large unit context selection
            dir - activation and direction selection
                sdhcNum - SD Host Controller Number
            slotNum - slot number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to set the configuration for the Context ID
*******************************************************************/
INT32 eMMCContextConfig(UCHAR id, UCHAR rel_mode, UCHAR lu_mult, UCHAR lu_context, UCHAR dir, INT32 sdhcNum,
                        INT32 slotNum);

/*******************************************************************
 Name 	  	  : EnableDisableAutoCMD

 Argument 	  : autocmd_enable - enables or disables autocmd. Set to either SD_AUTOCMD_DISABLE/SD_AUTOCMD12/SD_AUTOCMD23
                sdhcNum - SD Host Controller Number
            slotNum - slot number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to enable or disable autocmd12/23
*******************************************************************/
VOID EnableDisableAutoCMD(INT32 autocmd_enable, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name 	  	  : eMMCProtocolRD

 Argument 	  : buf - buffer to hold read data
            blkcnt - no. of blocks to be read
            arg - argument
                sdhcNum - SD Host Controller Number
            slotNum - slot number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to read specified number of blocks of extended security protocol information
*******************************************************************/
INT32 eMMCProtocolRD(VOID *buf, UINT32 blkcnt, UINT32 arg, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
Name 	  	  : eMMCProtocolWR

 Argument 	  : buf - buffer holding data to be written
            blkcnt - no. of blocks to be written
            arg - argument
                sdhcNum - SD Host Controller Number
            slotNum - slot number

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to write specified number of blocks of extended security protocol information
*******************************************************************/
INT32 eMMCProtocolWR(VOID *buf, UINT32 blkcnt, UINT32 arg, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : setDebugLevel

 Argument 	  : level - debug level - 0 to 5

 Returns	         : None

 Description	  : This API is used to set the Debuglevel from 0 - 5
*******************************************************************/
VOID setDebugLevel(UINT32 level);

/*******************************************************************
 Name 	  	  : Get_NumOfSDHostControllers

 Argument 	  : None

 Returns	         : No. of SD host Controllers used

 Description	  : This API is used to get the number of host controllers used
*******************************************************************/
INT32 Get_NumOfSDHostControllers(VOID);

/*******************************************************************
 Name 	  	  : Get_NumOfSlots

 Argument 	  : sdhcNum - SD host controller number

 Returns	         : No. of slots in the SD host controller instance specified

 Description	  : This API is used to get the number of slots supported by
                        the host controller specified
*******************************************************************/
INT32 Get_NumOfSlots(INT32 sdhcNum);

/*******************************************************************

  Name           : eMMCCardReset


  Argument     :  	 reset - 0 for software reset using CMD0 with arg 0xF0F0F0F0 reset, and 1 for hardware reset,
                        2 for power reset,3 for CMD0 with arg0 reset

                sdhcNum - SD host controller number
                slotNum - slot number

  Returns        : Success : 0

                       Failure   : returns error code

  Description   : This API is used to reset the card to PreIdle state
*******************************************************************/
INT32 eMMCCardReset(UCHAR reset, INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
  Name           : eMMCCardInit

 Argument 	    : sdhcNum - SD Host Controller Number
                     slotNum - slot Number

  Returns        : Success : 0
                       Failure   : returns error code

  Description   : This API is used to perform eMMC card Initialization
*******************************************************************/
INT32 eMMCCardInit(INT32 sdhcNum, INT32 slotNum);

/*******************************************************************
 Name 	  	  : SDMMC_SetVoltage

 Argument 	  : sdhcNum - SD Host Controller Number
            slotNum - slot number
            power - 0 for power off, 5 for 1.8V, 6 for 3.0V and 7 for 3.3V

 Returns	  : Success : 0
            Failure : returns error code

 Description	  : This API is used to set voltage in Host Controller
*******************************************************************/
INT32 SDMMC_SetVoltage(INT32 sdhcNum, INT32 slotNum, INT32 power);

VOID nex_irqhandler(INT32 sdhcNum, INT32 slotNum);

INT32 Nex_HwInit(VOID);

#endif
