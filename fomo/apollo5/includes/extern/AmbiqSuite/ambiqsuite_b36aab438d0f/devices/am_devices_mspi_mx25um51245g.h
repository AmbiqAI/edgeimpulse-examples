//*****************************************************************************
//
//! @file am_devices_mspi_mx25um51245g.h
//!
//! @brief General Multibit SPI mx25um51245g driver.
//!
//! @addtogroup mspi_mx25um51245g mx25um51245g MSPI Driver
//! @ingroup devices
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_DEVICES_MSPI_MX25UM51245G_H
#define AM_DEVICES_MSPI_MX25UM51245G_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Global definitions for mx25um51245g commands
//! @{
//
//*****************************************************************************

//
// SPI command set
//

// 1-byte command 3-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_READ3B             0x03    // Normal read
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_FREAD3B            0x0B    // Fast read data, 8 dummy cycles after address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_PP3B               0x02    // Page program 1-256 data cycles
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_SE3B               0x20    // Sector erase
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_BE3B               0xD8    // Block erase
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_CE                 0x60    // Chip erase, 0x60 or 0xC7? according to datasheet

// 1-byte command 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_READ4B             0x13    // Normal read
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_FREAD4B            0x0C    // Fast read data, 8 dummy cycles after address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_PP4B               0x12    // Page program 1-256 data cycles
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_SE4B               0x21    // Sector erase
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_BE4B               0xDC    // Block erase

// Setting Commands
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WREN               0x06    // Write enable
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WRDI               0x04    // Write disable
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_PAUSE_PGM_ERS      0xB0    // Suspends Program/Erase
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RESUME_PGM_ERS     0x30    // Resumes Program/Erase
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_DP                 0xB9    // Deep power down
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDP                0xAB    // Release from deep power down
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_SBL                0xC0    // Set Burst Length
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_ENSO               0xB1    // Enter secured OTP
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_EXSO               0xC1    // Exit secured OTP

// Reset Commands
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_NOP                0x00    // No operation
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RSTEN              0x66    // Reset Enable
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RST                0x99    // Reset Memory

// Register Commands
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDID               0x9F    // Read identification, outputs 1-byte manufacturer ID & 2-byte device ID
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDSFDP             0x5A    // Read SFDP mode, 3-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDSR               0x05    // Read status register
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDCR               0x15    // Read configuration register
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WRSR               0x01    // write status/configuration register
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDCR2              0x71    // Read configuration register 2, 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WRCR2              0x72    // Write configuration register 2, 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDFBR              0x16    // Read fast boot register
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WRFBR              0x17    // Write fast boot register
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_ESFBR              0x18    // erase fast boot register
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDSCUR             0x2B    // Read security register
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WRSCUR             0x2F    // Write security register
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WRLR               0x2C    // Write lock register
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDLR               0x2D    // Read lock register
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WRSPB              0xE3    // Program SPB bit, 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_ESSPB              0xE4    // Erase SPB bit
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDSPB              0xE2    // Read SPB status, 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WRDPB              0xE1    // Write DPB register, 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDDPB              0xE0    // Read DPB register, 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WPSEL              0x68    // Write protect selection to enter and enable individal block protect mode
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_GBLK               0x7E    // Whole chip write protect
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_GBULK              0x98    // Whole chip unprotect
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_RDPASS             0x27    // Read password register, 4-byte address(0x00000000), 8 dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_WRPASS             0x28    // Write password register, 4-byte address(0x00000000)
#define AM_DEVICES_MSPI_MX25UM51245G_SPI_PASSULK            0x29    // Password unlock, 4-byte address(0x00000000)

//
// OPI command set
//

// 2-byte command, 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_8READ              0xEC13  // Octal STR read, frequency based dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_8DTRD              0xEE11  // Octal DTR read, frequency based dummy cycles, even address
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDID               0x9F60  // Read identification, address(0x00000000), 4 dummy cycles, data output in STR
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDSFDP             0x5AA5  // Read SFDP mode, 20 dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_PP                 0x12ED  // Page porgram, 1-256 data cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_SE                 0x21DE  // Sector erase
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_BE                 0xDC23  // Block erase
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_CE                 0x609F  // Chip erase, 0x609F or 0xC738

// Setting Commands
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WREN               0x06F9  // Write enable
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WRDI               0x04FB  // Write disable
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_PAUSE_PGM_ERS      0xB04F  // Suspends Program/Erase
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RESUME_PGM_ERS     0x30CF  // Resumes Program/Erase
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_DP                 0xB946  // Deep power down
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDP                0xAB54  // Release from deep power down
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_SBL                0xC03F  // Set Burst Length, 4-byte address(0x00000000)
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_ENSO               0xB14E  // Enter secured OTP
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_EXSO               0xC13E  // Exit secured OTP

// Reset Commands
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_NOP                0x00FF  // No operation
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RSTEN              0x6699  // Reset Enable
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RST                0x9966  // Reset Memory

#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDSR               0x05FA  // Read status register, 4-byte address(0x00000000), 4 dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDCR               0x15EA  // Read configuration register, 4-byte address(0x00000001), 4 dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WRSR               0x01FE  // write status register, 4-byte address(0x00000000)
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WRCR               0x01FE  // write configuration register, 4-byte address(0x00000001)
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDCR2              0x718E  // Read configuration register 2, 4-byte address, 4 dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WRCR2              0x728D  // Write configuration register 2, 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDFBR              0x16E9  // Read fast boot register, 4-byte address(0x00000000), 4 dummy cycles, data output in STR
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WRFBR              0x17E8  // Write fast boot register 4-byte address(0x00000000)
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_ESFBR              0x18E7  // erase fast boot register
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDSCUR             0x2BD4  // Read security register 4-byte address(0x00000000), 4 dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WRSCUR             0x2FD0  // Write security register
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WRLR               0x2CD3  // Write lock register
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDLR               0x2DD2  // Read lock register, 4-byte address(0x00000000), 4 dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WRSPB              0xE31C  // Program SPB bit, 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_ESSPB              0xE41B  // Erase SPB bit
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDSPB              0xE21D  // Read SPB status, 4-byte address, frequency based dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WRDPB              0xE11E  // Write DPB register, 4-byte address
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDDPB              0xE01F  // Read DPB register, 4-byte address, frequency based dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WPSEL              0x6897  // Write protect selection to enter and enable individal block protect mode
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_GBLK               0x7E81  // Whole chip write protect
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_GBULK              0x9867  // Whole chip unprotect
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_RDPASS             0x27D8  // Read password register, 4-byte address(0x00000000), 20 dummy cycles
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_WRPASS             0x28D7  // Write password register, 4-byte address(0x00000000)
#define AM_DEVICES_MSPI_MX25UM51245G_OPI_PASSULK            0x29D6  // Password unlock, 4-byte address(0x00000000)

//user may choose to add more based on list above
typedef enum
{
    AM_DEVICES_MSPI_MX25UM51245G_CMD_WREN,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_WRDI,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_PAUSE_PGM_ERS,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_RESUME_PGM_ERS,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_DP,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_RDP,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_SBL,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_ENSO,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_EXSO,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_RSTEN,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_RST,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_RDSR,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_RDCR,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_WRSR,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_WRCR,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_RDCR2,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_WRCR2,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_PP,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_SE,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_BE,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_CE,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_RDID,
    AM_DEVICES_MSPI_MX25UM51245G_CMD_MAXNUM,
}am_devices_mspi_mx25um51245g_cmd_e;

typedef struct
{
    uint16_t    ui16Cmd;
    uint32_t    ui32Addr;
    bool        bSendAddr;
    uint32_t    *pData;
    uint32_t    ui32NumBytes;
    bool        bTurnaround;
    uint8_t     ui8Turnaround;
}am_devices_mspi_mx25um51245g_cmd_t;

//! @}

//*****************************************************************************
//
//! @name Global definitions for the mx25um51245g registers
//! @{
//
//*****************************************************************************
typedef struct
{
    union
    {
        uint8_t SR;                                 //Status register

        struct
        {
            uint8_t WIP             : 1;            //[0..0] write in progress bit.        |       1=write operation, 0=not in write operation                  |       volatile
            uint8_t WEL             : 1;            //[1..1] write enable latch.           |       1=write enable, 0=not write enable                           |       volatile
            uint8_t BP              : 4;            //[2..5] level of protected block      |       0=none, 1~10=protected 2^(n) blocks                          |       non-volatile
                                                    //                                     |       from top or bottom based on TB bit, 11~15= protected all     |
            uint8_t                 : 2;
        }SR_b;
    } ;

    union
    {
        uint8_t CR;                                 //Configuration register

        struct
        {
            uint8_t ODS             : 3;            //[0..2] output driver strength.       |       0x7~0x=24,26,30,34,41,52,76,146 Ohms. Default=0x7            |       volatile
            uint8_t TB              : 1;            //[3..3] top/bottom selected.          |       0=Top area protect, 1=Bottom area protect. Default=0         |       OTP
            uint8_t PBE             : 1;            //[4..4] Preamble bit enable.          |       0=Disable, 1=Enable                                          |       volatile
            uint8_t                 : 3;
        }CR_b;
    } ;

    union
    {
        uint8_t CR2_00000000;                       //SPI, DOPI, SOPI

        struct
        {
            uint8_t MODE            : 2;            //[0..1] STR DTR OPI mode selection.   |       0x0=SPI, 0x1=STR OPI, 0x2=DTR OPI, 0x3=inhibit               |       volatile
            uint8_t                 : 6;
        }CR2_00000000_b;
    } ;

    union
    {
        uint8_t CR2_00000200;                       //DQS

        struct
        {
            uint8_t DQSPRC          : 1;            //[0..0] DTR DQS pre-cycle.            |       0=1 cycle, 1=0 cycle(seems to be a mistake in datasheet)     |       volatile
            uint8_t DOS             : 1;            //[1..1] DQS on STR mode.              |       0=Disable, 1=Enable                                          |       volatile
            uint8_t                 : 6;
        }CR2_00000200_b;
    } ;

    union
    {
        uint8_t CR2_00000300;                       //Dummy cycle

        struct
        {
            uint8_t DCn             : 3;            //[0..2] Dummy cycle.                  |       000 = 20 cycles BGA-200MHz SOP 133MHz                        |       volatile
                                                    //                                     |       001 = 18 cycles BGA-200MHz SOP 133MHz
                                                    //                                     |       010 = 16 cycles BGA-173MHz SOP 133MHz
                                                    //                                     |       011 = 14 cycles BGA-155MHz SOP 133MHz
                                                    //                                     |       100 = 12 cycles BGA-139MHz SOP 104MHz
                                                    //                                     |       101 = 10 cycles BGA-121MHz SOP 104MHz
                                                    //                                     |       110 =  8 cycles BGA- 86MHz SOP  84MHz
                                                    //                                     |       111 =  6 cycles BGA- 70MHz SOP  66MHz
            uint8_t                 : 5;
        }CR2_00000300_b;
    } ;

    union
    {
        uint8_t CR2_00000400;                       //ECS

        struct
        {
            uint8_t ECS             : 2;            //[0..1] ECS# pin goes low define      |       00 = 2 bit error or double programmed                        |       volatile
                                                    //                                     |       01 = 1 or 2 bit error or double programmed
                                                    //                                     |       10 = 2 bit error only
                                                    //                                     |       11 = 1 or 2 bit error
            uint8_t                 : 6;
        }CR2_00000400_b;
    } ;

    union
    {
        uint8_t CR2_00000500;                       //CRC

        struct
        {
            uint8_t PPTSEL          : 1;            //[0..0] Preamble pattern selection.   |                                                                    |       volatile
            uint8_t                 : 3;
            uint8_t CRCBEN          : 1;            //[3..3] CRC# output enable.           |       0=CRC# output Disable, 1=CRC# output Enable                  |       volatile
            uint8_t CRC_CYC         : 2;            //[4..4] CRC chunk size configuration. |       0x0=16byte, 0x1=32byte, 0x2=64byte, 0x3=128byte              |       volatile
            uint8_t                 : 1;
        }CR2_00000500_b;
    } ;

    union
    {
        uint8_t CR2_00000800;                       //ECC

        struct
        {
            uint8_t ECCCNT          : 4;            //[0..3] ECC failure chunk counter.    |                                                                    |       volatile
            uint8_t ECCFS           : 3;            //[4..6] ECC fail status.              |       0=None, xx1=1 bit corrected, x1x=2 bits detected,            |       volatile
                                                    //                                     |       1xx=Double programmed page detected                          |
            uint8_t ECCFAVLD        : 1;            //[7..7] ECC fail address valid.       |       0= ECC failure address invalid (no address recorded)         |       volatile
                                                    //                                     |       1= ECC failure address valid(there's fail address recorded)  |
        }CR2_00000800_b;
    } ;

    union
    {
        uint8_t CR2_00000C00;                       //ECCFA A7:A4

        struct
        {
            uint8_t                 : 4;
            uint8_t ECCFA_7_4       : 4;            //[4..7] ECC failure chunk address.    |       ECC 1st failure chunkaddress (A7:A4)                         |       volatile
        }CR2_00000C00_b;
    } ;

    union
    {
        uint8_t CR2_00000D00;                       //ECCFA A15:A8

        struct
        {
            uint8_t ECCFA_15_8      : 8;            //[0..7] ECC failure chunk address.    |       ECC 1st failure chunkaddress (A8:A15)                         |       volatile
        }CR2_00000D00_b;
    } ;

    union
    {
        uint8_t CR2_00000E00;                       //ECCFA A16:A23

        struct
        {
            uint8_t ECCFA_23_16     : 8;            //[0..7] ECC failure chunk address.    |       ECC 1st failure chunkaddress (A16:A23)                         |       volatile
        }CR2_00000E00_b;
    } ;

    union
    {
        uint8_t CR2_00000F00;                       //ECCFA A24:A25

        struct
        {
            uint8_t ECCFA_25_24     : 2;            //[0..1] ECC failure chunk address.    |       ECC 1st failure chunkaddress (A24:A25)                         |       volatile
            uint8_t                 : 6;
        }CR2_00000F00_b;
    } ;

    union
    {
        uint8_t CR2_40000000;                       //Default device mode

        struct
        {
            uint8_t DEFMODE         : 2;            //[0..1] default device mode           |       00=inhibit, 01=default DTR OPI,                                |       OTP
                                                    //                                     |       10=default STR OPI, 11=default SPI mdoe
            uint8_t                 : 1;
            uint8_t CRCEN           : 1;            //[3..3] Enable Parity checking        |       0=Parity check enable, 1=Parity check disable                  |       OTP
            uint8_t                 : 4;
        }CR2_40000000_b;
    } ;

    union
    {
        uint8_t CR2_80000000;                       //CRCERR

        struct
        {
            uint8_t                 : 4;
            uint8_t CRCERR          : 1;            //[4..4] CMD# or Parity checked fail   |       0=check pass, 1=check fail                                     |       volatile
            uint8_t                 : 3;
        }CR2_80000000_b;
    } ;

    union
    {
        uint8_t SECURITY_REG;                           //Security register

        struct
        {
            uint8_t SOTP            : 1;            //[0..0] Secured OTP indicator         |       0=non-factory lock, 1=factory lock                             |       non-volatile
            uint8_t LDSO            : 1;            //[1..1] If lock-down                  |       0=not lock-down, 1=lock-down(cannot program/erase OTP)         |       non-volatile(OTP)
            uint8_t PSB             : 1;            //[2..2] Program suspend bit           |       0=program not suspended, 1=program suspended                   |       volatile
            uint8_t ESB             : 1;            //[3..3] Erase suspend bit             |       0=erase not suspended, 1=erase suspended                       |       volatile
            uint8_t                 : 1;
            uint8_t P_FAIL          : 1;            //[5..5] Program success/fail indicator|       0=program succeed, 1=program failed                            |       volatile
            uint8_t E_FAIL          : 1;            //[6..6] Erase success/fail indicator  |       0=erase succeed, 1=erase failed                                |       volatile
            uint8_t WPSEL           : 1;            //[7..7] Write protection selection    |       0=normal WP mode, 1=individual mode                            |       non-volatile(OTP)
        }SECURITY_REG_b;
    } ;

    union
    {
        uint8_t LOCK;                               //Lock register

        struct
        {
            uint8_t                 : 2;
            uint8_t PPML            : 1;            //[2..2] password protection mode      |       0=password protection mode enable, 1=solid protection mode     |       OTP
            uint8_t                 : 3;
            uint8_t SPBLKDN         : 1;            //SPB lock down                        |       0=SPB bit protected, 1=SPB bit unprotected                     |       volatile/OTP based on PPML bit
            uint8_t                 : 1;
        }LOCK_b;
    } ;

}am_devices_mspi_mx25um51245g_regs_t;

#define AM_DEVICES_MSPI_MX25UM51245G_WEL         0x02        // Write enable latch
#define AM_DEVICES_MSPI_MX25UM51245G_WIP         0x01        // Write in progress

typedef enum
{
    AM_DEVICES_MSPI_MX25UM51245G_ODS_146_Ohms,
    AM_DEVICES_MSPI_MX25UM51245G_ODS_76_Ohms,
    AM_DEVICES_MSPI_MX25UM51245G_ODS_52_Ohms,
    AM_DEVICES_MSPI_MX25UM51245G_ODS_41_Ohms,
    AM_DEVICES_MSPI_MX25UM51245G_ODS_34_Ohms,
    AM_DEVICES_MSPI_MX25UM51245G_ODS_30_Ohms,
    AM_DEVICES_MSPI_MX25UM51245G_ODS_26_Ohms,
    AM_DEVICES_MSPI_MX25UM51245G_ODS_24_Ohms,
}am_devices_mspi_mx25um51245g_ODS_e;

typedef enum
{
    AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI,
    AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI,
    AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI,
}am_devices_mspi_mx25um51245g_MODE_e;

typedef enum
{
    AM_DEVICES_MSPI_MX25UM51245G_DEFMODE_INVALID,
    AM_DEVICES_MSPI_MX25UM51245G_DEFMODE_DOPI,
    AM_DEVICES_MSPI_MX25UM51245G_DEFMODE_SOPI,
    AM_DEVICES_MSPI_MX25UM51245G_DEFMODE_SPI,
}am_devices_mspi_mx25um51245g_DEFMODE_e;

typedef enum
{
    AM_DEVICES_MSPI_MX25UM51245G_DC_20,
    AM_DEVICES_MSPI_MX25UM51245G_DC_18,
    AM_DEVICES_MSPI_MX25UM51245G_DC_16,
    AM_DEVICES_MSPI_MX25UM51245G_DC_14,
    AM_DEVICES_MSPI_MX25UM51245G_DC_12,
    AM_DEVICES_MSPI_MX25UM51245G_DC_10,
    AM_DEVICES_MSPI_MX25UM51245G_DC_8,
    AM_DEVICES_MSPI_MX25UM51245G_DC_6,
}am_devices_mspi_mx25um51245g_DC_e;

typedef enum
{
    AM_DEVICES_MSPI_MX25UM51245G_BGA,
    AM_DEVICES_MSPI_MX25UM51245G_SOP,
}am_devices_mspi_mx25um51245g_pkg_e;

typedef enum
{
    AM_DEVICES_MSPI_MX25UM51245G_BURST_INVALID,
    AM_DEVICES_MSPI_MX25UM51245G_BURST_16B,
    AM_DEVICES_MSPI_MX25UM51245G_BURST_32B,
    AM_DEVICES_MSPI_MX25UM51245G_BURST_64B,
    AM_DEVICES_MSPI_MX25UM51245G_BURST_NONE,
}am_devices_mspi_mx25um51245g_burst_len_e;

//! @}

//*****************************************************************************
//
//! @name Device specific identification.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_MX25UM51245G_ID        0x003A80C2
#define AM_DEVICES_MSPI_MX25UM51245G_ID_MASK   0x00FFFFFF
//! @}

//*****************************************************************************
//
//! @name Device specific definitions for the mx25um51245g size information
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE       0x100    //256 bytes, minimum program unit
#define AM_DEVICES_MSPI_MX25UM51245G_SECTOR_SIZE     0x1000   //4K bytes
#define AM_DEVICES_MSPI_MX25UM51245G_BLOCK_SIZE      0x10000  //64K bytes.
#define AM_DEVICES_MSPI_MX25UM51245G_MAX_SECTORS     16*1024
#define AM_DEVICES_MSPI_MX25UM51245G_MAX_BLOCKS      1024
//! @}

//*****************************************************************************
//
//! @name Global definitions for the maximum device number supported
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_MX25UM51245G_MAX_DEVICE_NUM    1
//! @}

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS,
    AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR
} am_devices_mspi_mx25um51245g_status_t;

typedef struct
{
    uint32_t ui32DeviceNum;
    am_hal_mspi_device_e eDeviceConfig;
    am_hal_mspi_clock_e eClockFreq;
    bool     bDQSEnable;                //only enable/disable DQS in STR mode

    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
    uint32_t ui32ScramblingStartAddr;
    uint32_t ui32ScramblingEndAddr;
} am_devices_mspi_mx25um51245g_config_t;

typedef struct
{
    bool            bTxNeg;
    bool            bRxNeg;
    bool            bRxCap;
    uint8_t         ui8TxDQSDelay;
    uint8_t         ui8RxDQSDelay;
    uint8_t         ui8Turnaround;
} am_devices_mspi_mx25um51245g_timing_config_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the mspi_mx25um51245g driver.
//!
//! @param ui32Module
//! @param psMSPISettings
//! @param ppHandle
//! @param ppMspiHandle
//!
//! This function should be called before any other am_devices_spiflash
//! functions. It is used to set tell the other functions how to communicate
//! with the external spiflash hardware.
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_init(uint32_t ui32Module,
                                           am_devices_mspi_mx25um51245g_config_t *psMSPISettings,
                                           void **ppHandle, void **ppMspiHandle);

//*****************************************************************************
//
//! @brief De-Initialization the mspi_mx25um51245g driver.
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function should be called before any other am_devices_spiflash
//! functions. It is used to set tell the other functions how to communicate
//! with the external spiflash hardware.
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_deinit(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the ID of the external mx25um51245g and returns the value.
//!
//! @param pHandle - Pointer to driver handle.
//!
//! This function reads the device ID register of the external mx25um51245g, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_id(void *pHandle);

//*****************************************************************************
//
//! @brief Reset the external mx25um51245g
//!
//! @param pHandle - Pointer to driver handle.
//!
//! This function reset the external mx25um51245g, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_reset(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the contents of the external mx25um51245g into a buffer.
//!
//! @param pHandle         - Pointer to driver handle
//! @param pui8RxBuffer    - Data is saved here
//! @param ui32ReadAddress - Address of desired data in external mx25um51245g
//! @param ui32NumBytes    - Number of bytes to read from external mx25um51245g
//! @param bWaitForCompletion
//!
//! This function reads the external mx25um51245g at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_read(void *pHandle,
                                                  uint8_t *pui8RxBuffer,
                                                  uint32_t ui32ReadAddress,
                                                  uint32_t ui32NumBytes,
                                                  bool bWaitForCompletion);

//*****************************************************************************
//
//! @brief Programs the given range of mx25um51245g addresses.
//!
//! @param pHandle           - Pointer to driver handle
//! @param pui8TxBuffer      - Buffer to write the external mx25um51245g data from
//! @param ui32WriteAddress  - Address to write to in the external mx25um51245g
//! @param ui32NumBytes      - Number of bytes to write to the external mx25um51245g
//! @param bWaitForCompletion
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external mx25um51245g at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target mx25um51245g
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_write(void *pHandle,
                                                   uint8_t *pui8TxBuffer,
                                                   uint32_t ui32WriteAddress,
                                                   uint32_t ui32NumBytes,
                                                   bool bWaitForCompletion);

//*****************************************************************************
//
//! @brief Erases the entire contents of the external mx25um51245g
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function uses the "Chip Erase" instruction to erase the entire
//! contents of the external mx25um51245g.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_chip_erase(void *pHandle);

//*****************************************************************************
//
//! @brief Erases the contents of a single sector of mx25um51245g
//!
//! @param pHandle           - Pointer to driver handle
//! @param ui32SectorAddress - Address to erase in the external mx25um51245g
//!
//! This function erases a single sector of the external mx25um51245g as specified by
//! ui32EraseAddress. The entire sector where ui32EraseAddress will
//! be erased.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_sector_erase(void *pHandle, uint32_t ui32SectorAddress);

//*****************************************************************************
//
//! @brief Sets up the MSPI and external mx25um51245g into XIP mode.
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function sets the external psram device and the MSPI into XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_enable_xip(void *pHandle);

//*****************************************************************************
//
//! @brief Removes the MSPI and external mx25um51245g from XIP mode.
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function removes the external device and the MSPI from XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_disable_xip(void *pHandle);

//*****************************************************************************
//
//! @brief Sets up the MSPI and external mx25um51245g into scrambling mode.
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function sets the external psram device and the MSPI into scrambling mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_enable_scrambling(void *pHandle);

//*****************************************************************************
//
//! @brief Removes the MSPI and external mx25um51245g from scrambling mode.
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function removes the external device and the MSPI from scrambling mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_disable_scrambling(void *pHandle);

//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param pui8RxBuffer
//! @param ui32ReadAddress
//! @param ui32NumBytes
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback
//! @param pCallbackCtxt
//!
//! @return
//
//*****************************************************************************
extern uint32_t
am_devices_mspi_mx25um51245g_read_adv(void *pHandle,
                                      uint8_t *pui8RxBuffer,
                                      uint32_t ui32ReadAddress,
                                      uint32_t ui32NumBytes,
                                      uint32_t ui32PauseCondition,
                                      uint32_t ui32StatusSetClr,
                                      am_hal_mspi_callback_t pfnCallback,
                                      void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Reads the contents of the external mx25um51245g into a buffer.
//!
//! @param pHandle            - Pointer to driver handle
//! @param pui8RxBuffer       - Buffer to store the received data from the mx25um51245g
//! @param ui32ReadAddress    - Address of desired data in external mx25um51245g
//! @param ui32NumBytes       - Number of bytes to read from external mx25um51245g
//! @param bWaitForCompletion
//!
//! This function reads the external mx25um51245g at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t
am_devices_mspi_mx25um51245g_read_hiprio(void *pHandle,
                                         uint8_t *pui8RxBuffer,
                                         uint32_t ui32ReadAddress,
                                         uint32_t ui32NumBytes,
                                         bool bWaitForCompletion);

//*****************************************************************************
//
//! @brief Running timing scan for the external mx25um51245g.
//!
//! @param ui32module    - MSPI module number
//! @param pDevCfg       - MSPI setting for mx25um51245g
//! @param pDevSdrCfg    - MSPI timing scan result for mx25um51245g
//!
//! This function conducts timing scan on the external mx25um51245g based on
//! provided MSPI settings and return valid timing scan results if successful
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t
am_devices_mspi_mx25um51245g_init_timing_check(uint32_t ui32module,
                                               am_devices_mspi_mx25um51245g_config_t *pDevCfg,
                                               am_devices_mspi_mx25um51245g_timing_config_t *pDevSdrCfg);

//*****************************************************************************
//
//! @brief Apply timing scan parameters for the external mx25um51245g.
//!
//! @param pHandle       - mx25um51245g device handle
//! @param pDevSdrCfg    - MSPI timing scan parameters for mx25um51245g
//!
//! This function apply the provided timing scan parameters to MSPI
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t
am_devices_mspi_mx25um51245g_apply_timing(void *pHandle,
                                          am_devices_mspi_mx25um51245g_timing_config_t *pDevSdrCfg);
// ##### INTERNAL BEGIN #####
//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param pui8RxBuffer
//! @param ui32ReadAddress
//! @param ui32NumBytes
//!
//! @return staus
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25um51245g_read_callback(void *pHandle,
                                                           uint8_t *pui8RxBuffer,
                                                           uint32_t ui32ReadAddress,
                                                           uint32_t ui32NumBytes);
// ##### INTERNAL END #####

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_MSPI_MX25UM51245G_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
