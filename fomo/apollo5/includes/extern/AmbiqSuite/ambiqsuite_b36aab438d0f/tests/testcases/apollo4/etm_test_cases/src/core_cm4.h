/*
 *-----------------------------------------------------------------------------
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from ARM Limited.
 *
 *            (C) COPYRIGHT 2009-2010 ARM Limited.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from ARM Limited.
 *
 *      SVN Information
 *
 *      Revision            : $Revision: 142008 $
 *      Release information : cortexm4_r0p1_00rel0
 *-----------------------------------------------------------------------------
 */
#ifndef __CM4_CORE_H__
#define __CM4_CORE_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#define __CM4_CMSIS_VERSION_MAIN  (0x01)                                                       /*!< [31:16] CMSIS HAL main version */
#define __CM4_CMSIS_VERSION_SUB   (0x20)                                                       /*!< [15:0]  CMSIS HAL sub version  */
#define __CM4_CMSIS_VERSION       ((__CM4_CMSIS_VERSION_MAIN << 16) | __CM4_CMSIS_VERSION_SUB) /*!< CMSIS HAL version number       */

#define __CORTEX_M                (0x04)                                                       /*!< Cortex core                    */

/**
 *  Lint configuration \n
 *  ----------------------- \n
 *
 *  The following Lint messages will be suppressed and not shown: \n
 *  \n
 *    --- Error 10: --- \n
 *    register uint32_t __regBasePri         __asm("basepri"); \n
 *    Error 10: Expecting ';' \n
 *     \n
 *    --- Error 530: --- \n
 *    return(__regBasePri); \n
 *    Warning 530: Symbol '__regBasePri' (line 264) not initialized \n
 *     \n
 *    --- Error 550: --- \n
 *      __regBasePri = (basePri & 0x1ff); \n
 *    } \n
 *    Warning 550: Symbol '__regBasePri' (line 271) not accessed \n
 *     \n
 *    --- Error 754: --- \n
 *    uint32_t RESERVED0[24]; \n
 *    Info 754: local structure member '<some, not used in the HAL>' (line 109, file ./cm3_core.h) not referenced \n
 *     \n
 *    --- Error 750: --- \n
 *    #define __CM4_CORE_H__ \n
 *    Info 750: local macro '__CM4_CORE_H__' (line 43, file./cm3_core.h) not referenced \n
 *     \n
 *    --- Error 528: --- \n
 *    static __INLINE void NVIC_DisableIRQ(uint32_t IRQn) \n
 *    Warning 528: Symbol 'NVIC_DisableIRQ(unsigned int)' (line 419, file ./cm3_core.h) not referenced \n
 *     \n
 *    --- Error 751: --- \n
 *    } InterruptType_Type; \n
 *    Info 751: local typedef 'InterruptType_Type' (line 170, file ./cm3_core.h) not referenced \n
 * \n
 * \n
 *    Note:  To re-enable a Message, insert a space before 'lint' * \n
 *
 */

/*lint -save */
/*lint -e10  */
/*lint -e530 */
/*lint -e550 */
/*lint -e754 */
/*lint -e750 */
/*lint -e528 */
/*lint -e751 */


#include <stdint.h>                           /* Include standard types */

#if defined (__ICCARM__)
  #include <intrinsics.h>                     /* IAR Intrinsics   */
#endif


#ifndef __NVIC_PRIO_BITS
  #define __NVIC_PRIO_BITS    4               /*!< standard definition for NVIC Priority Bits */
#endif




/**
 * IO definitions
 *
 * define access restrictions to peripheral registers
 */

#ifdef __cplusplus
#define     __I     volatile                  /*!< defines 'read only' permissions      */
#else
#define     __I     volatile const            /*!< defines 'read only' permissions      */
#endif
#define     __O     volatile                  /*!< defines 'write only' permissions     */
#define     __IO    volatile                  /*!< defines 'read / write' permissions   */



/*******************************************************************************
 *                 Register Abstraction
 ******************************************************************************/


/* System Reset */
#define NVIC_VECTRESET              0         /*!< Vector Reset Bit             */
#define NVIC_SYSRESETREQ            2         /*!< System Reset Request         */
#define NVIC_AIRCR_VECTKEY    (0x5FA << 16)   /*!< AIRCR Key for write access   */
#define NVIC_AIRCR_ENDIANESS        15        /*!< Endianess                    */
#define NVIC_MEMFAULTENA          (1 << 16)   /*!< MEMFAULTENA bit              */

/*SCB */
#define SCB_CPACR_FPU           (0xF << 20)   /*!< cp10 and cp11 set in CPACR  */


/*MPU */
#define MPU_FULL_ACC_NON_CACHEABLE    (0x320 << 16)   /*!< Attribute registerin MPU   */
#define MPU_READ_ONLY_NON_CACHEABLE   (0x620 << 16)   /*!< Attribute registerin MPU   */
#define MPU_NO_ACC_SO                 (0x0000 << 16)  /*!< Attribute registerin MPU   */

#define MPU_SIZE_512MB                (0x1C << 1)  /*!< MPU region size configuration in MPU    */
#define MPU_SIZE_2GB                  (0x1E << 1)  /*!< MPU region size configuration in MPU    */
#define MPU_SIZE_32BYTES               (0x4 << 1)  /*!< MPU region size configuration in MPU    */


#define MMFSR_IACCVIOL                  (1 << 0)   /*!< Fault status register information   */
#define MMFSR_DACCVIOL                  (1 << 1)   /*!< Fault status register information   */
#define MMFSR_MUNSTKERR                 (1 << 3)   /*!< Fault status register information   */
#define MMFSR_MSTKERR                   (1 << 4)   /*!< Fault status register information   */
#define MMFSR_MLSPERR                   (1 << 5)   /*!< Fault status register information   */
#define MMFSR_MMARVALID                 (1 << 7)   /*!< Fault status register information   */

/* Core Debug */
#define CoreDebug_DEMCR_TRCENA  (1 << 24)     /*!< DEMCR TRCENA enable          */
#define ITM_TCR_BUSY            (1 << 23)     /*!< ITM Busy                     */
#define ITM_TCR_TBUSID          16            /*!< ITM TraceBusID offset        */
#define ITM_TCR_TS_GLOBAL_128   (0x01 << 10)  /*!< Timestamp every 128 cycles   */
#define ITM_TCR_TS_GLOBAL_8192  (0x10 << 10)  /*!< Timestamp every 8192 cycles  */
#define ITM_TCR_TS_GLOBAL_ALL   (0x11 << 10)  /*!< Timestamp all                */
#define ITM_TCR_SWOENA          (1 << 4)      /*!< ITM Enables asynchronous-specific usage model*/
#define ITM_TCR_TXENA           (1 << 3)      /*!< ITM Enable hardware event packet 
                                                    emission to the TPIU from the DWT */
#define ITM_TCR_SYNCENA         (1 << 2)      /*!< ITM Enable synchronization packet
                                                   transmission for a synchronous TPIU*/
#define ITM_TCR_TSENA           (1 << 1)      /*!< ITM Enable differential timestamps */
#define ITM_TCR_ITMENA          (1 << 0)      /*!< ITM enable                   */
#define ITM_TER_STIM0           (1 << 0)      /*!< ITM stimulus0 enable         */
#define ITM_TER_STIM1           (1 << 1)      /*!< ITM stimulus1 enable         */
#define ITM_TER_STIM2           (1 << 2)      /*!< ITM stimulus2 enable         */
   
#define DWT_CTRL_CYCEVTENA      (1 << 22)     /*!< DWT Periodic packet enable   */
#define DWT_CTRL_FOLDEVTENA     (1 << 21)     /*!< DWT fold event enable        */
#define DWT_CTRL_LSUEVTENA      (1 << 20)     /*!< DWT lsu event enable         */
#define DWT_CTRL_SLEEPEVTENA    (1 << 19)     /*!< DWT sleep event enabel       */
#define DWT_CTRL_EXCEVTENA      (1 << 18)     /*!< DWT exception event enable   */
#define DWT_CTRL_CPIEVTENA      (1 << 17)     /*!< DWT CPI event enable         */
#define DWT_CTRL_EXCTRCENA      (1 << 16)     /*!< DWT Exception trace enable   */
#define DWT_CTRL_PCSAMPLENA     (1 << 12)     /*!< DWT Periodic event PC select */
#define DWT_CTRL_SYNCTAP24      (1 << 10)     /*!< DWT Synch packet tap at 24   */
#define DWT_CTRL_SYNCTAP26      (2 << 10)     /*!< DWT Synch packet tap at 26   */
#define DWT_CTRL_SYNCTAP28      (3 << 10)     /*!< DWT Synch packet tap at 28   */
#define DWT_CTRL_CYCTAP         (1 << 9)      /*!< DWT POSTCNT tap select       */
#define DWT_CTRL_POSTPRESET_BITS     1        /*!< DWT POSTCNT reload offset    */
#define DWT_CTRL_CYCCNTENA           1        /*!< DWT Cycle counter enable     */

/* DWT Function, EMITRANGE=0, CYCMATCH = 0 */
#define DWT_FUNC_SAMP_PC        0x1           /*!< DWT Func: PC Sample Packet   */
#define DWT_FUNC_SAMP_DATA      0x2           /*!< DWT Func: Data Value Packet  */
#define DWT_FUNC_SAMP_PC_DATA   0x3           /*!< DWT Func: PC and Data Packets*/
#define DWT_FUNC_PC_WPT         0x4           /*!< DWT Func: PC Watchpoint      */
#define DWT_FUNC_TRIG_PC        0x8           /*!< DWT Func: PC to CMPMATCH(ETM)*/
#define DWT_FUNC_TRIG_RD        0x9           /*!< DWT Func: Daddr(R)to CMPMATCH*/
#define DWT_FUNC_TRIG_WR        0xA           /*!< DWT Func: Daddr(W)to CMPMATCH*/
#define DWT_FUNC_TRIG_RW        0xB           /*!< DWT Func: Daddr to CMPMATCH  */

#define DWT_CTRL_POSTPRESET_10  0xA
   
/* ETM */
#define ETM_CR_PWRDN                1         /*!< ETM Power Down                */
#define ETM_CR_STALLPROC        (1 << 7)      /*!< ETM Stall Processor           */
#define ETM_CR_BRANCH_OUTPUT    (1 << 8)      /*!< ETM Branch Broadcast          */
#define ETM_CR_DEBUG            (1 << 9)      /*!< ETM Debug Request control     */
#define ETM_CR_PROGBIT          (1 << 10)     /*!< ETM ProgBit                   */
#define ETM_CR_ETMEN            (1 << 11)     /*!< ETM ProgBit                   */
#define ETM_CR_TSEN             (1 << 28)     /*!< ETM Timestamp Enable          */
#define ETM_SR_OVERFLOW             1         /*!< ETM Overflow Status           */
#define ETM_SR_PROGBIT          (1 << 1)      /*!< ETM Progbit Status            */
#define ETM_SR_SSTOP            (1 << 2)      /*!< ETM Start/Stop Status         */
#define ETM_SR_TRIGGER          (1 << 3)      /*!< ETM Triggered Status          */
#define ETM_SCR_FIFOFULL        (1 << 8)      /*!< ETM System FIFOFILL Support   */
#define ETM_EVT_A                   0         /*!< ETM Event A offset            */
#define ETM_EVT_NOTA            (1 << 14)     /*!< ETM Event fn Not(A)           */
#define ETM_EVT_AANDB           (2 << 14)     /*!< ETM Event fn (A)and(B)        */
#define ETM_EVT_NOTAANDB        (3 << 14)     /*!< ETM Event fn (A)and(not(B))   */
#define ETM_EVT_NOTAANDNOTB     (4 << 14)     /*!< ETM Event fn (not(A))and(not(B))*/
#define ETM_EVT_AORB            (5 << 14)     /*!< ETM Event fn (A)or(B)         */
#define ETM_EVT_NOTAORB         (6 << 14)     /*!< ETM Event fn (not(A))or(B)    */
#define ETM_EVT_NOTAORNOTB      (7 << 14)     /*!< ETM Event fn (not(A))or(not(B))*/
#define ETM_EVT_RESB                   7     /*!< ETM Event B offset            */
#define ETM_EVT_DWT0               0x20      /*!< ETM Event select DWT0         */
#define ETM_EVT_DWT1               0x21      /*!< ETM Event select DWT1         */
#define ETM_EVT_DWT2               0x22      /*!< ETM Event select DWT2         */
#define ETM_EVT_DWT3               0x23      /*!< ETM Event select DWT3         */
#define ETM_EVT_COUNT1             0x40      /*!< ETM Event select Counter1 at zero */
#define ETM_EVT_SSTOP              0x5F      /*!< ETM Event select Start/Stop   */
#define ETM_EVT_EXTIN0             0x60      /*!< ETM Event select ExtIn0       */
#define ETM_EVT_EXTIN1             0x61      /*!< ETM Event select ExtIn1       */
#define ETM_EVT_TRUE               0x6F      /*!< ETM Event select Always True  */
#define ETM_TECR1_USE_SS           (1 << 25) /*!< ETM TraceEnable Start/Stop enable */
#define CS_UNLOCK                  0xC5ACCE55UL

#define ETM_TESSEICR_ICE0          0x1       /*!< ETM Start/Stop from DWT0      */
#define ETM_TESSEICR_ICE1          0x2       /*!< ETM Start/Stop from DWT1      */
#define ETM_TESSEICR_ICE2          0x4       /*!< ETM Start/Stop from DWT2      */
#define ETM_TESSEICR_ICE3          0x8       /*!< ETM Start/Stop from DWT3      */
   
#define ETM_TESSEICR_STOP          16        /*!< ETM Start/Stop Stop offset    */

/* TPIU */
#define TPIU_PIN_TRACEPORT         0       /*!< TPIU Selected Pin Protocol Trace Port */
#define TPIU_PIN_MANCHESTER        1       /*!< TPIU Selected Pin Protocol Manchester */
#define TPIU_PIN_NRZ               2       /*!< TPIU Selected Pin Protocol NRZ (uart) */

/* memory mapping struct for Nested Vectored Interrupt Controller (NVIC) */
typedef struct
{
  __IO uint32_t ISER[8];                      /*!< Interrupt Set Enable Register            */
       uint32_t RESERVED0[24];
  __IO uint32_t ICER[8];                      /*!< Interrupt Clear Enable Register          */
       uint32_t RSERVED1[24];
  __IO uint32_t ISPR[8];                      /*!< Interrupt Set Pending Register           */
       uint32_t RESERVED2[24];
  __IO uint32_t ICPR[8];                      /*!< Interrupt Clear Pending Register         */
       uint32_t RESERVED3[24];
  __IO uint32_t IABR[8];                      /*!< Interrupt Active bit Register            */
       uint32_t RESERVED4[56];
  __IO uint8_t  IP[240];                      /*!< Interrupt Priority Register, 8Bit wide   */
       uint32_t RESERVED5[644];
  __O  uint32_t STIR;                         /*!< Software Trigger Interrupt Register      */
}  NVIC_Type;


/* memory mapping struct for System Control Block */
typedef struct
{
  __I  uint32_t CPUID;                        /*!< CPU ID Base Register                                     */
  __IO uint32_t ICSR;                         /*!< Interrupt Control State Register                         */
  __IO uint32_t VTOR;                         /*!< Vector Table Offset Register                             */
  __IO uint32_t AIRCR;                        /*!< Application Interrupt / Reset Control Register           */
  __IO uint32_t SCR;                          /*!< System Control Register                                  */
  __IO uint32_t CCR;                          /*!< Configuration Control Register                           */
  __IO uint8_t  SHP[12];                      /*!< System Handlers Priority Registers (4-7, 8-11, 12-15)    */
  __IO uint32_t SHCSR;                        /*!< System Handler Control and State Register                */
  __IO uint32_t CFSR;                         /*!< Configurable Fault Status Register                       */
  __IO uint32_t HFSR;                         /*!< Hard Fault Status Register                               */
  __IO uint32_t DFSR;                         /*!< Debug Fault Status Register                              */
  __IO uint32_t MMFAR;                        /*!< Mem Manage Address Register                              */
  __IO uint32_t BFAR;                         /*!< Bus Fault Address Register                               */
  __IO uint32_t AFSR;                         /*!< Auxiliary Fault Status Register                          */
  __I  uint32_t PFR[2];                       /*!< Processor Feature Register                               */
  __I  uint32_t DFR;                          /*!< Debug Feature Register                                   */
  __I  uint32_t ADR;                          /*!< Auxiliary Feature Register                               */
  __I  uint32_t MMFR[4];                      /*!< Memory Model Feature Register                            */
  __I  uint32_t ISAR[5];                      /*!< ISA Feature Register                                     */
       uint32_t RESERVED0[5];
  __IO uint32_t CPACR;                        /*!< Coprocessor access register                              */
       uint32_t RESERVED1[106];
  __IO uint32_t FPCCR;                        /*!< Floating point context control register                              */
  __IO uint32_t FPCAR;                        /*!< Floating point context address register                             */
  __IO uint32_t FPDSCR;                        /*!< Floating point default status control register                     */
  __IO uint32_t MVFR0;                        /*!< Media and VFP feature register                              */
  __IO uint32_t MVFR1;                        /*!< Media and VFP feature register                              */
       uint32_t RESERVED2[34];
  __I  uint32_t PID4;                        /*!< CoreSight register  */
  __I  uint32_t PID5;                        /*!< CoreSight register  */
  __I  uint32_t PID6;                        /*!< CoreSight register  */
  __I  uint32_t PID7;                        /*!< CoreSight register  */
  __I  uint32_t PID0;                        /*!< CoreSight register  */
  __I  uint32_t PID1;                        /*!< CoreSight register  */
  __I  uint32_t PID2;                        /*!< CoreSight register  */
  __I  uint32_t PID3;                        /*!< CoreSight register  */
  __I  uint32_t CID0;                        /*!< CoreSight register  */
  __I  uint32_t CID1;                        /*!< CoreSight register  */
  __I  uint32_t CID2;                        /*!< CoreSight register  */
  __I  uint32_t CID3;                        /*!< CoreSight register  */
} SCB_Type;


/* memory mapping struct for SysTick */
typedef struct
{
  __IO uint32_t CTRL;                         /*!< SysTick Control and Status Register */
  __IO uint32_t LOAD;                         /*!< SysTick Reload Value Register       */
  __IO uint32_t VAL;                          /*!< SysTick Current Value Register      */
  __I  uint32_t CALIB;                        /*!< SysTick Calibration Register        */
} SysTick_Type;


/* memory mapping structur for ITM */
typedef struct
{
  __O  union  
  {
    __O  uint8_t    u8;                       /*!< ITM Stimulus Port 8-bit               */
    __O  uint16_t   u16;                      /*!< ITM Stimulus Port 16-bit              */
    __O  uint32_t   u32;                      /*!< ITM Stimulus Port 32-bit              */
  }  PORT [32];                               /*!< ITM Stimulus Port Registers           */
       uint32_t RESERVED0[864];
  __IO uint32_t TER;                          /*!< ITM Trace Enable Register             */
       uint32_t RESERVED1[15];
  __IO uint32_t TPR;                          /*!< ITM Trace Privilege Register          */
       uint32_t RESERVED2[15];
  __IO uint32_t TCR;                          /*!< ITM Trace Control Register            */
       uint32_t RESERVED3[29];
  __IO uint32_t IWR;                          /*!< ITM Integration Write Register        */
  __IO uint32_t IRR;                          /*!< ITM Integration Read Register         */
  __IO uint32_t IMCR;                         /*!< ITM Integration Mode Control Register */
       uint32_t RESERVED4[43];
  __IO uint32_t LAR;                          /*!< ITM Lock Access Register              */
  __IO uint32_t LSR;                          /*!< ITM Lock Status Register              */
       uint32_t RESERVED5[6];
  __I  uint32_t PID4;                        /*!< CoreSight register  */
  __I  uint32_t PID5;                        /*!< CoreSight register  */
  __I  uint32_t PID6;                        /*!< CoreSight register  */
  __I  uint32_t PID7;                        /*!< CoreSight register  */
  __I  uint32_t PID0;                        /*!< CoreSight register  */
  __I  uint32_t PID1;                        /*!< CoreSight register  */
  __I  uint32_t PID2;                        /*!< CoreSight register  */
  __I  uint32_t PID3;                        /*!< CoreSight register  */
  __I  uint32_t CID0;                        /*!< CoreSight register  */
  __I  uint32_t CID1;                        /*!< CoreSight register  */
  __I  uint32_t CID2;                        /*!< CoreSight register  */
  __I  uint32_t CID3;                        /*!< CoreSight register  */
} ITM_Type;

/* memory mapping structur for DWT */
typedef struct
{
  __IO uint32_t CTRL;                         /*!< DWT Control Register        */
  __IO uint32_t CYCCNT;                       /*!< DWT Cycle Count             */
  __IO uint32_t CPICNT;                       /*!< DWT CPI Count               */
  __IO uint32_t EXCCNT;                       /*!< DWT Exception Count         */
  __IO uint32_t SLEEPCNT;                     /*!< DWT Sleep Count             */
  __IO uint32_t LSUCNT;                       /*!< DWT LSU Count               */
  __IO uint32_t FOLDCNT;                      /*!< DWT Cold Count              */
  __I  uint32_t PCSR;                         /*!< DWT PC Sample Register      */
  __IO uint32_t COMP0;                        /*!< DWT Comparator0 Value       */
  __IO uint32_t MASK0;                        /*!< DWT Comparator0 Mask        */
  __IO uint32_t FUNCTION0;                    /*!< DWT Comparator0 Function    */
       uint32_t RESERVED0;
  __IO uint32_t COMP1;                        /*!< DWT Comparator1 Value       */
  __IO uint32_t MASK1;                        /*!< DWT Comparator1 Mask        */
  __IO uint32_t FUNCTION1;                    /*!< DWT Comparator1 Function    */
       uint32_t RESERVED1;
  __IO uint32_t COMP2;                        /*!< DWT Comparator2 Value       */
  __IO uint32_t MASK2;                        /*!< DWT Comparator2 Mask        */
  __IO uint32_t FUNCTION2;                    /*!< DWT Comparator2 Function    */
       uint32_t RESERVED2;
  __IO uint32_t COMP3;                        /*!< DWT Comparator3 Value       */
  __IO uint32_t MASK3;                        /*!< DWT Comparator3 Mask        */
  __IO uint32_t FUNCTION3;                    /*!< DWT Comparator3 Function    */
       uint32_t RESERVED3[989];
  __I  uint32_t PID4;                        /*!< CoreSight register  */
  __I  uint32_t PID5;                        /*!< CoreSight register  */
  __I  uint32_t PID6;                        /*!< CoreSight register  */
  __I  uint32_t PID7;                        /*!< CoreSight register  */
  __I  uint32_t PID0;                        /*!< CoreSight register  */
  __I  uint32_t PID1;                        /*!< CoreSight register  */
  __I  uint32_t PID2;                        /*!< CoreSight register  */
  __I  uint32_t PID3;                        /*!< CoreSight register  */
  __I  uint32_t CID0;                        /*!< CoreSight register  */
  __I  uint32_t CID1;                        /*!< CoreSight register  */
  __I  uint32_t CID2;                        /*!< CoreSight register  */
  __I  uint32_t CID3;                        /*!< CoreSight register  */
} DWT_Type;

/* memory mapping structur for FPB */
typedef struct
{
  __IO uint32_t CTRL;                         /*!< FPB Control Registers                */
  __IO uint32_t REMAP;                        /*!< FPB Control Registers                */
  __IO uint32_t COMP0;                        /*!< FPB Control Registers                */
  __IO uint32_t COMP1;                        /*!< FPB Control Registers                */
  __IO uint32_t COMP2;                        /*!< FPB Control Registers                */
  __IO uint32_t COMP3;                        /*!< FPB Control Registers                */
  __IO uint32_t COMP4;                        /*!< FPB Control Registers                */
  __IO uint32_t COMP5;                        /*!< FPB Control Registers                */
  __IO uint32_t COMP6;                        /*!< FPB Control Registers                */
  __IO uint32_t COMP7;                        /*!< FPB Control Registers                */
       uint32_t RESERVED3[1002];
  __I  uint32_t PID4;                        /*!< CoreSight register  */
  __I  uint32_t PID5;                        /*!< CoreSight register  */
  __I  uint32_t PID6;                        /*!< CoreSight register  */
  __I  uint32_t PID7;                        /*!< CoreSight register  */
  __I  uint32_t PID0;                        /*!< CoreSight register  */
  __I  uint32_t PID1;                        /*!< CoreSight register  */
  __I  uint32_t PID2;                        /*!< CoreSight register  */
  __I  uint32_t PID3;                        /*!< CoreSight register  */
  __I  uint32_t CID0;                        /*!< CoreSight register  */
  __I  uint32_t CID1;                        /*!< CoreSight register  */
  __I  uint32_t CID2;                        /*!< CoreSight register  */
  __I  uint32_t CID3;                        /*!< CoreSight register  */
} FPB_Type;


typedef struct
{
  __I  uint32_t SSPSR;                        /*!< TPIU Supported Synchronous Port Size register */
  __IO uint32_t CSPSR;                        /*!< TPIU Current Synchronous Port Size register  */
       uint32_t RESERVED0[2];
  __IO uint32_t ACPR;                         /*!< TPIU Asynchronous Clock Prescale Register    */
       uint32_t RESERVED1[55];
  __IO uint32_t SPPR;                         /*!< TPIU Selected Pin Protocol Register          */
       uint32_t RESERVED2[132];
  __I  uint32_t FFPR;                         /*!< TPIU Formatter and Flush Status Register     */
       uint32_t RESERVED3[759];
  __I  uint32_t TRIGGER;                      /*!< TPIU Trigger Integration Register            */
  __I  uint32_t ITETMDATA;                    /*!< TPIU  Integration ETM Data Register          */
  __I  uint32_t ITATBCR2;                     /*!< TPIU  Integration ATB Control2 Register      */
  __I  uint32_t ITITMDATA;                    /*!< TPIU  Integration ITM Data Register          */
  __I  uint32_t ITATBCR0;                     /*!< TPIU  Integration ATB Control0 Register      */
       uint32_t RESERVED4;
  __IO uint32_t ITCTRL;                       /*!< TPIU  Integration Control Register  */
       uint32_t RESERVED5[40];
  __IO uint32_t CLAIMSET;                     /*!< TPIU  Claim Tag Set Register        */
  __IO uint32_t CLAIMCLR;                     /*!< TPIU  Claim Tag Clear Register      */
       uint32_t RESERVED6[8];
  __IO uint32_t DEVID;                        /*!< TPIU  Device ID Register            */
       uint32_t RESERVED7;
  __I  uint32_t PID4;                         /*!< CoreSight register  */
  __I  uint32_t PID5;                         /*!< CoreSight register  */
  __I  uint32_t PID6;                         /*!< CoreSight register  */
  __I  uint32_t PID7;                         /*!< CoreSight register  */
  __I  uint32_t PID0;                         /*!< CoreSight register  */
  __I  uint32_t PID1;                         /*!< CoreSight register  */
  __I  uint32_t PID2;                         /*!< CoreSight register  */
  __I  uint32_t PID3;                         /*!< CoreSight register  */
  __I  uint32_t CID0;                         /*!< CoreSight register  */
  __I  uint32_t CID1;                         /*!< CoreSight register  */
  __I  uint32_t CID2;                         /*!< CoreSight register  */
  __I  uint32_t CID3;                         /*!< CoreSight register  */
} TPIU_Type;

/* memory mapping structure for ETM */
typedef struct
{
  __IO uint32_t CR;                          /*!< ETM Control Register                      */
  __I  uint32_t CCR;                         /*!< ETM Configuration Code Register           */
  __IO uint32_t TRIGGER;                     /*!< ETM Trigger Event Register                */
       uint32_t RESERVED0;
  __IO uint32_t SR;                          /*!< ETM Status Register                       */
  __I  uint32_t SCR;                         /*!< ETM System Configuration Register         */
       uint32_t RESERVED1[2];
  __IO uint32_t TEEVR;                       /*!< ETM Trace Enable Event Register           */
  __IO uint32_t TECR1;                       /*!< ETM Trace Enable Control 1 Register       */
       uint32_t RESERVED2X;
  __IO uint32_t FFLR;                        /*!< ETM Fifo Full Level Register              */
       uint32_t RESERVED2[68];
  __IO uint32_t CNTRLDVR1;                   /*!< ETM Counter1 Reload Register              */
       uint32_t RESERVED3[39];
  __I  uint32_t SYNCFR;                      /*!< ETM Sync Frequency Register               */
  __I  uint32_t IDR;                         /*!< ETM ID Register                           */
  __I  uint32_t CCER;                        /*!< ETM Configuration Code Extention Register */
       uint32_t RESERVED4;
  __IO uint32_t TESSEICR;                    /*!< ETM Trace Enable Start Stop EICE Register */
       uint32_t RESERVED5;
  __IO uint32_t TSEVR;                       /*!< ETM Timestamp Event Register              */
       uint32_t RESERVED6;
  __IO uint32_t TRACEIDR;                    /*!< ETM Trace ID Register                     */
       uint32_t RESERVED7[68];
  __I  uint32_t PDSR;                        /*!< ETM Power Down Status Register            */
       uint32_t RESERVED8[754];
  __I  uint32_t ITMISCIN;                    /*!< ETM Integration Misc In Register          */
       uint32_t RESERVED9;
  __IO uint32_t ITTRIGOUT;                   /*!< ETM Integration Trigger Register          */
       uint32_t RESERVED10;
  __I  uint32_t ITATBCR2;                    /*!< ETM Integration ATB2 Register             */
       uint32_t RESERVED11;
  __IO uint32_t ITATBCR0;                    /*!< ETM Integration ATB0 Register             */
       uint32_t RESERVED12;
  __IO uint32_t ITCTRL;                      /*!< ETM Integration Mode Control Register     */
       uint32_t RESERVED13[39];
  __IO uint32_t CLAIMSET;                    /*!< ETM Claim Set Register                    */
  __IO uint32_t CLAIMCLR;                    /*!< ETM Claim Clear Register                  */
       uint32_t RESERVED14[2];
  __IO uint32_t LAR;                         /*!< ETM Lock Access Register                  */
  __I  uint32_t LSR;                         /*!< ETM Lock Status Register                  */
  __I  uint32_t AUTHSTATUS;                  /*!< ETM Authentication Status Register        */
       uint32_t RESERVED15[4]; 
  __I  uint32_t DEVTYPE;                     /*!< ETM Device Type Register                  */
  __I  uint32_t PID4;                        /*!< CoreSight register  */
  __I  uint32_t PID5;                        /*!< CoreSight register  */
  __I  uint32_t PID6;                        /*!< CoreSight register  */
  __I  uint32_t PID7;                        /*!< CoreSight register  */
  __I  uint32_t PID0;                        /*!< CoreSight register  */
  __I  uint32_t PID1;                        /*!< CoreSight register  */
  __I  uint32_t PID2;                        /*!< CoreSight register  */
  __I  uint32_t PID3;                        /*!< CoreSight register  */
  __I  uint32_t CID0;                        /*!< CoreSight register  */
  __I  uint32_t CID1;                        /*!< CoreSight register  */
  __I  uint32_t CID2;                        /*!< CoreSight register  */
  __I  uint32_t CID3;                        /*!< CoreSight register  */
} ETM_Type;
     
/* memory mapped struct for Interrupt Type */
typedef struct
{
       uint32_t RESERVED0;
  __I  uint32_t ICTR;                         /*!< Interrupt Control Type Register  */
  __IO uint32_t ACTLR;                        /*!< Auxiliary Control Register       */
} InterruptType_Type;


/* Memory Protection Unit */
#if defined (__MPU_PRESENT) && (__MPU_PRESENT == 1)
typedef struct
{
  __I  uint32_t TYPE;                         /*!< MPU Type Register                               */
  __IO uint32_t CTRL;                         /*!< MPU Control Register                            */
  __IO uint32_t RNR;                          /*!< MPU Region RNRber Register                      */
  __IO uint32_t RBAR;                         /*!< MPU Region Base Address Register                */
  __IO uint32_t RASR;                         /*!< MPU Region Attribute and Size Register          */
  __IO uint32_t RBAR_A1;                      /*!< MPU Alias 1 Region Base Address Register        */
  __IO uint32_t RASR_A1;                      /*!< MPU Alias 1 Region Attribute and Size Register  */
  __IO uint32_t RBAR_A2;                      /*!< MPU Alias 2 Region Base Address Register        */
  __IO uint32_t RASR_A2;                      /*!< MPU Alias 2 Region Attribute and Size Register  */
  __IO uint32_t RBAR_A3;                      /*!< MPU Alias 3 Region Base Address Register        */
  __IO uint32_t RASR_A3;                      /*!< MPU Alias 3 Region Attribute and Size Register  */
} MPU_Type;
#endif


/* Core Debug Register */
typedef struct
{
  __IO uint32_t DHCSR;                        /*!< Debug Halting Control and Status Register       */
  __O  uint32_t DCRSR;                        /*!< Debug Core Register Selector Register           */
  __IO uint32_t DCRDR;                        /*!< Debug Core Register Data Register               */
  __IO uint32_t DEMCR;                        /*!< Debug Exception and Monitor Control Register    */
} CoreDebug_Type;


/* Memory mapping of Cortex-M4 Hardware */

#define SCS_BASE            (0xE000E000)                              /*!< System Control Space Base Address    */
#define ITM_BASE            (0xE0000000)                              /*!< ITM Base Address                     */
#define DWT_BASE            (0xE0001000)                              /*!< DWT Base Address                     */
#define FPB_BASE            (0xE0002000)                              /*!< FPB Base Address                     */
#define TPIU_BASE           (0xE0040000)                              /*!< TPIU Base Address                    */
#define ETM_BASE            (0xE0041000)                              /*!< ETM Base Address                     */
#define CoreDebug_BASE      (0xE000EDF0)                              /*!< Core Debug Base Address              */
#define SysTick_BASE        (SCS_BASE +  0x0010)                      /*!< SysTick Base Address                 */
#define NVIC_BASE           (SCS_BASE +  0x0100)                      /*!< NVIC Base Address                    */
#define SCB_BASE            (SCS_BASE +  0x0D00)                      /*!< System Control Block Base Address    */

#define InterruptType       ((InterruptType_Type *) SCS_BASE)         /*!< Interrupt Type Register              */
#define SCB                 ((SCB_Type *)           SCB_BASE)         /*!< SCB configuration struct             */
#define SysTick             ((SysTick_Type *)       SysTick_BASE)     /*!< SysTick configuration struct         */
#define NVIC                ((NVIC_Type *)          NVIC_BASE)        /*!< NVIC configuration struct            */
#define ITM                 ((ITM_Type *)           ITM_BASE)         /*!< ITM configuration struct             */
#define DWT                 ((DWT_Type *)           DWT_BASE)         /*!< DWT configuration struct             */
#define FPB                 ((FPB_Type *)           FPB_BASE)         /*!< DWT configuration struct             */
#define TPIU                ((TPIU_Type *)          TPIU_BASE)         /*!< TPIU configuration struct           */
#define ETM                 ((ETM_Type *)           ETM_BASE)         /*!< ETM configuration struct             */
#define CoreDebug           ((CoreDebug_Type *)     CoreDebug_BASE)   /*!< Core Debug configuration struct      */

#if defined (__MPU_PRESENT) && (__MPU_PRESENT == 1)
  #define MPU_BASE          (SCS_BASE +  0x0D90)                      /*!< Memory Protection Unit               */
  #define MPU               ((MPU_Type*)            MPU_BASE)         /*!< Memory Protection Unit               */
#endif



/*******************************************************************************
 *                Hardware Abstraction Layer
 ******************************************************************************/


#if defined ( __CC_ARM   )
  #define __ASM            __asm                                      /*!< asm keyword for ARM Compiler          */
  #define __INLINE         __inline                                   /*!< inline keyword for ARM Compiler       */

#elif defined ( __ICCARM__ )
  #define __ASM           __asm                                       /*!< asm keyword for IAR Compiler           */
  #define __INLINE        inline                                      /*!< inline keyword for IAR Compiler. Only avaiable in High optimization mode! */

#elif defined   (  __GNUC__  )
  #define __ASM            __asm                                      /*!< asm keyword for GNU Compiler          */
  #define __INLINE         inline                                     /*!< inline keyword for GNU Compiler       */

#elif defined   (  __TASKING__  )
  #define __ASM            __asm                                      /*!< asm keyword for TASKING Compiler          */
  #define __INLINE         inline                                     /*!< inline keyword for TASKING Compiler       */

#endif


/* ###################  Compiler specific Intrinsics  ########################### */

#if defined ( __CC_ARM   ) /*------------------RealView Compiler -----------------*/
/* ARM armcc specific functions */

#define __enable_fault_irq                __enable_fiq
#define __disable_fault_irq               __disable_fiq

#define __NOP                             __nop
#define __WFI                             __wfi
#define __WFE                             __wfe
#define __SEV                             __sev
#define __ISB()                           __isb(0)
#define __DSB()                           __dsb(0)
#define __DMB()                           __dmb(0)
#define __REV                             __rev
#define __RBIT                            __rbit
#define __LDREXB(ptr)                     ((unsigned char ) __ldrex(ptr))
#define __LDREXH(ptr)                     ((unsigned short) __ldrex(ptr))
#define __LDREXW(ptr)                     ((unsigned int  ) __ldrex(ptr))
#define __STREXB(value, ptr)              __strex(value, ptr)
#define __STREXH(value, ptr)              __strex(value, ptr)
#define __STREXW(value, ptr)              __strex(value, ptr)


/* intrinsic unsigned long long __ldrexd(volatile void *ptr) */
/* intrinsic int __strexd(unsigned long long val, volatile void *ptr) */
/* intrinsic void __enable_irq();     */
/* intrinsic void __disable_irq();    */


/**
 * @brief  Return the Process Stack Pointer
 *
 * @param  none
 * @return uint32_t ProcessStackPointer
 *
 * Return the actual process stack pointer
 */
extern uint32_t __get_PSP(void);

/**
 * @brief  Set the Process Stack Pointer
 *
 * @param  uint32_t Process Stack Pointer
 * @return none
 *
 * Assign the value ProcessStackPointer to the MSP 
 * (process stack pointer) Cortex processor register
 */
extern void __set_PSP(uint32_t topOfProcStack);

/**
 * @brief  Return the Main Stack Pointer
 *
 * @param  none
 * @return uint32_t Main Stack Pointer
 *
 * Return the current value of the MSP (main stack pointer)
 * Cortex processor register
 */
extern uint32_t __get_MSP(void);

/**
 * @brief  Set the Main Stack Pointer
 *
 * @param  uint32_t Main Stack Pointer
 * @return none
 *
 * Assign the value mainStackPointer to the MSP 
 * (main stack pointer) Cortex processor register
 */
extern void __set_MSP(uint32_t topOfMainStack);

/**
 * @brief  Reverse byte order in unsigned short value
 *
 * @param  uint16_t value to reverse
 * @return uint32_t reversed value
 *
 * Reverse byte order in unsigned short value
 */
extern uint32_t __REV16(uint16_t value);

/*
 * @brief  Reverse byte order in signed short value with sign extension to integer
 *
 * @param  int16_t value to reverse
 * @return int32_t reversed value
 *
 * Reverse byte order in signed short value with sign extension to integer
 */
extern int32_t __REVSH(int16_t value);


#if (__ARMCC_VERSION < 400000)

/**
 * @brief  Remove the exclusive lock created by ldrex
 *
 * @param  none
 * @return none
 *
 * Removes the exclusive lock which is created by ldrex.
 */
extern void __CLREX(void);

/**
 * @brief  Return the Base Priority value
 *
 * @param  none
 * @return uint32_t BasePriority
 *
 * Return the content of the base priority register
 */
extern uint32_t __get_BASEPRI(void);

/**
 * @brief  Set the Base Priority value
 *
 * @param  uint32_t BasePriority
 * @return none
 *
 * Set the base priority register
 */
extern void __set_BASEPRI(uint32_t basePri);

/**
 * @brief  Return the FPSCR value
 *
 * @param  none
 * @return uint32_t FPSCR
 *
 * Return the content of the FPSCR register
 */
extern uint32_t __get_FPSCR(void);

/**
 * @brief  Set FPSCR value
 *
 * @param  uint32_t FPSCR
 * @return none
 *
 * Set the FPSCR register
 */
extern void __set_FPSCR(uint32_t fpscr);


/**
 * @brief  Return the Priority Mask value
 *
 * @param  none
 * @return uint32_t PriMask
 *
 * Return the state of the priority mask bit from the priority mask
 * register
 */
extern uint32_t __get_PRIMASK(void);

/**
 * @brief  Set the Priority Mask value
 *
 * @param  uint32_t PriMask
 * @return none
 *
 * Set the priority mask bit in the priority mask register
 */
extern void __set_PRIMASK(uint32_t priMask);

/**
 * @brief  Return the Fault Mask value
 *
 * @param  none
 * @return uint32_t FaultMask
 *
 * Return the content of the fault mask register
 */
extern uint32_t __get_FAULTMASK(void);

/**
 * @brief  Set the Fault Mask value
 *
 * @param  uint32_t faultMask value
 * @return none
 *
 * Set the fault mask register
 */
extern void __set_FAULTMASK(uint32_t faultMask);

/**
 * @brief  Return the Control Register value
 * 
 * @param  none
 * @return uint32_t Control value
 *
 * Return the content of the control register
 */
extern uint32_t __get_CONTROL(void);

/**
 * @brief  Return the IPSR Register value
 * 
 * @param  none
 * @return uint32_t IPSR value
 *
 * Return the content of the IPSR register
 */

extern uint32_t __get_IPSR(void);

/**
 * @brief  Set the Control Register value
 *
 * @param  uint32_t Control value
 * @return none
 *
 * Set the control register
 */
extern void __set_CONTROL(uint32_t control);

#else  /* (__ARMCC_VERSION >= 400000)  */


/**
 * @brief  Remove the exclusive lock created by ldrex
 *
 * @param  none
 * @return none
 *
 * Removes the exclusive lock which is created by ldrex.
 */
#define __CLREX                           __clrex

/**
 * @brief  Return the Base Priority value
 *
 * @param  none
 * @return uint32_t BasePriority
 *
 * Return the content of the base priority register
 */
static __INLINE uint32_t  __get_BASEPRI(void)
{
  register uint32_t __regBasePri         __ASM("basepri");
  return(__regBasePri);
}

/**
 * @brief  Set the Base Priority value
 *
 * @param  uint32_t BasePriority
 * @return none
 *
 * Set the base priority register
 */
static __INLINE void __set_BASEPRI(uint32_t basePri)
{
  register uint32_t __regBasePri         __ASM("basepri");
  __regBasePri = (basePri & 0x1ff);
}

/**
 * @brief  Return FPSCR value
 *
 * @param  none
 * @return uint32_t FPSCR
 *
 * Return the content of the FPSCR register
 */
static __INLINE uint32_t  __get_FPSCR(void)
{
#if defined (__TARGET_FPU_VFP)
  register uint32_t __regfpscr         __ASM("fpscr");
  return(__regfpscr);
#else
    return(0);
#endif
}

/**
 * @brief  Set the FPSCR value
 *
 * @param  uint32_t FPSCR
 * @return none
 *
 * Set the FPSCR register
 */
static __INLINE void __set_FPSCR(uint32_t fpscr)
{
#if defined (__TARGET_FPU_VFP)
  register uint32_t __regfpscr         __ASM("fpscr");
  __regfpscr = (fpscr);
#endif
}


/**
 * @brief  Return the Priority Mask value
 *
 * @param  none
 * @return uint32_t PriMask
 *
 * Return the state of the priority mask bit from the priority mask
 * register
 */
static __INLINE uint32_t __get_PRIMASK(void)
{
  register uint32_t __regPriMask         __ASM("primask");
  return(__regPriMask);
}

/**
 * @brief  Set the Priority Mask value
 *
 * @param  uint32_t PriMask
 * @return none
 *
 * Set the priority mask bit in the priority mask register
 */
static __INLINE void __set_PRIMASK(uint32_t priMask)
{
  register uint32_t __regPriMask         __ASM("primask");
  __regPriMask = (priMask);
}

/**
 * @brief  Return the Fault Mask value
 *
 * @param  none
 * @return uint32_t FaultMask
 *
 * Return the content of the fault mask register
 */
static __INLINE uint32_t __get_FAULTMASK(void)
{
  register uint32_t __regFaultMask       __ASM("faultmask");
  return(__regFaultMask);
}



/**
 * @brief  Set the Fault Mask value
 *
 * @param  uint32_t faultMask value
 * @return none
 *
 * Set the fault mask register
 */
static __INLINE void __set_FAULTMASK(uint32_t faultMask)
{
  register uint32_t __regFaultMask       __ASM("faultmask");
  __regFaultMask = (faultMask & 1);
}

/**
 * @brief  Return the Control Register value
 * 
 * @param  none
 * @return uint32_t Control value
 *
 * Return the content of the control register
 */
static __INLINE uint32_t __get_CONTROL(void)
{
  register uint32_t __regControl         __ASM("control");
  return(__regControl);
}

/**
 * @brief  Return the IPSR Register value
 * 
 * @param  none
 * @return uint32_t IPSR value
 *
 * Return the content of the IPSR register
 */

static __INLINE uint32_t __get_IPSR(void)
{
  register uint32_t __regControl         __ASM("ipsr");
  return(__regControl);
}


/**
 * @brief  Set the Control Register value
 *
 * @param  uint32_t Control value
 * @return none
 *
 * Set the control register
 */
static __INLINE void __set_CONTROL(uint32_t control)
{
  register uint32_t __regControl         __ASM("control");
  __regControl = control;
}

#endif /* __ARMCC_VERSION  */ 



#elif (defined (__ICCARM__)) /*------------------ ICC Compiler -------------------*/
/* IAR iccarm specific functions */

#define __enable_irq                              __enable_interrupt        /*!< global Interrupt enable */
#define __disable_irq                             __disable_interrupt       /*!< global Interrupt disable */

static __INLINE void __enable_fault_irq()         { __ASM ("cpsie f"); }
static __INLINE void __disable_fault_irq()        { __ASM ("cpsid f"); }

#define __NOP                                     __no_operation()          /*!< no operation intrinsic in IAR Compiler */ 
static __INLINE  void __WFI()                     { __ASM ("wfi"); }
static __INLINE  void __WFE()                     { __ASM ("wfe"); }
static __INLINE  void __SEV()                     { __ASM ("sev"); }
static __INLINE  void __CLREX()                   { __ASM ("clrex"); }

/* intrinsic void __ISB(void)                                     */
/* intrinsic void __DSB(void)                                     */
/* intrinsic void __DMB(void)                                     */
/* intrinsic void __set_PRIMASK();                                */
/* intrinsic void __get_PRIMASK();                                */
/* intrinsic void __set_FAULTMASK();                              */
/* intrinsic void __get_FAULTMASK();                              */
/* intrinsic uint32_t __REV(uint32_t value);                      */
/* intrinsic uint32_t __REVSH(uint32_t value);                    */
/* intrinsic unsigned long __STREX(unsigned long, unsigned long); */
/* intrinsic unsigned long __LDREX(unsigned long *);              */


/**
 * @brief  Return the Process Stack Pointer
 *
 * @param  none
 * @return uint32_t ProcessStackPointer
 *
 * Return the actual process stack pointer
 */
extern uint32_t __get_PSP(void);

/**
 * @brief  Set the Process Stack Pointer
 *
 * @param  uint32_t Process Stack Pointer
 * @return none
 *
 * Assign the value ProcessStackPointer to the MSP 
 * (process stack pointer) Cortex processor register
 */
extern void __set_PSP(uint32_t topOfProcStack);

/**
 * @brief  Return the Main Stack Pointer
 *
 * @param  none
 * @return uint32_t Main Stack Pointer
 *
 * Return the current value of the MSP (main stack pointer)
 * Cortex processor register
 */
extern uint32_t __get_MSP(void);

/**
 * @brief  Set the Main Stack Pointer
 *
 * @param  uint32_t Main Stack Pointer
 * @return none
 *
 * Assign the value mainStackPointer to the MSP 
 * (main stack pointer) Cortex processor register
 */
extern void __set_MSP(uint32_t topOfMainStack);

/**
 * @brief  Reverse byte order in unsigned short value
 *
 * @param  uint16_t value to reverse
 * @return uint32_t reversed value
 *
 * Reverse byte order in unsigned short value
 */
//extern uint32_t __REV16(uint16_t value);

/**
 * @brief  Reverse bit order of value
 *
 * @param  uint32_t value to reverse
 * @return uint32_t reversed value
 *
 * Reverse bit order of value
 */
extern uint32_t __RBIT(uint32_t value);

/**
 * @brief  LDR Exclusive
 *
 * @param  uint8_t* address
 * @return uint8_t value of (*address)
 *
 * Exclusive LDR command
 */
//extern uint8_t __LDREXB(uint8_t *addr);

/**
 * @brief  LDR Exclusive
 *
 * @param  uint16_t* address
 * @return uint16_t value of (*address)
 *
 * Exclusive LDR command
 */
//extern uint16_t __LDREXH(uint16_t *addr);

/**
 * @brief  LDR Exclusive
 *
 * @param  uint32_t* address
 * @return uint32_t value of (*address)
 *
 * Exclusive LDR command
 */
extern uint32_t __LDREXW(uint32_t *addr);

/**
 * @brief  STR Exclusive
 *
 * @param  uint8_t *address
 * @param  uint8_t value to store
 * @return uint32_t successful / failed
 *
 * Exclusive STR command
 */
//extern uint32_t __STREXB(uint8_t value, uint8_t *addr);

/**
 * @brief  STR Exclusive
 *
 * @param  uint16_t *address
 * @param  uint16_t value to store
 * @return uint32_t successful / failed
 *
 * Exclusive STR command
 */
//extern uint32_t __STREXH(uint16_t value, uint16_t *addr);

/**
 * @brief  STR Exclusive
 *
 * @param  uint32_t *address
 * @param  uint32_t value to store
 * @return uint32_t successful / failed
 *
 * Exclusive STR command
 */
extern uint32_t __STREXW(uint32_t value, uint32_t *addr);



#elif (defined (__GNUC__)) /*------------------ GNU Compiler ---------------------*/
/* GNU gcc specific functions */

static __INLINE void __enable_irq()               { __ASM volatile ("cpsie i"); }
static __INLINE void __disable_irq()              { __ASM volatile ("cpsid i"); }

static __INLINE void __enable_fault_irq()         { __ASM volatile ("cpsie f"); }
static __INLINE void __disable_fault_irq()        { __ASM volatile ("cpsid f"); }

static __INLINE void __NOP()                      { __ASM volatile ("nop"); }
static __INLINE void __WFI()                      { __ASM volatile ("wfi"); }
static __INLINE void __WFE()                      { __ASM volatile ("wfe"); }
static __INLINE void __SEV()                      { __ASM volatile ("sev"); }
static __INLINE void __ISB()                      { __ASM volatile ("isb"); }
static __INLINE void __DSB()                      { __ASM volatile ("dsb"); }
static __INLINE void __DMB()                      { __ASM volatile ("dmb"); }
static __INLINE void __CLREX()                    { __ASM volatile ("clrex"); }


/**
 * @brief  Return the Process Stack Pointer
 *
 * @param  none
 * @return uint32_t ProcessStackPointer
 *
 * Return the actual process stack pointer
 */
extern uint32_t __get_PSP(void);

/**
 * @brief  Set the Process Stack Pointer
 *
 * @param  uint32_t Process Stack Pointer
 * @return none
 *
 * Assign the value ProcessStackPointer to the MSP 
 * (process stack pointer) Cortex processor register
 */
extern void __set_PSP(uint32_t topOfProcStack);

/**
 * @brief  Return the Main Stack Pointer
 *
 * @param  none
 * @return uint32_t Main Stack Pointer
 *
 * Return the current value of the MSP (main stack pointer)
 * Cortex processor register
 */
extern uint32_t __get_MSP(void);

/**
 * @brief  Set the Main Stack Pointer
 *
 * @param  uint32_t Main Stack Pointer
 * @return none
 *
 * Assign the value mainStackPointer to the MSP 
 * (main stack pointer) Cortex processor register
 */
extern void __set_MSP(uint32_t topOfMainStack);

/**
 * @brief  Return the Base Priority value
 *
 * @param  none
 * @return uint32_t BasePriority
 *
 * Return the content of the base priority register
 */
extern uint32_t __get_BASEPRI(void);

/**
 * @brief  Set the Base Priority value
 *
 * @param  uint32_t BasePriority
 * @return none
 *
 * Set the base priority register
 */
extern void __set_BASEPRI(uint32_t basePri);

/**
 * @brief  Return the FPSCR value
 *
 * @param  none
 * @return uint32_t FPSCR
 *
 * Return the content of the FPSCR register
 */
extern uint32_t __get_FPSCR(void);

/**
 * @brief  Set the FPSCR value
 *
 * @param  uint32_t FPSCR
 * @return none
 *
 * Set the FPSCR register
 */
extern void __set_FPSCR(uint32_t fpscr);


/**
 * @brief  Return the Priority Mask value
 *
 * @param  none
 * @return uint32_t PriMask
 *
 * Return the state of the priority mask bit from the priority mask
 * register
 */
extern uint32_t  __get_PRIMASK(void);

/**
 * @brief  Set the Priority Mask value
 *
 * @param  uint32_t PriMask
 * @return none
 *
 * Set the priority mask bit in the priority mask register
 */
extern void __set_PRIMASK(uint32_t priMask);

/**
 * @brief  Return the Fault Mask value
 *
 * @param  none
 * @return uint32_t FaultMask
 *
 * Return the content of the fault mask register
 */
extern uint32_t __get_FAULTMASK(void);

/**
 * @brief  Set the Fault Mask value
 *
 * @param  uint32_t faultMask value
 * @return none
 *
 * Set the fault mask register
 */
extern void __set_FAULTMASK(uint32_t faultMask);

/**
 * @brief  Return the Control Register value
* 
*  @param  none
*  @return uint32_t Control value
 *
 * Return the content of the control register
 */
extern uint32_t __get_CONTROL(void);

/**
 * @brief  Return the IPSR Register value
 * 
 * @param  none
 * @return uint32_t IPSR value
 *
 * Return the content of the IPSR register
 */

extern uint32_t __get_IPSR(void);

/**
 * @brief  Set the Control Register value
 *
 * @param  uint32_t Control value
 * @return none
 *
 * Set the control register
 */
extern void __set_CONTROL(uint32_t control);

/**
 * @brief  Reverse byte order in integer value
 *
 * @param  uint32_t value to reverse
 * @return uint32_t reversed value
 *
 * Reverse byte order in integer value
 */
extern uint32_t __REV(uint32_t value);

/**
 * @brief  Reverse byte order in unsigned short value
 *
 * @param  uint16_t value to reverse
 * @return uint32_t reversed value
 *
 * Reverse byte order in unsigned short value
 */
extern uint32_t __REV16(uint16_t value);

/*
 * Reverse byte order in signed short value with sign extension to integer
 *
 * @param  int16_t value to reverse
 * @return int32_t reversed value
 *
 * @brief  Reverse byte order in signed short value with sign extension to integer
 */
extern int32_t __REVSH(int16_t value);

/**
 * @brief  Reverse bit order of value
 *
 * @param  uint32_t value to reverse
 * @return uint32_t reversed value
 *
 * Reverse bit order of value
 */
extern uint32_t __RBIT(uint32_t value);

/**
 * @brief  LDR Exclusive
 *
 * @param  uint8_t* address
 * @return uint8_t value of (*address)
 *
 * Exclusive LDR command
 */
extern uint8_t __LDREXB(uint8_t *addr);

/**
 * @brief  LDR Exclusive
 *
 * @param  uint16_t* address
 * @return uint16_t value of (*address)
 *
 * Exclusive LDR command
 */
extern uint16_t __LDREXH(uint16_t *addr);

/**
 * @brief  LDR Exclusive
 *
 * @param  uint32_t* address
 * @return uint32_t value of (*address)
 *
 * Exclusive LDR command
 */
extern uint32_t __LDREXW(uint32_t *addr);

/**
 * @brief  STR Exclusive
 *
 * @param  uint8_t *address
 * @param  uint8_t value to store
 * @return uint32_t successful / failed
 *
 * Exclusive STR command
 */
extern uint32_t __STREXB(uint8_t value, uint8_t *addr);

/**
 * @brief  STR Exclusive
 *
 * @param  uint16_t *address
 * @param  uint16_t value to store
 * @return uint32_t successful / failed
 *
 * Exclusive STR command
 */
extern uint32_t __STREXH(uint16_t value, uint16_t *addr);

/**
 * @brief  STR Exclusive
 *
 * @param  uint32_t *address
 * @param  uint32_t value to store
 * @return uint32_t successful / failed
 *
 * Exclusive STR command
 */
extern uint32_t __STREXW(uint32_t value, uint32_t *addr);


#elif (defined (__TASKING__)) /*------------------ TASKING Compiler ---------------------*/
/* TASKING carm specific functions */

/*
 * The CMSIS functions have been implemented as intrinsics in the compiler.
 * Please use "carm -?i" to get an up to date list of all instrinsics,
 * Including the CMSIS ones.
 */

#endif



/* ##########################   NVIC functions  #################################### */


/**
 * @brief  Set the Priority Grouping in NVIC Interrupt Controller
 *
 * @param  uint32_t priority_grouping is priority grouping field
 * @return none 
 *
 * Set the priority grouping field using the required unlock sequence.
 * The parameter priority_grouping is assigned to the field 
 * SCB->AIRCR [10:8] PRIGROUP field. Only values from 0..7 are used.
 * In case of a conflict between priority grouping and available
 * priority bits (__NVIC_PRIO_BITS) the smallest possible priority group is set.
 */
static __INLINE void NVIC_SetPriorityGrouping(uint32_t PriorityGroup)
{
  uint32_t reg_value;
  uint32_t PriorityGroupTmp = (PriorityGroup & 0x07);                         /* only values 0..7 are used          */
  
  reg_value  = SCB->AIRCR;                                                    /* read old register configuration    */
  reg_value &= ~((0xFFFFU << 16) | (0x0F << 8));                              /* clear bits to change               */
  reg_value  = ((reg_value | NVIC_AIRCR_VECTKEY | (PriorityGroupTmp << 8)));  /* Insert write key and priorty group */
  SCB->AIRCR = reg_value;
}

/**
 * @brief  Get the Priority Grouping from NVIC Interrupt Controller
 *
 * @param  none
 * @return uint32_t   priority grouping field 
 *
 * Get the priority grouping from NVIC Interrupt Controller.
 * priority grouping is SCB->AIRCR [10:8] PRIGROUP field.
 */
static __INLINE uint32_t NVIC_GetPriorityGrouping(void)
{
  return ((SCB->AIRCR >> 8) & 0x07);                                          /* read priority grouping field */
}

/**
 * @brief  Enable Interrupt in NVIC Interrupt Controller
 *
 * @param  IRQn_Type IRQn specifies the interrupt number
 * @return none 
 *
 * Enable a device specific interupt in the NVIC interrupt controller.
 * The interrupt number cannot be a negative value.
 */
static __INLINE void NVIC_EnableIRQ(IRQn_Type IRQn)
{
  NVIC->ISER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* enable interrupt */
}

/**
 * @brief  Disable the interrupt line for external interrupt specified
 * 
 * @param  IRQn_Type IRQn is the positive number of the external interrupt
 * @return none
 * 
 * Disable a device specific interupt in the NVIC interrupt controller.
 * The interrupt number cannot be a negative value.
 */
static __INLINE void NVIC_DisableIRQ(IRQn_Type IRQn)
{
  NVIC->ICER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* disable interrupt */
}

/**
 * @brief  Read the interrupt pending bit for a device specific interrupt source
 * 
 * @param  IRQn_Type IRQn is the number of the device specifc interrupt
 * @return uint32_t 1 if pending interrupt else 0
 *
 * Read the pending register in NVIC and return 1 if its status is pending, 
 * otherwise it returns 0
 */
static __INLINE uint32_t NVIC_GetPendingIRQ(IRQn_Type IRQn)
{
  return((uint32_t) ((NVIC->ISPR[(uint32_t)(IRQn) >> 5] & (1 << ((uint32_t)(IRQn) & 0x1F)))?1:0)); /* Return 1 if pending else 0 */
}

/**
 * @brief  Set the pending bit for an external interrupt
 * 
 * @param  IRQn_Type IRQn is the Number of the interrupt
 * @return none
 *
 * Set the pending bit for the specified interrupt.
 * The interrupt number cannot be a negative value.
 */
static __INLINE void NVIC_SetPendingIRQ(IRQn_Type IRQn)
{
  NVIC->ISPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* set interrupt pending */
}

/**
 * @brief  Clear the pending bit for an external interrupt
 *
 * @param  IRQn_Type IRQn is the Number of the interrupt
 * @return none
 *
 * Clear the pending bit for the specified interrupt. 
 * The interrupt number cannot be a negative value.
 */
static __INLINE void NVIC_ClearPendingIRQ(IRQn_Type IRQn)
{
  NVIC->ICPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* Clear pending interrupt */
}

/**
 * @brief  Read the active bit for an external interrupt
 *
 * @param  IRQn_Type  IRQn is the Number of the interrupt
 * @return uint32_t   1 if active else 0
 *
 * Read the active register in NVIC and returns 1 if its status is active, 
 * otherwise it returns 0.
 */
static __INLINE uint32_t NVIC_GetActive(IRQn_Type IRQn)
{
  return((uint32_t)((NVIC->IABR[(uint32_t)(IRQn) >> 5] & (1 << ((uint32_t)(IRQn) & 0x1F)))?1:0)); /* Return 1 if active else 0 */
}

/**
 * @brief  Set the priority for an interrupt
 *
 * @param  IRQn_Type IRQn is the Number of the interrupt
 * @param  priority is the priority for the interrupt
 * @return none
 *
 * Set the priority for the specified interrupt. The interrupt 
 * number can be positive to specify an external (device specific) 
 * interrupt, or negative to specify an internal (core) interrupt. \n
 *
 * Note: The priority cannot be set for every core interrupt.
 */
static __INLINE void NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)
{
  if(IRQn < 0) {
    SCB->SHP[((uint32_t)(IRQn) & 0xF)-4] = ((priority << (8 - __NVIC_PRIO_BITS)) & 0xff); } /* set Priority for Cortex-M4 System Interrupts */
  else {
    NVIC->IP[(uint32_t)(IRQn)] = ((priority << (8 - __NVIC_PRIO_BITS)) & 0xff);    }        /* set Priority for device specific Interrupts      */
}

/**
 * @brief  Read the priority for an interrupt
 *
 * @param  IRQn_Type IRQn is the Number of the interrupt
 * @return uint32_t  priority is the priority for the interrupt
 *
 * Read the priority for the specified interrupt. The interrupt 
 * number can be positive to specify an external (device specific) 
 * interrupt, or negative to specify an internal (core) interrupt.
 *
 * The returned priority value is automatically aligned to the implemented
 * priority bits of the microcontroller.
 *
 * Note: The priority cannot be set for every core interrupt.
 */
static __INLINE uint32_t NVIC_GetPriority(IRQn_Type IRQn)
{

  if(IRQn < 0) {
    return((uint32_t)(SCB->SHP[((uint32_t)(IRQn) & 0xF)-4] >> (8 - __NVIC_PRIO_BITS)));  } /* get priority for Cortex-M4 system interrupts */
  else {
    return((uint32_t)(NVIC->IP[(uint32_t)(IRQn)]           >> (8 - __NVIC_PRIO_BITS)));  } /* get priority for device specific interrupts  */
}


/**
 * @brief  Encode the priority for an interrupt
 *
 * @param  uint32_t PriorityGroup   is the used priority group
 * @param  uint32_t PreemptPriority is the preemptive priority value (starting from 0)
 * @param  uint32_t SubPriority     is the sub priority value (starting from 0)
 * @return uint32_t                    the priority for the interrupt
 *
 * Encode the priority for an interrupt with the given priority group,
 * preemptive priority value and sub priority value.
 * In case of a conflict between priority grouping and available
 * priority bits (__NVIC_PRIO_BITS) the samllest possible priority group is set.
 *
 * The returned priority value can be used for NVIC_SetPriority(...) function
 */
static __INLINE uint32_t NVIC_EncodePriority (uint32_t PriorityGroup, uint32_t PreemptPriority, uint32_t SubPriority)
{
  uint32_t PriorityGroupTmp = (PriorityGroup & 0x07);                         /* only values 0..7 are used          */
  uint32_t PreemptPriorityBits;
  uint32_t SubPriorityBits;

  PreemptPriorityBits = ((7 - PriorityGroupTmp) > __NVIC_PRIO_BITS) ? __NVIC_PRIO_BITS : 7 - PriorityGroupTmp;
  SubPriorityBits     = ((PriorityGroupTmp + __NVIC_PRIO_BITS) < 7) ? 0 : PriorityGroupTmp - 7 + __NVIC_PRIO_BITS;
 
  return (
           ((PreemptPriority & ((1 << (PreemptPriorityBits)) - 1)) << SubPriorityBits) |
           ((SubPriority     & ((1 << (SubPriorityBits    )) - 1)))
         );
}


/**
 * @brief  Decode the priority of an interrupt
 *
 * @param  uint32_t   Priority       the priority for the interrupt
 * @param  uint32_t   PrioGroup   is the used priority group
 * @param  uint32_t* pPreemptPrio is the preemptive priority value (starting from 0)
 * @param  uint32_t* pSubPrio     is the sub priority value (starting from 0)
 * @return none
 *
 * Decode an interrupt priority value with the given priority group to 
 * preemptive priority value and sub priority value.
 * In case of a conflict between priority grouping and available
 * priority bits (__NVIC_PRIO_BITS) the samllest possible priority group is set.
 *
 * The priority value can be retrieved with NVIC_GetPriority(...) function
 */
static __INLINE void NVIC_DecodePriority (uint32_t Priority, uint32_t PriorityGroup, uint32_t* pPreemptPriority, uint32_t* pSubPriority)
{
  uint32_t PriorityGroupTmp = (PriorityGroup & 0x07);                         /* only values 0..7 are used          */
  uint32_t PreemptPriorityBits;
  uint32_t SubPriorityBits;

  PreemptPriorityBits = ((7 - PriorityGroupTmp) > __NVIC_PRIO_BITS) ? __NVIC_PRIO_BITS : 7 - PriorityGroupTmp;
  SubPriorityBits     = ((PriorityGroupTmp + __NVIC_PRIO_BITS) < 7) ? 0 : PriorityGroupTmp - 7 + __NVIC_PRIO_BITS;
  
  *pPreemptPriority = (Priority >> SubPriorityBits) & ((1 << (PreemptPriorityBits)) - 1);
  *pSubPriority     = (Priority                   ) & ((1 << (SubPriorityBits    )) - 1);
}



/* ##################################    SysTick function  ############################################ */

#if (!defined (__Vendor_SysTickConfig)) || (__Vendor_SysTickConfig == 0)

/* SysTick constants */
#define SYSTICK_ENABLE              0                                          /* Config-Bit to start or stop the SysTick Timer                         */
#define SYSTICK_TICKINT             1                                          /* Config-Bit to enable or disable the SysTick interrupt                 */
#define SYSTICK_CLKSOURCE           2                                          /* Clocksource has the offset 2 in SysTick Control and Status Register   */
#define SYSTICK_MAXCOUNT       ((1<<24) -1)                                    /* SysTick MaxCount                                                      */

/**
 * @brief  Initialize and start the SysTick counter and its interrupt.
 *
 * @param  uint32_t ticks is the number of ticks between two interrupts
 * @return  none
 *
 * Initialise the system tick timer and its interrupt and start the
 * system tick timer / counter in free running mode to generate 
 * periodical interrupts.
 */
static __INLINE uint32_t SysTick_Config(uint32_t ticks)
{ 
  if (ticks > SYSTICK_MAXCOUNT)  return (1);                                             /* Reload value impossible */

  SysTick->LOAD  =  (ticks & SYSTICK_MAXCOUNT) - 1;                                      /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);                            /* set Priority for Cortex-M0 System Interrupts */
  SysTick->VAL   =  (0x00);                                                              /* Load the SysTick Counter Value */
  SysTick->CTRL = (1 << SYSTICK_CLKSOURCE) | (1<<SYSTICK_ENABLE) | (1<<SYSTICK_TICKINT); /* Enable SysTick IRQ and SysTick Timer */
  return (0);                                                                            /* Function successful */
}

#endif





/* ##################################    Reset function  ############################################ */

/**
 * @brief  Initiate a system reset request.
 *
 * @param   none
 * @return  none
 *
 * Initialize a system reset request to reset the MCU
 */
static __INLINE void NVIC_SystemReset(void)
{
  SCB->AIRCR  = (NVIC_AIRCR_VECTKEY | (SCB->AIRCR & (0x700)) | (1<<NVIC_SYSRESETREQ)); /* Keep priority group unchanged */
  __DSB();                                                                             /* Ensure completion of memory access */              
  while(1);                                                                            /* wait until reset */
}


/* ##################################    Debug Output  function  ############################################ */


/**
 * @brief  Outputs a character via the ITM channel 0
 *
 * @param   uint32_t character to output
 * @return  uint32_t input character
 *
 * The function outputs a character via the ITM channel 0. 
 * The function returns when no debugger is connected that has booked the output.  
 * It is blocking when a debugger is connected, but the previous character send is not transmitted. 
 */
static __INLINE uint32_t ITM_SendChar (uint32_t ch)
{
  if (ch == '\n') ITM_SendChar('\r');
  
  if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA)  &&
      (ITM->TCR & ITM_TCR_ITMENA)                  &&
      (ITM->TER & (1UL << 0))  ) 
  {
    while (ITM->PORT[0].u32 == 0);
    ITM->PORT[0].u8 = (uint8_t) ch;
  }  
  return (ch);
}

#ifdef __cplusplus
}
#endif

#endif /* __CM4_CORE_H__ */

/*lint -restore */
