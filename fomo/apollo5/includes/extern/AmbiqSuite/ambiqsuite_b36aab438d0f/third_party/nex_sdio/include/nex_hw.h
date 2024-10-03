//____________________________________________________________________
//Copyright Info  : All Rights Reserved - (c) Arasan Chip Systems Inc.,
//
//Name            :  nex_hw.h
//
//Creation Date   :  14-Feb-2007
//
//Description     :
//Contains header file information for the hardware file
//
//Changed History :
//<Date>        <Author>        <Version>        < Description >
//____________________________________________________________________

#ifndef _NEX_HW_H
#define _NEX_HW_H

#include "nex_mmc.h"
#include "nex_porting.h"

// NEX Host Controller Registers
#define HOST_DMASTART               0x00 // SDMA systems address or Argument 2 for storing BLK_CNT of ADMA when using CMD23 (upto 4294967296 blocks * 512 = 2 Tera Bytes )
#define HOST_BLKSIZE                0x04
#define HOST_BLKCOUNT               0x06
#define HOST_ARG0                   0x08
#define HOST_ARG1                   0x0a
#define HOST_ARG2                   0xf4
#define HOST_TRANSMODE              0x0c
#define HOST_CMD                    0x0e
#define HOST_RESP0                  0x10
#define HOST_RESP1                  0x12
#define HOST_RESP2                  0x14
#define HOST_RESP3                  0x16
#define HOST_RESP4                  0x18
#define HOST_RESP5                  0x1a
#define HOST_RESP6                  0x1c
#define HOST_RESP7                  0x1e

#define NEX_HOST_SD_BUFFER         0x20
#define HOST_PRESENT_STATE         0x24
#define HOST_CONTROL               0x28
#define HOST_POWER_CONTROL         0x29
#define HOST_BLOCKGAP_CONTROL      0x2a
#define HOST_WAKEUP_CONTROL        0x2b
#define HOST_CLOCK_CONTROL         0x2c
#define HOST_TIMEOUT_CONTROL       0x2e
#define HOST_SOFTWARE_RESET        0x2f
#define HOST_NORMINT_STATUS        0x30
#define HOST_ERRINT_STATUS         0x32
#define HOST_NORMINT_STATUSEN      0x34
#define HOST_ERRINT_STATUSEN       0x36
#define HOST_NORMINT_SIGEN         0x38
#define HOST_ERRINT_SIGEN          0x3a
#define HOST_ACMD12_ERRSTATUS      0x3c
#define HOST_CAPABLITY             0x40
#define HOST_CAPABLITY_SD3         0x44
#define HOST_MAXCURR_CAPABLITY     0x48
#define HOST_SLOT_INTSTATUS        0xfc
#define HOST_CONTROLLER_VERSION    0xfe
#define HOST_ADMA_ERR              0x54
#define HOST_ADMASTART             0x58
#define HOST_BOOT_DATA_TIMEOUT     0x70

// Added for SD 3.0
#define HOST_SHAREDBUS_CONTROL     0xE0
#define HOST_CONTROL2              0x3e


// For Capability Control Register 0x40

// MSB 32 bits are defined here

#define CAPAB_SDR50_REQ_TUNING     	1<<13
#define CAPAB_TIMER_COUNT_RETUNING	0x100
#define CAPAB_DRIVER_D             	0x040
#define CAPAB_DRIVER_C             	0x020
#define CAPAB_DRIVER_A             	0x010
#define CAPAB_DDR50                	0x004
#define CAPAB_SDR104               	0x002
#define CAPAB_SDR50                	0x001

#define CAPAB_SHARED_BUS           0x00080000000
#define CAPAB_EMBEDDED_SLOT_1      0x00040000000
#define CAPAB_ASYNC_INT            0x00020000000
#define CAPAB_64BIT_BUS            0x00010000000
#define CAPAB_POWER_MASK           0x00007000000
#define CAPAB_SUSPEND_RESUME       0x00000800000
#define CAPAB_SDMA_SUPPORT         0x00000400000
#define CAPAB_HI_SP_MASK           0x00000200000
#define CAPAB_ADMA2                0x00000080000
#define CAPAB_EXT_MEDIA_BUS        0x00000040000
#define CAPAB_BLOCK_4096           0x00000030000
#define CAPAB_BLOCK_2048           0x00000020000
#define CAPAB_BLOCK_1024           0x00000010000
#define CAPAB_CLOCK_MASK           0x0000000ff00
#define CAPAB_TIMEOUT_CLK_UNIT     0x00000000080
#define CAPAB_TIMEOUT_CLK_FREQ     0x00000000080

// For HOST_MAXCURR_CAPABLITY 0x48
#define VDD_180                    0x04000000
#define VDD_300                    0x02000000
#define VDD_330                    0x01000000
#define POWER_330                  0x07
#define POWER_300                  0x06
#define POWER_180                  0x05
#define POWER_SD                   0x01
#define eMMC_HW_RESET              0x10
#define SET_DDR			   			0x4


// Control bus Width HOST CONTROL
#define SDHCI_NEX_1BITBUS           0xDD          // failing 2nd bit of 028h register : buswidth 1 now
#define SDHCI_NEX_4BITBUS           0x02          // setting 2nd bit of 028 register : buswidth 4 now
#define SDHCI_NEX_8BITBUS           0x20          // setting 5th bit of 028h register : buswidth 8 now
#define SDHCI_NEX_HIGH_SPEED_ENABLE 0x04          // setting bit 3 of 028h : high speed enable now
#define SDHCI_NEX_FULL_SPEED_ENABLE 0xFB 

// Transfer Mode Flags
#define TRANSFER_MODE_DMA             0x01
#define TRANSFER_MODE_BLK_CNT_ENABLE  0x02
#define TRANSFER_MODE_ACMD12_ENABLE   0x04
#define TRANSFER_MODE_ACMD23_ENABLE   0x08
#define TRANSFER_MODE_DATA_READ       0x10
#define TRANSFER_MODE_BLK_SELECT      0x06
#define TRANSFER_MODE_MULTIPLE_BLOCKS 0x20

// For DMA Select signal of host control register (offset 028h)
#define DMA_MODE_AMDA1                0x08
#define DMA_MODE_ADMA2                0x10
#define DMA_MODE_ADMA2_64             0x18

#define RSP_BUSY                      0x0
#define CMD_INHIBIT_FLAG              0x00000001
#define DATA_INHIBIT_FLAG             0x00000002
#define CARD_PRESENT                  0x00010000
#define CARD_WRITE_PROTECT            0x00080000
#define HOST_RESET_DATA               0x04
#define HOST_RESET_CMD                0x02
#define HOST_BOOT_EN                  0x100

// Command Response Flags
#define CMD_RESP_NONE                 0x00
#define CMD_RESP_LONG                 0x01
#define CMD_RESP_SHORT                0x02
#define CMD_RESP_SHORT_BUSY           0x03
#define CMD_CRC                       0x08
#define CMD_INDEX                     0x10
#define CMD_DATA                      0x20

// clock control register offset 2ch
#define SD_INT_CLK_ENABLE             0x0001
#define SD_CLK_INT_STABLE             0x0002
#define SD_CLK_START_BIT              0x0004
#define SD_DIV8_SHIFT                 0x0008
#define SD_PROGRAMMABLE_CLOCK_MODE    0x0020
#define SD_CLK_BASE_DIVIDER           0x00FF
#define SD_DISABLE                    0xFFFB
#define SD_CLK_EN_BIT                 0xFFFE

// Hard Ware level Registers
#define HOST_LEDON                    0x01
#define HOST_VENDOR_VERSION           0x00ff
#define HOST_BASECLOCK_MASK           0x0000ff00 // for SD3.0
#define HOST_TIMEOUTCLOCK_MASK        0x0000003f
#define HOST_BASECLOCK_UNIT           7
#define HOST_BASECLOCK_SHIFT          8
#define HOST_TIMEOUTCLOCK_UNIT        0x00000080
#define HOST_BLKLEN_MASK              0x00030000
#define HOST_BLKLEN_SHIFT             16

// Card Present State Register
#define RETUNING_REQUEST              0x08
#define NEX_CARD_PRESENT              0x00010000
#define NEX_CARD_STATE_STABLE         0x00020000
#define NEX_CARD_RESET                0x01
#define HOST_RESET_ALL                0x01
#define SD_BUFFER_READ                0x00000800
#define SD_BUFFER_WRITE               0x00000400

// Reset Offset values
#define NEX_CARD_RESET                0x01
#define HOST_RESET_ALL                0x01
#define HOST_RESET_DATA               0x04
#define	HOST_RESET_CMD                0x02

// Interuppt Flags
#define CARD_TUNING_ERR				  0x04000000
#define CARD_ADMA2_INT                0x02000000
#define CARD_AUTOCMD12_INT            0x01000000
#define CARD_INT_BUS_POWER            0x00800000
#define CARD_DATA_INT_END             0x00400000
#define CARD_DATA_INT_CRC             0x00200000
#define CARD_DATA_INT_TIMEOUT         0x00100000
#define CARD_CMD_INT_INDEX            0x00080000
#define CARD_CMD_END_BIT              0x00040000
#define CARD_CMD_INT_CRC              0x00020000
#define CARD_CMD_TIMEOUT              0x00010000

//Added for MEM_PRO
#define CARD_MEM_PRO_INS       		  0x00000800
#define CARD_RETUNING_PERFORMED	      0x00001000

//Added for SD3.0
#define ERROR_INTERRUPT               0x00008000
#define CARD_BOOT_ACK_RCV        	  0x00002000
#define CARD_BOOT_TERMINATE        	  0x00004000
#define RETUNING_EVENT                0x00001000
#define CARD_INT_C	       		      0x00000800
#define CARD_INT_B       		      0x00000400
#define CARD_INT_A       		      0x00000200

#define CARD_INT_STAT_EN              0x00000100
#define CARD_REMOVE_INT               0x00000080
#define CARD_INSERT_INT               0x00000040
#define CARD_READ_READY               0x00000020
#define CARD_WRITE_READY              0x00000010
#define CARD_INT_DMA_END              0x00000008
#define CARD_BLKGP_INT                0x00000004
#define CARD_DATA_FIN                 0x00000002
#define CARD_CMD_FIN                  0x00000001

//BOOT TIMEOUT VALUE
#define BOOT_TIMEOUT_ACK 			  0x98968    //50ms
#define BOOT_TIMEOUT_DAT 			  0xbebc200  //1s
#define TX_MODE_MULTIBLK_PIO		  0x32
#define TX_MODE_SINGLEBLK_PIO		  0x12
#define TX_MODE_BOOT_PIO			  0x132
#define TX_MODE_BOOT_DIS			  0x0FF
#define BLKGAP_BOOT_ENABLE			  0x20
#define BLKGAP_BOOT_DISABLE_MASK	  0x5F
#define BLKGAP_ALT_BOOT_ENABLE		  0x60
#define BLKGAP_ALT_BOOT_DISABLE_MASK  0x1F
#define BLKGAP_READWAIT				  0x04

// For HOST_SHAREDBUS_CONTROL *** Added for SD 3.0
#define SH_SDCLK_BIN_7 0x07
#define SH_SDCLK_BIN_6 0x06
#define SH_SDCLK_BIN_5 0x05
#define SH_SDCLK_BIN_4 0x04
#define SH_SDCLK_BIN_3 0x03
#define SH_SDCLK_BIN_2 0x02
#define SH_SDCLK_BIN_1 0x01
#define SD_INTA_B_C    0x030
#define SD_INTA_B      0x020
#define SD_INTA        0x010

#define SWITCH_1P8V    0x1000000  // 1 << 24
#define ENABLE_1P8V    0x08       // 1 << 3

// DMA Boundary
#define DMA_BOUNDARY      0x0 //Fixed to 4K DMA boundary.. Needs to be changed if user needs other DMA boundaries..
#define KB                1024
#define DMA_BOUNDARY_INCR ((4<<DMA_BOUNDARY)*KB)

// For HOST_CONTROLLER_VERSION 0xFE
#define HOST_SPEC_VERSION_2 0x01
#define HOST_SPEC_VERSION_3 0x02

#define CARD_CMD_MASK  (CARD_CMD_FIN | CARD_CMD_TIMEOUT |         \
                        CARD_CMD_INT_CRC | CARD_CMD_END_BIT |     \
                        CARD_CMD_INT_INDEX)

#define CARD_DATA_MASK (CARD_DATA_FIN | CARD_INT_DMA_END | CARD_AUTOCMD12_INT |     \
                        CARD_READ_READY | CARD_WRITE_READY | CARD_ADMA2_INT |       \
                        CARD_DATA_INT_TIMEOUT | CARD_DATA_INT_CRC |CARD_BLKGP_INT | \
                        CARD_DATA_INT_END | CARD_BOOT_TERMINATE | CARD_BOOT_ACK_RCV)
                                        

#define TIMEOUT_MAX 0xE

// Macros for reading the registers
#define M_READ_HOST_ADMASTART(host)    M_NEX_READ_WORD(host->token + HOST_ADMASTART)

#define M_READ_HOST_ADMAERR_REG(host)  M_NEX_READ_BYTE(host->token + HOST_ADMA_ERR)

#define M_READ_HOST_DMASTART(host)     M_NEX_READ_WORD(host->token + HOST_DMASTART)

#define M_READ_HOST_BLOCKSIZE(host)    M_NEX_READ_HALF_WORD(host->token	+ HOST_BLKSIZE)

#define M_READ_HOST_BLOCKCOUNT(host)   M_NEX_READ_HALF_WORD(host->token	+ HOST_BLKCOUNT)

#define M_READ_HOST_ARGUMENT(host)     M_NEX_READ_WORD(host->token + HOST_ARG0)

#define	M_READ_HOST_ARGUMENT2(host)	   M_NEX_READ_WORD(host->token + HOST_ARG2)

#define M_READ_HOST_TRANSFERMODE(host) M_NEX_READ_HALF_WORD(host->token + HOST_TRANSMODE)

#define M_READ_HOST_COMMAND(host)      M_NEX_READ_HALF_WORD(host->token + HOST_CMD)

#define M_READ_HOST_RESPONSE_0(host)   M_NEX_READ_HALF_WORD(host->token + HOST_RESP0)

#define M_READ_HOST_RESPONSE_1(host)   M_NEX_READ_HALF_WORD(host->token + HOST_RESP1)

#define M_READ_HOST_RESPONSE_2(host)   M_NEX_READ_HALF_WORD(host->token + HOST_RESP2)

#define M_READ_HOST_RESPONSE_3(host)   M_NEX_READ_HALF_WORD(host->token + HOST_RESP3)

#define M_READ_RESPONSE_REG(host)      M_NEX_READ_WORD(host->token + HOST_RESP0)

#define M_READ_RESPONSE2_REG(host)     M_NEX_READ_WORD(host->token + HOST_RESP2)

#define M_READ_HOST_RESPONSE_4(host)   M_NEX_READ_HALF_WORD(host->token + HOST_RESP4)

#define M_READ_HOST_RESPONSE_5(host)   M_NEX_READ_HALF_WORD(host->token + HOST_RESP5)

#define M_READ_HOST_RESPONSE_6(host)   M_NEX_READ_HALF_WORD(host->token + HOST_RESP6)

#define M_READ_HOST_RESPONSE_7(host)   M_NEX_READ_HALF_WORD(host->token + HOST_RESP7)

#define M_READ_PRESENT_STATE_REG(host) M_NEX_READ_WORD((host->token) + HOST_PRESENT_STATE)

#define M_READ_HOST_BUFFER(host)       M_NEX_READ_WORD((host->token) + NEX_HOST_SD_BUFFER)

#define M_READ_HOST_CONTROL(host)      M_NEX_READ_BYTE(host->token	+ HOST_CONTROL)

#define M_READ_HOST_CONTROL2(host)     M_NEX_READ_HALF_WORD(host->token + HOST_CONTROL2)

#define M_READ_POWER_CONTROL(host)     M_NEX_READ_BYTE(host->token + HOST_POWER_CONTROL)

#define M_READ_BLOCK_GAP_CONTROL(host) M_NEX_READ_BYTE(host->token + HOST_BLOCKGAP_CONTROL)

#define M_READ_WAKEUP_CONTROL(host)    M_NEX_READ_BYTE(host->token + HOST_WAKEUP_CONTROL)

#define M_READ_SD_CLK_STATE(host)      M_NEX_READ_HALF_WORD(host->token	+ HOST_CLOCK_CONTROL)

#define M_READ_TIMEOUT_CONTROL(host)   M_NEX_READ_BYTE(host->token + HOST_TIMEOUT_CONTROL)

#define M_READ_SOFTRESET(host)         M_NEX_READ_BYTE(host->token + HOST_SOFTWARE_RESET)

#define M_READ_INT_REG(host)           M_NEX_READ_WORD(host->token + HOST_NORMINT_STATUS)

#define M_READ_NORMINT_STATUS_EN(host) M_NEX_READ_WORD(host->token + HOST_NORMINT_STATUSEN)

#define M_READ_ERRINT_STATUS_EN(host)  M_NEX_READ_HALF_WORD(host->token + HOST_ERRINT_STATUSEN)

#define M_READ_ERRINT_STATUS(host)     M_NEX_READ_HALF_WORD(host->token + HOST_ERRINT_STATUS)

#define M_READ_NORMINT_SIG_EN(host)    M_NEX_READ_WORD(host->token + HOST_NORMINT_SIGEN)

#define M_READ_ERRINT_SG_EN(host)      M_NEX_READ_HALF_WORD(host->token + HOST_ERRINT_SIGEN)

#define M_READ_ACMD12_ERR_STAT(host)   M_NEX_READ_HALF_WORD(host->token + HOST_ACMD12_ERRSTATUS)

#define M_READ_CAPABLITIES(host)       M_NEX_READ_WORD(host->token + HOST_CAPABLITY)

#define M_READ_CAPABLITIES_SD3(host)   M_NEX_READ_WORD(host->token + HOST_CAPABLITY_SD3)

#define M_READ_CURR_CAPABLITIES(host)  M_NEX_READ_WORD(host->token + HOST_MAXCURR_CAPABLITY)

#define M_READ_HOST_VER(host)          M_NEX_READ_HALF_WORD(host->token + HOST_CONTROLLER_VERSION)

#define M_READ_INT_STATUS(host)        M_NEX_READ_HALF_WORD(host->token	+ HOST_SLOT_INTSTATUS)

// Macros for writing the registers
#define M_WRITE_TRANSFER_REG(host, flag)     M_NEX_WRITE_HALF_WORD(flag, host->token + HOST_TRANSMODE)

#define M_WRITE_ARGUMENT_REG(host, arg)      M_NEX_WRITE_WORD(arg, host->token + HOST_ARG0)

#define M_WRITE_ARGUMENT2_REG(host, arg)     M_NEX_WRITE_WORD(arg, host->token + HOST_ARG2)

#define M_WRITE_CMD_REG(host, opcode, flags) value = (((opcode & 0xff) << 8 ) | (flags & 0xff));            \
                                                        REG_DUMP(host->slot, "COMMAND REG", (value));       \
                                                        M_NEX_WRITE_HALF_WORD(value, host->token + HOST_CMD)

#define M_WRITE_BLKGAP_CONTROL(host, value)    M_NEX_WRITE_BYTE(value, host->token + HOST_BLOCKGAP_CONTROL)

#define M_WRITE_INT_REG(host, value)           M_NEX_WRITE_WORD(value, host->token + HOST_NORMINT_STATUS)

#define M_WRITE_NORMINT_SIGEN(host, value)     M_NEX_WRITE_WORD(value, host->token + HOST_NORMINT_SIGEN)

#define M_WRITE_HOST_CONTROL(host, value)      M_NEX_WRITE_BYTE(value, host->token + HOST_CONTROL)

#define M_WRITE_HOST_CONTROL2(host, value)     M_NEX_WRITE_HALF_WORD(value, host->token	+ HOST_CONTROL2)

#define M_WRITE_POWER_CONTROL(host, value)     M_NEX_WRITE_BYTE(value,	host->token	+ HOST_POWER_CONTROL)

#define M_WRITE_SD_CLK_STATE(host, value)      M_NEX_WRITE_HALF_WORD(value, host->token + HOST_CLOCK_CONTROL)

#define M_WRITE_SOFTRESET(host, value)         M_NEX_WRITE_BYTE(value,	host->token	+ HOST_SOFTWARE_RESET)

#define M_WRITE_NORMINT_STATUS_EN(host, value) M_NEX_WRITE_WORD(value, host->token + HOST_NORMINT_STATUSEN)

#define M_WRITE_TIMEOUT_CTRL(host, value)      M_NEX_WRITE_BYTE(value, host->token + HOST_TIMEOUT_CONTROL)

#define M_WRITE_DMA_ADDRESS(host, value)       M_NEX_WRITE_WORD(value, host->token + HOST_DMASTART)

#define M_WRITE_ADMA_ADDRESS(host, value)      M_NEX_WRITE_WORD(value,host->token + HOST_ADMASTART)

#define M_WRITE_HOST_BLOCKSIZE(host, value)    M_NEX_WRITE_HALF_WORD(value, host->token + HOST_BLKSIZE)

#define M_WRITE_HOST_BLOCKCOUNT(host, value)   M_NEX_WRITE_HALF_WORD(value, host->token + HOST_BLKCOUNT)

#define M_WRITE_HOST_BUFFER(host, value)       M_NEX_WRITE_WORD(value, (host->token) + NEX_HOST_SD_BUFFER)

#define M_WRITE_BOOT_TIMEOUT(host, value)      M_NEX_WRITE_WORD(value, (host->token) + HOST_BOOT_DATA_TIMEOUT)

#define M_WRITE_PRESENT_STATE_REG(host, value) M_NEX_WRITE_WORD(value, (host->token) + HOST_PRESENT_STATE)


// MACRO DEFINITION FOR COMMON OPERATIONS
#define NEX_MAKE_BLKSIZE(dma, blksize)        (((dma & 0x7) << 12) | (blksize & 0xFFF))

#define NEX_MAKE_VOLTAGE_CAPAB(data, val) \
	switch(data & (VDD_180 | VDD_300 | VDD_330)) \
	{ case VDD_180 : val = 180;   \
		               break;     \
		case VDD_300 : val = 300; \
		               break;     \
		case VDD_330 : val = 330; \
		               break;     \
													}

#define NEX_CHECKDMA_CAPAB(data)          (data & CAPAB_SDMA_SUPPORT)

#define NEX_CHECK_SUS_RES_CAPAB(data)     (data & CAPAB_SUSPEND_RESUME)

#define NEX_CHECKHI_SP_CAPAB(data)        (data & CAPAB_HI_SP_MASK)

#define NEX_MAKE_BLOCK_LENGTH_CAPAB(data) ((data & HOST_BLKLEN_MASK ) >> HOST_BLKLEN_SHIFT)

#define NEX_MAKE_BASE_CLOCK_FREQ_SD(data) ((data & HOST_BASECLOCK_MASK) >> HOST_BASECLOCK_SHIFT)

#define NEX_FREQ_UNIT(data)               (data & HOST_TIMEOUTCLOCK_UNIT)

#define NEX_MAKE_TIMEOUT_CLOCK_FREQ(data) (data & HOST_TIMEOUTCLOCK_MASK)


// Nex Host Register Structure
struct nex_host_register
{
	UINT32 dma_start_address;       // DMA Start address consists of system address
                                    // from where DMA transfer will occur.
    UINT16 block_size;              // Used to configure number of bytes in data block.

    UINT16 block_count;             // Block count used for transferring multiple block
                                    // transfer operation.
    UINT16 arg0;                    // Contains  SD Command Argument which is specified
                                    // in the bit39 -- bit8 of the command format in the
                                    // SD memory card physical layer.
    UINT16 arg1;                    // Argument 1.
    UINT32 arg2;                    // Argument 2.
    UINT16 transfer_mode;           // Transfer mode register is used to control the
                                    // operation of data transfer.
    UINT16 cmd;                     // Command Register Writing to the uppewr byte makes
                                    // to generate the SD command.
    UINT16 resp0;                   // Response 0-7 is used to store responses from
                                    // the SD Cards these are are ROC bits information
                                    // is to be extaracted from bit 00d - bit 127d.
    UINT16 resp1;                   // Response 1.
    UINT16 resp2;
    UINT16 resp3;
    UINT16 resp4;
    UINT16 resp5;
    UINT16 resp6;
    UINT16 resp7;
    UINT16 bufferdata_port0;        // Bufferdata data port 0 used to access internal
                                    // host buffer through this 32 bit data port register.
    UINT16 bufferdata_port1;        // Bufferdata data port 1
    UINT32 present_state;           // Present State Register is used to read the
                                    // present status of host controller using the
                                    // various fields of this register
    UCHAR hostcontrol_reg;          // The host controller register is 8 bit  and
                                    // is used to define for high speed data tarnsfer
                                    // data width like 4 bit tx'fer or 1 bit tx'fer,
                                    // also has led control mostly used for debug or
                                    // warning purpose only.
    UCHAR power_control;            // This byte is typically used for voltage level controland switching host controller to off state.
    UCHAR blockgap_control;         // Block gap control register is used during block transfer operation
    UCHAR wakeup_control;           // Used to wake up the host controller but its functioning will depend upon the hardware support.
    UINT16 clock_control;           // Used to control the clock frequency. It has to set particular which the user driver will request for

	//Description will be added as per usage
    UCHAR timeout_control;
    UCHAR software_reset;
    UINT16 normalinterrupt_status;
    UINT16 errorinterrupt_status;
    UINT16 normalinterrupt_status_enable;
    UINT16 errorinterrupt_status_enable;
    UINT16 normalinterrupt_signal_enable;
    UINT16 errorinterrupt_signal_enable;
    UINT16 autocmd12_error_status;
    UINT32 capabilities;
    UINT16 max_current_capablities;
    UINT16 slot_int_status;
    UINT16 host_controller_version;
};

// Struct NEX BOARD
struct nex_host;
struct nex_board
{
    INT32 num_slots;                // Holds the number of slots the pci will support
    struct nex_host *nex_host[0];   // Holds element of struct nex_host
};

// Struct NEX HOST
struct nex_host
{
	struct nex_board *nex_board;
	UINT32 token;                   // This will hold the address of iomapped region
	TASKLET nex_taskcard;
	TASKLET finish_card_interrupt;
	TASKLET finish_end;
	TASKLET poll_resp;
	TASKLET stoptransmit;
	SCATTERLIST *current_sg;        // Currnt sg on which operation to be performed
	TIMER_LIST 	timer;
	TIMER_LIST 	retuning_timer;
	COMPLETION 	comp;
	COMPLETION 	inf_comp;
	MUTEXLOCK 	lock;
	
	DMA_ADDR_T dma_addr;
	struct sd_hw_host_ops *ops;
	struct sd_host *sdhost;
	struct sd_card_command *cmd;
	struct sd_card_command *cmd13;
	struct sd_card_data    *data;
	struct sd_card_request *stop_request;
	struct sd_card_request *mrq;
	struct emmc_boot_data  *edata;
	INT32 max_clock;                // Maximum clock frequency host supports
	INT32 clock;
	INT32 voltage;
	INT32 size;                     // This will store the number of bytes left to be transfered. During non DMA transfer.
	INT32 left_sg;                  // Total sg list left to be	transferred
	INT32 offset;                   // offset in the current sg.
	INT32 remain;                   // remaining data left for operation
	INT32 dma_size;                 // Number of bytes transferred in last dma operation
	UINT32 irq;
	INT32 intr;
	INT32 retuning_req;             // retuning expiration flag. set to 1 when the timer in mode1 expires
	INT32 retuning_timer_count;     // retuning timer value in seconds
	INT32 retuning_mode;            // 0 - for mode1, 1 - for mode2, 2 - for mode3
	INT32 retuning;                 // 1 when retuning is enabled
	INT32 retuning_inprogress;
	INT32 stop_at_blk;
	INT32 blk_transferred;
	INT32 error_mask;
	INT32 io_volt;
    INT32 slot;                     // The slot it represents
	INT32 start_addr;
	INT32 end_addr;
	INT32 length;
	INT32 temp;
	INT32 timeout_clock;
    INT32 card_inserted;            // added for SD3.0
	INT32 max_blklen;
	INT32 sdhcNum;		            //The sd host controller number it represents
	UINT16 power;
	CHAR nex_desc[20];
	CHAR *host_buffer;              // host buffer to hold the data in case of PIO
	UCHAR flags;
	UCHAR uhs_mode;
	UCHAR infinite_transfer;
	UCHAR wait_for_tx_intr;
	UCHAR error_rec;
	UCHAR pcmd;
	UCHAR early_intr;
};

#define NEX_ADMA2_TRANSFER        0x0021
#define NEX_ADMA2_END             0x0022

struct nex_adma2_table_row
{
	INT32 length_flags;
	INT32 address;
};

struct nex_adma2_desc_table
{
	struct nex_adma2_table_row *desc;
  	DMA_ADDR_T dma_addr;
  	INT32 entries;
  	INT32 dma_size;

};

VOID nex_request (struct sd_host* sdc, struct sd_card_request *req);
extern VOID nex_hw_set_ios (struct sd_host *host, struct sd_ios *ios);
INT32 nex_get_ro(struct sd_host *host);
extern INT32  nex_host_reset (struct nex_board *board, UCHAR res, INT32 slot);
VOID  nex_init(struct nex_host* host);
extern VOID nex_bus_dumpregs(struct nex_board *board, INT32 slot);
extern VOID nexhostled_on(struct nex_board *board, INT32 slot);
extern VOID nexhostled_off(struct nex_board *board, INT32 slot);
extern VOID nex_hw_set_power(struct nex_host *host, UINT16 voltage);
extern VOID nex_hw_set_clock(struct nex_host *host, INT32 clock);
extern VOID nex_send_command(struct nex_host *nexhost);
extern VOID nex_hw_enable_card_int(struct nex_host *host);
INT32 get_ro(struct sd_host *host);
VOID sd_set_high_speed_on_host(struct sd_host * host);
VOID sd_set_full_speed_on_host(struct sd_host * host);
VOID sd_set_bus_width8_on_host(struct sd_host * host);
VOID sdio_set_bus_width_on_host(struct sd_host *host,INT32 bus_width);
VOID sdio_set_blocklen_on_host(struct sd_host * host,INT32  blocklen);
VOID sdio_set_driver_strength_on_host(struct sd_host * host, INT32 drive_strength );
VOID stop_transmission(struct nex_host *nexhost);
#if  SOFTWARE_LOOPBACK
INT32 nex_error_codes(struct nex_host *host, struct sd_card_command *cmd);
#endif
INT32 mmc_set_hw_reset(struct sd_host * host);
VOID sd_mmc_set_ddr(struct sd_host * host);

VOID nex_hw_start_cmd(struct nex_host *nexhost, struct sd_card_command *cmd);
VOID nex_hw_set_transfer_mode(struct nex_host *host, struct sd_card_data *data);
VOID nex_hw_prepare_data(struct nex_host *host, struct sd_card_data *data);
INT32 nex_hw_cmd_done(struct nex_host *nexhost);
VOID nex_finish_data(struct nex_host *nexhost);
VOID host_read_capablity(struct nex_host *host);
VOID nex_transfer_pio(struct nex_host *host);
VOID nex_dump_host_registers(struct nex_host *host, struct sd_card_command *cmd);
VOID nex_read_blk_pio(struct nex_host *host);
VOID nex_write_blk_pio(struct nex_host *host);
VOID emmc_boot_sequence(struct nex_host* host);
INT32 nex_next_sg(struct nex_host* host);
INT32 nex_setup_slot(INT32 sdhc,INT32 slot);
VOID nex_down_slot(INT32 sdhc,INT32 slot);
INT32 Nex_HwInit(VOID);
VOID Nex_HwDeInit(VOID);

#endif
