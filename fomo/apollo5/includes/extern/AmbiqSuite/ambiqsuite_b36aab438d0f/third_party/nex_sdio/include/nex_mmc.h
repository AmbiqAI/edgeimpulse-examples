//____________________________________________________________________
// Copyright Info   : All Rights Reserved - (c) Arasan Chip Systems Inc.,
//
// Name             :  nex_mmc.h
//
// Creation Date    :  12-Feb-2007
//
// Description      :  This file contains the header file code for interfacing with the MMC and SD driver
//
// Changed History  :
//<Date>        <Author>        <Version>        < Description >
// 19-Feb-2006    AG              0.1               base code
//____________________________________________________________________

#ifndef _NEX_MMC_H
#define _NEX_MMC_H

#include "nex_api.h"
#include "nex_porting.h"

#define MMC_ERR_NONE           0
#define MMC_ERR_TIMEOUT        1
#define MMC_ERR_BADCRC         2
#define MMC_ERR_FAILED         3
#define MMC_ERR_INVALID        4
#define MMC_CMD_CMP_RCV        5
#define MMC_TX_CMP_RCV         6

#define MMC_CARD_BUSY          0x80000000
#define SDHC_IS_CCS_HIGH       0x40000000
#define RETRIES_COUNT          2

// Command Err Defines
#define CMD_ERR_CRC            0x08

// IOS Defines
#define SD_POWER_OFF           0
#define SD_POWER_UP            1
#define SD_POWER_ON            2

// For SD High-Speed

#define SWITCH_CLASS10          (1 << 10)
#define SWITCH_CMD_SET          0x00 /* Change the command set */
#define SWITCH_SET_BITS         0x01 /* Set bits which are 1 in value */
#define SWITCH_CLEAR_BITS       0x02 /* Clear bits which are 1 in value */
#define SWITCH_WRITE_BYTE       0x03 /* Set target to value */

#define UHS_EXE_TUNING          1 << 6
#define SAMPLING_CLOCK_SELECT   1 << 7

#define SD_SPEC_VER_0           0 // Compliant to SD Version 1.0-1.01
#define SD_SPEC_VER_1           1 // Compliant to SD Version 1.10
#define SD_SPEC_VER_2           2 // Compliant to SD Version 2.00

// MMC-HC - CMD1 Access modes
#define SECTOR_MODE             0x4
#define BYTE_MODE               0x00
#define MMC_HC_MASK             0x60000000
#define MMC_HC_ACCESSMODE       0x40000000

#define CHECK_SPEED             0
#define SET_SPEED               1

// eMMC Packed command
#define PACKED_CMD_VERSION      0x1
#define PACKED_CMD_READ         0x01
#define PACKED_CMD_WRITE        0x02

#define MMC_R1B_EXCEPTION_EVENT (0x1 << 6)
#define MMC_URGENT_BKOPS        0x1
#define MMC_EXT_SEC_EN          0x1 << 4
#define MMC_PKD_EVT_EN          0x1 << 3

#define MMC_IS_LOCKED           0x1 << 25
#define MMC_LOCK_UNLOCK_FAILED  0x1 << 24

// Card Command that represents the Command that is sent to the Hardware
struct sd_card_command
{
    UINT32 opcode;
    UINT32 arg;
    UINT32 arg2;                  /* added for Memory PRO EX_SET_CMD */
    UINT32 resp[4];
    INT32 flags;                  /* expected response type */
    UINT32 retries;               /* max number of retries */
    INT32 error;                  /* command error */
    struct sd_card_data *data;    /* data segment associated with cmd */
    struct sd_card_request *mrq;  /* associated request */
};

// The structure represents the Card Data. It also represents the associated request, data len, block size etc
struct sd_card_data
{

    UINT32 timeout_ns;            /* data timeout (in ns, max 80ms) */
    UINT32 timeout_clks;          /* data timeout (in clocks) */
    UINT32 blocksize;             /* data block size */
    UINT32 no_of_blocks;          /* number of blocks */
    UINT32 data_len;
    UINT32 bytes_xfered;
    UINT32 data_flag;
    UINT32 buffer_len;
    struct sd_card_command *stop; /* stop command */
    struct sd_card_request *mrq;  /* associated request */

    SD_NEX_ADMA_TABLE *card_adma_table;
    SCATTERLIST *sg;              /* I/O scatter list */
    UINT32 sg_len;                /* size of scatter list */
    INT32 error;                  /* data error */
    INT32 flags;
    UCHAR *buffer;                /*Changed for error/warning at nex_sdio.c line no 581*/
};

struct emmc_boot_data
{
    UINT32 total_boot_cnt;
    UINT32 chunk_cnt;
};

// The Card request
struct sd_card_request
{
    struct sd_card_command *cmd;
    struct sd_card_data *data;
    struct sd_card_command *stop;
    INT32 done_data;                        /* completion data */
    VOID (*done)(struct sd_card_request *); /* completion function */
};

// The IOS Structure
struct sd_ios
{
    UINT32 clock;      /* clock rate */
    UINT32 vdd;
    UCHAR bus_mode;    /* command output mode */
    UCHAR chip_select; /* SPI chip select */
    UCHAR power_mode;  /* power supply mode */
    UCHAR bus_width;   /* data bus width */
};

struct sd_host;
struct sd_mmc_cid;     /* card identification */
struct sd_mmc_csd;     /* card specific */
struct sd_sdio_cis;    /* CIS information for sd */

struct sd_hw_host_ops
{
    VOID (*request)(struct sd_host *host, struct sd_card_request *req);
    VOID (*set_ios)(struct sd_host *host, struct sd_ios *ios);
    INT32 (*get_ro)(struct sd_host *host);
};

struct sd_card;

// The SDHOST structure.
struct sd_host
{
    struct nex_host *nexhost;
    WORK_STRUCT detect;
    SEMAPHORE command_sema;
    UINT32 host_version;           /* added for SD3.0 */
    UINT32 card_version;           /* added for SD3.0 */
    UINT32 low_volt;               /* added for SD3.0 */
    UINT32 f_min;
    UINT32 f_max;
    UINT32 sdr12_support;          /* added for SD3.0 */
    UINT32 sdr25_support;          /* added for SD3.0 */
    UINT32 sdr50_support;          /* added for SD3.0 */
    UINT32 sdr104_support;         /* added for SD3.0 */
    UINT32 ddr50_support;          /* added for SD3.0 */
    UINT32 mA200_support;          /* added for SD3.0 */
    UINT32 mA400_support;          /* added for SD3.0 */
    UINT32 mA600_support;          /* added for SD3.0 */
    UINT32 mA800_support;          /* added for SD3.0 */
    UINT32 hs_support;             /* added for SD3.0 */
    UINT32 ocr_avail;
    UINT32 mode;                   /* current card mode of host */
    UINT32 card_latch;             /* For read only status in sd card 1 == write enabled 0 == write disabled */
    UINT32 block_length;
    UINT32 dma_enable_flag;
    UINT32 dma_type;
    UINT32 boot_size;
    INT32 slot;
    INT32 rca;                     /* the current RCA of the card */
    INT32 ocr;                     /* the current OCR setting */
    UCHAR card_initialized;        /* Indicates the Card Status. SD_TRUE is initialized,  SD_FALSE is not initialized*/
    UCHAR autocmd_enable;          /* Enable/disable autocmd */
    UCHAR wq_scheduled;            /* To check whether workqueue was already scheduled or not*/
    struct sd_hw_host_ops *ops;
    struct sd_ios ios;             /* current io bus settings */
    struct sd_card *card_selected; /* the selected MMC card */
    struct sd_card *active_card;
    struct sd_card_data card_rw_data;

    VOID (*user_call_back)(SD_CALL_BACK_INFO *);
    ULONG caps;                    /* Host capabilities */
};

struct sd_card_function
{
    struct sd_card *card;
    UINT32 number;
    UINT32 vendor;
    UINT32 device;
};

// The card structure
struct sd_card
{
    struct sd_host *host;          /* the host this device belongs to */
    INT32 rca;                     /* relative card address of device */
    UINT32 state;                  /* (our) card state */
    UINT32 type;                   /* type of card */
    UINT32 raw_cid[4];             /* raw card CID */
    UINT32 raw_csd[4];             /* raw card CSD */
    UINT32 n_functions;            /* number of card subfunctions */
    UINT32 full_speed;             /* means Full speed supported */
    UINT32 high_speed;             /* 1 - means high speed supported */
    UINT32 UHS50_support;          /* 1 - means supported */
    UINT32 UHS104_support;         /* 1 - means supported */
    UINT32 readWait;               /* 1 - means ReadWait supported */
    UINT32 suspendResume;          /* 1-means Suspend/Resume supported */
    UINT32 functionSelect;         /* function number of the selected function */
    UINT32 suspendedFn;            /* function number of the suspended fn */
    UINT32 block_gap;              /* set to 1 when stop at blockgap is set */
    struct sd_mmc_cid cid;         /* card identification */
    struct sd_mmc_csd csd;         /* MMC card specific CSD information*/
    struct mspro_info dinfo;
    struct sd_mmc_csd_v4 mmc_csd_v4;    /* MMC card version 4 specific CSD information*/
    struct sd_csd_v2 sd_csd2_info;      /* SD Card specific CSD v2 Information */
    struct sd_csd_v1 sd_csd_info;       /* common memory allloccation (4 bytes): can take v1 or v2*/
    struct sd_mmc_ext_csd ext_csd;      /* extended csd structure of MMCv4 cards */
    struct sd_sdio_cis cis;             /* CIS information for sdio */
    struct sd_scr scr;                  /* SCR information for SD */
    struct sd_card_function *functions; /* function data */
    UCHAR *raw_scr;                     /* raw card SCR */
    UCHAR *savedRegisters;              /* buffer to hold the saved register values from register 0x00 to 0x0C of the host */
};

static inline VOID *sd_get_private_data(struct sd_host *host)
{
    return (VOID *)host->nexhost;
}

// All Function Prototypes  //

extern VOID sd_request_done(struct sd_host *host, struct sd_card_request *mrq);
extern VOID sd_free_host(struct sd_host *host);

VOID  sd_bus_rescan(ULONG data);
INT32 sd_add_host(struct sd_host *host);
struct sd_host *sd_alloc_host(INT32 sdhc, INT32 slot);

VOID  sd_dump_ext_csd(struct sd_host *host);
INT32 sd_send_card_status_cmd13(struct sd_host *host, struct sd_card_command *cmd);

INT32 sd_mmc_read_multiple_bytes(struct sd_host *host, VOID *buffer, UINT32 length, UINT32 address);
INT32 sd_mmc_write_multiple_bytes(struct sd_host *host, VOID *buffer, UINT32 length, UINT32 address);

INT32 sd_mmc_read_byte(struct sd_host *host, UCHAR *buffer, UINT32 length, UINT32 address);
INT32 sd_mmc_write_byte(struct sd_host *host, UCHAR buffer, UINT32 length, UINT32 address);

INT32 sd_mmc_write_multiple_bytes_dma(struct sd_host *host, VOID *buffer, UINT32 length, UINT32 address);
INT32 sd_mmc_read_multiple_bytes_dma(struct sd_host *host, VOID *buffer, UINT32 length, UINT32 address);

INT32 sd_wait_for_cmd(struct sd_host *host, struct sd_card_command *cmd, UINT32 retries);
INT32 sd_wait_for_req(struct sd_host *host, struct sd_card_request *request);

VOID  sd_set_dma_type(struct sd_host *host, SD_DMA_TYPE dma_type);
INT32 sd_set_high_speed_cmd6(struct sd_host *host);

INT32 emmc_boot_partition_select(struct sd_host *host, UCHAR boot_config);
INT32 emmc_set_boot_buswidth(struct sd_host *host, UCHAR bus_width);
INT32 emmc_set_ddr(struct sd_host *host, UCHAR bus_width);
INT32 emmc_set_boot_configprot(struct sd_host *host, UCHAR config_prot);
INT32 emmc_set_boot_WPEnable(struct sd_host *host, UCHAR config_prot);
INT32 emmc_set_usr_WPEnable(struct sd_host *host, UCHAR usr_wp_en);
INT32 emmc_set_erase_grp_defn(struct sd_host *host, UCHAR erase_grp_defn);
INT32 emmc_set_part_complete(struct sd_host *host, UCHAR part_complete);
INT32 emmc_set_configure_EUDA(struct sd_host *host, INT32 enh_area_size, INT32 enh_start_address);
INT32 emmc_set_configure_GPP(struct sd_host *host, UCHAR gpp_no, INT32 GP_SIZE_MULT_X_0, INT32 GP_SIZE_MULT_X_1,
                             INT32 GP_SIZE_MULT_X_2);
INT32 emmc_set_part_attribute(struct sd_host *host, UCHAR part_attrb);
INT32 emmc_set_pre_idle_state(struct sd_host *host, UCHAR rst_n_en);
INT32 emmc_set_FW_config(struct sd_host *host, UCHAR enable_update);

INT32 emmc_send_write_prot_type(struct sd_host *host, VOID *Buffer, UINT32 Length, INT32 address);
INT32 emmc_send_write_prot(struct sd_host *host, VOID *Buffer, UINT32 Length, INT32 address);
INT32 emmc_set_write_prot(struct sd_host *host, INT32 wp_address);
INT32 emmc_clr_write_prot(struct sd_host *host, INT32 wp_address);

INT32 mmc_set_blk_cnt(struct sd_host *host, struct emmc_cmd23_arg cmdarg);

VOID sd_power_up(struct sd_host *host);
VOID sd_delay(UINT32 millisecond);
VOID sd_power_off(struct sd_host *host);

// Macro definitions
#define MMC_STATE_PRESENT     (1 << 0)
#define MMC_STATE_DEAD        (1 << 1)
#define MMC_STATE_BAD         (1 << 2)

#define MMC_STATE_SDMEM       (1 << 4)
#define MMC_STATE_SDIO        (1 << 5)
#define M_IS_CARD_SDMEM(c)    ((c)->state & MMC_STATE_SDMEM)

#define mmc_card_set_sdmem(c) ((c)->state |= MMC_STATE_SDMEM)
#define mmc_card_set_sdio(c)  ((c)->state |= MMC_STATE_SDIO)

#define sd_card_present(c)    ((c)->state |= MMC_STATE_SDMEM)
#define sd_card_dead(c)       ((c)->state & MMC_STATE_DEAD)

#define M_IS_CARD_SDMEM(c)    ((c)->state & MMC_STATE_SDMEM)
#define M_IS_CARD_SDIO(c)     ((c)->state & MMC_STATE_SDIO)
#define M_IS_CARD_SD(c)       (M_IS_CARD_SDMEM(c) || M_IS_CARD_SDIO(c))
#define M_IS_CARD_MMC(c)      (!M_IS_CARD_SDMEM(c)) // Changed from (!mmc_card_sd(c) )

#define M_SET_CARD_PRESENT(c) ((c)->state |= MMC_STATE_PRESENT)
#define M_SET_CARD_DEAD(c)    ((c)->state |= MMC_STATE_DEAD)
#define M_SET_CARD_BAD(c)     ((c)->state |= MMC_STATE_BAD)

#define READ_BITS(value, mask, shift, out)                                                                             \
    {                                                                                                                  \
        UINT32 temp;                                                                                                   \
        temp = (value & mask);                                                                                         \
        temp >>= shift;                                                                                                \
        out = temp;                                                                                                    \
    }

struct nex_host;

INT32 mmc_set_highspeed(struct sd_host *host);
INT32 mmc_bus_test(struct sd_host *host, UCHAR bus_width);
INT32 mmc_Erase(struct sd_host *host, INT32 start_addr, INT32 end_addr, INT32 erasetype);
INT32 mmc_LockUnlock(struct sd_host *host, struct mmc_lock_data lockdata);
INT32 mmc_SleepAwake(struct sd_host *host, UINT32 slp_awk);
INT32 mmc_WriteExtCSD_Cmd6(struct sd_host *host, UINT32 offset, UCHAR value);
INT32 mmc_set_fullspeed(struct sd_host *host);

INT32 eSD_SelectPartition(struct sd_host *host, INT32 arg);
INT32 eSD_ManagePartition(struct sd_host *host, VOID *buffer, UINT32 length, UINT32 arg);
INT32 eSD_ManagePartition_dma(struct sd_host *host, SD_NEX_ADMA_TABLE *adma_buffer, UINT32 length, UINT32 arg);
INT32 eSD_QueryPartition(struct sd_host *host, VOID *buffer, UINT32 length, UINT32 arg);
INT32 eSD_QueryPartition_dma(struct sd_host *host, SD_NEX_ADMA_TABLE *adma_buffer, UINT32 length, UINT32 arg);

INT32 emmc_SendTuningBlk_CMD21(struct sd_host *sdhost, INT32 slotNum);
INT32 emmc_SetTuning_CMD21(struct sd_host *sdhost, INT32 slotNum);
INT32 emmc_SetHS200(struct sd_host *host, UINT32 clkfreq, UCHAR drvStrength, UCHAR buswidth);
INT32 emmc_PackedWrite(struct sd_host *host, struct packedcmd *pcmd);
INT32 emmc_PackedRead(struct sd_host *host, struct packedcmd *pcmd);
VOID  emmc_CreatePackedHeader(struct sd_host *host, struct packedcmd *pcmd, UCHAR *buffer);
INT32 emmc_EnableHPI(struct sd_host *host, UCHAR hpi);
INT32 emmc_SetHPI(struct sd_host *host);
INT32 emmc_SetTime(struct sd_host *host);
INT32 emmc_PowerNotify(struct sd_host *host, UCHAR pwr);
INT32 emmc_EnableBKOPS(struct sd_host *host, UCHAR bkops);
INT32 emmc_StartBKOPS(struct sd_host *host);
INT32 emmc_CacheONOFF(struct sd_host *host, UINT32 onoff);
INT32 emmc_FlushCache(struct sd_host *host);
INT32 emmc_ContextConfig(struct sd_host *host, UCHAR id, UCHAR rel_mode, UCHAR lu_mult, UCHAR lu_context, UCHAR dir);
INT32 emmc_ProtocolRD_CMD53(struct sd_host *host, VOID *buf, UINT32 blkcnt, UINT32 arg);
INT32 emmc_ProtocolWR_CMD54(struct sd_host *host, VOID *buf, UINT32 blkcnt, UINT32 arg);
INT32 emmc_Sanitize(struct sd_host *host);
INT32 emmc_HWReset(struct sd_host *host);
INT32 emmc_CardInit(struct sd_host *host);

// Added for SDXC
INT32 sd_SetUltraHighSpeed(struct sd_host *host, INT32 slotNum, INT32 Current_limit, INT32 drive_strength,
                           INT32 cmd_sys, INT32 access_mode);
INT32 sd_SDXC_SwitchUHS(struct sd_card *card, UINT32 arg);
INT32 sd_card_voltage_switch_cmd11(struct sd_host *host, INT32 sloNum);
INT32 sdxc_SetBlockCount_ACMD41(struct sd_host *host, UINT32 ocr, UINT32 *rocr);
INT32 sdxc_SetSpeedClass_CMD20(struct sd_host *host, UCHAR Control);
INT32 sdxc_SetTuning_CMD19(struct sd_host *sdhost, INT32 slotNum);
INT32 sdxc_SetBlockCount_CMD23(struct sd_host *host, UINT32 BlockCount);
INT32 sd_select_card_cmd7(struct sd_host *host, struct sd_card *card);
VOID  sd_read_csd_cmd9(struct sd_host *host);
INT32 sd_UHS_Tuning(struct sd_host *host, INT32 slotNum, INT32 uhs_mode);
INT32 sd_TuningProc(struct sd_host *sdhost, INT32 slot);
INT32 sd_stop_at_block(struct nex_host *host, UINT32 blkNum);
INT32 sd_StopTransmitCmd12(struct sd_host *host);
INT32 sd_ErrorRecovery(struct sd_host *host);
INT32 sd_ACMD12ErrorRecovery(struct sd_host *host);
VOID  sd_send_idle_cmd0(struct sd_host *host, UINT32 arg);
INT32 sd_wait_for_app_cmd(struct sd_host *host, UINT32 rca, struct sd_card_command *cmd, INT32 retries);
INT32 sd_set_blockLen(struct sd_host *, UINT32 blocksize);
VOID  sd_enable_disable_dma(struct sd_host *host, SD_DMA_MODE dma_flag);
INT32 sd_set_bus_width(struct sd_host *host, UCHAR bus_width);
VOID  sd_detect_change(struct sd_host *host);
INT32 sd_check_clock(UINT32, INT32, INT32);
VOID  sd_set_user_voltage(struct nex_host *, UINT32);
VOID  sd_set_user_clock(struct nex_host *, UINT32);
INT32 sd_SetHighSpeed(struct sd_host *host);
INT32 sd_SetFullSpeed(struct sd_host *host);
VOID  sd_read_ext_csd(struct sd_host *host);
VOID  sd_set_power_class_cmd6(struct sd_host *host, INT32 pwr_class);

#endif
