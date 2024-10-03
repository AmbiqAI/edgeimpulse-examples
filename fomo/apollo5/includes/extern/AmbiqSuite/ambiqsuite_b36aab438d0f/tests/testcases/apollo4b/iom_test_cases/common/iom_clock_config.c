//*****************************************************************************
//
//! @file iom_clock_config.c
//!
//! @brief Allow changes in IOM sclk speeds
//!
//!
//! @addtogroup devices External Device Control Library
//! @addtogroup ADXL363 SPI Device Control for the ADXL363 External Accelerometer
//! @ingroup examples
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//****************************************************************************
#include "am_mcu_apollo.h"
#include "iom_clock_config.h"

#ifdef __IAR_SYSTEMS_ICC__
#define AM_INSTR_CLZ(n)                     __CLZ(n)
#else
#define AM_INSTR_CLZ(n)                     __builtin_clz(n)
#endif

typedef struct
{
    bool        bValid;
    uint32_t    regFIFOTHR;
    uint32_t    regDMATRIGEN;
    uint32_t    regCLKCFG;
    uint32_t    regSUBMODCTRL;
    uint32_t    regCQCFG;
    uint32_t    regCQADDR;
    uint32_t    regCQFLAGS;
    uint32_t    regCQPAUSEEN;
    uint32_t    regCQCURIDX;
    uint32_t    regCQENDIDX;
    uint32_t    regMSPICFG;
    uint32_t    regMI2CCFG;
    uint32_t    regINTEN;
}
am_hal_iom_register_state_t;
#define AM_HAL_IOM_MAX_PENDING_TRANSACTIONS      256 // Must be power of 2 for the implementation below

typedef enum
{
    AM_HAL_IOM_SEQ_NONE,
    AM_HAL_IOM_SEQ_UNDER_CONSTRUCTION,
    AM_HAL_IOM_SEQ_RUNNING,
}
am_hal_iom_seq_e;

typedef struct
{
    am_hal_handle_prefix_t  prefix;

    //
    // Physical module number.
    //
    uint32_t                ui32Module;

    //
    // Interface mode (SPI or I2C).
    //
    am_hal_iom_mode_e       eInterfaceMode;

    //
    // Non-Blocking transaction Tranfer Control Buffer.
    //
    uint32_t                *pNBTxnBuf;
    uint32_t                ui32NBTxnBufLength;

    //
    // Saves the user application defined interrupt configuration.
    //
    uint32_t                ui32UserIntCfg;

    //
    // Saves the transaction interrupt state for non-blocking interrupt service.
    //
    uint32_t                ui32TxnInt;

    //
    // Index of last non-blocking transaction processed in CQ.
    //
    uint32_t                ui32LastIdxProcessed;

    // Maximum number of transactions allowed in the CQ.
    uint32_t                ui32MaxTransactions;

    //
    // Number of pending transactions in the CQ.
    //
    volatile uint32_t       ui32NumPendTransactions;

    //
    // Stores the CQ callbacks and contexts.
    //
    am_hal_iom_callback_t   pfnCallback[AM_HAL_IOM_MAX_PENDING_TRANSACTIONS];
    void                    *pCallbackCtxt[AM_HAL_IOM_MAX_PENDING_TRANSACTIONS];

    //
    // Handle to the CQ.
    //
    void                    *pCmdQHdl;

    //
    // To support sequence.
    //
    am_hal_iom_seq_e        eSeq;
    bool                    bAutonomous;

    //
    // This is used to track the number of transactions in a sequence.
    //
    uint32_t                ui32NumSeqTransactions;
    volatile bool           bRestart;
    uint32_t                block;

    //
    // To support high priority transactions - out of band
    // High Priority DMA transactions
    //
    volatile bool           bHP;
    uint32_t                ui32NumHPEntries;
    uint32_t                ui32NumHPPendingEntries;
    uint32_t                ui32MaxHPTransactions;
    uint32_t                ui32NextHPIdx;
    uint32_t                ui32LastHPIdxProcessed;
    void                     *pHPTransactions; // change
    // Max pending transactions based on NB Buffer size
    uint32_t                ui32MaxPending;
    // Number of back to back transactions with no callbacks
    uint32_t                ui32NumUnSolicited;
    //
    // Delay timeout value.
    //
    uint32_t                waitTimeout;

    //
    // Configured clock time.
    //
    uint32_t                ui32BitTimeUs;

    //
    // IOM register state for power down save/restore.
    //
    am_hal_iom_register_state_t registerState;

    //
    // DCX state.
    //
    uint8_t                 dcx[AM_HAL_IOM_MAX_CS_SPI];
    bool                    bCmdQInTCM;

}
am_hal_iom_state_t;



static bool onebit(uint32_t ui32Value);
static uint32_t compute_freq(uint32_t ui32HFRCfreqHz,
                             uint32_t ui32Fsel,
                             uint32_t ui32Div3,
                             uint32_t ui32DivEn,
                             uint32_t ui32TotPer);

static uint64_t iom_get_interface_clock_cfg(uint32_t ui32FreqHz,
                                            uint32_t ui32Phase );

static uint32_t am_iom_clock_check_busy(am_hal_iom_state_t *pIOMState);


//
// holds value of last frequency used for each of the IOM channels
//
static uint32_t g_ui32LastClockFreq[AM_REG_IOM_NUM_MODULES];


//*****************************************************************************
//
//! @brief Return true if ui32Value has exactly 1 bit set, otherwise false.
//!
//! @param ui32Value
//! @return Return true if ui32Value has exactly 1 bit set, otherwise false.
//
//*****************************************************************************
static bool
onebit(uint32_t ui32Value)
{
    return ui32Value  &&  !(ui32Value & (ui32Value - 1));
} // onebit()


//*****************************************************************************
//
//! @brief compute_freq
//!
//! @note Compute the interface frequency based on the given parameters
//!
//! @param ui32HFRCfreqHz
//! @param ui32Fsel
//! @param ui32Div3
//! @param ui32DivEn
//! @param ui32TotPer
//!
//! @return com
//
//*****************************************************************************
static uint32_t
compute_freq(uint32_t ui32HFRCfreqHz,
             uint32_t ui32Fsel,
             uint32_t ui32Div3,
             uint32_t ui32DivEn,
             uint32_t ui32TotPer)
{
    uint32_t ui32Denomfinal, ui32ClkFreq;

    ui32Denomfinal = ((1 << (ui32Fsel - 1)) * (1 + ui32Div3 * 2) * (1 + ui32DivEn * (ui32TotPer)));
    ui32ClkFreq = (ui32HFRCfreqHz) / ui32Denomfinal;                           // Compute the set frequency value
    ui32ClkFreq +=  (((ui32HFRCfreqHz) % ui32Denomfinal) > (ui32Denomfinal / 2)) ? 1 : 0;

    return ui32ClkFreq;
} // compute_freq()

//*****************************************************************************
//
//! @brief iom_get_interface_clock_cfg
//!
//! @note Compute the interface frequency based on the given parameters
//!
//! @param ui32FreqHz the desired clock freq
//! @param ui32Phase  when one high phase is longer
//!
//! @return generic return status
//
//*****************************************************************************

static uint64_t
iom_get_interface_clock_cfg(uint32_t ui32FreqHz, uint32_t ui32Phase )
{
    uint32_t ui32Fsel, ui32Div3, ui32DivEn, ui32TotPer, ui32LowPer;
    uint32_t ui32Denom, ui32v1, ui32Denomfinal, ui32ClkFreq, ui32ClkCfg;
    uint32_t ui32HFRCfreqHz ;
    int32_t i32Div ;
    int32_t i32N ;

    if (ui32FreqHz == 0)
    {
        return 0;
    }

    //
    // Set the HFRC clock frequency.
    //
    //    ui32HFRCfreqHz = AM_HAL_CLKGEN_FREQ_MAX_HZ;
    ui32HFRCfreqHz = 96000000;  // TODO - FIXME: Repair once CLKGEN is included.

    //
    // Compute various parameters used for computing the optimal CLKCFG setting.
    //
    i32Div = (ui32HFRCfreqHz / ui32FreqHz) + ((ui32HFRCfreqHz % ui32FreqHz) ? 1 : 0);    // Round up (ceiling)

    //
    // Compute N (count the number of LS zeros of Div) = ctz(Div) = log2(Div & (-Div))
    //
    i32N = 31 - AM_INSTR_CLZ((i32Div & (-i32Div)));

    if (i32N > 6)
    {
        i32N = 6;
    }

    ui32Div3   = ((ui32FreqHz < (ui32HFRCfreqHz / 16384)) ||
                           (((ui32FreqHz >= (ui32HFRCfreqHz / 3)) &&
                             (ui32FreqHz <= ((ui32HFRCfreqHz / 2) - 1))))) ? 1 : 0;
    ui32Denom  = (1 << i32N) * (1 + (ui32Div3 * 2));
    ui32TotPer = i32Div / ui32Denom;
    ui32TotPer += (i32Div % ui32Denom) ? 1 : 0;
    ui32v1   = 31 - AM_INSTR_CLZ(ui32TotPer);     // v1 = log2(TotPer)
    ui32Fsel = (ui32v1 > 7) ? ui32v1 + i32N - 7 : i32N;
    ui32Fsel++;

    if (ui32Fsel > 7)
    {
        //
        // This is an error, can't go that low.
        //
        return 0;
    }

    if (ui32v1 > 7)
    {
        ui32DivEn  = ui32TotPer;     // Save TotPer for the round up calculation
        ui32TotPer = ui32TotPer >> (ui32v1 - 7);
        ui32TotPer += ((ui32DivEn) % (1 << (ui32v1 - 7))) ? 1 : 0;
    }

    ui32DivEn = ((ui32FreqHz >= (ui32HFRCfreqHz / 4)) ||
                 ((1 << (ui32Fsel - 1)) == i32Div)) ? 0 : 1;

    if (ui32Phase == 1)
    {
        ui32LowPer = (ui32TotPer - 2) / 2;          // Longer high phase
    }
    else
    {
        ui32LowPer = (ui32TotPer - 1) / 2;          // Longer low phase
    }

    ui32ClkCfg = _VAL2FLD(IOM0_CLKCFG_FSEL, ui32Fsel) |
                          _VAL2FLD(IOM0_CLKCFG_DIV3, ui32Div3) |
                          _VAL2FLD(IOM0_CLKCFG_DIVEN, ui32DivEn) |
                          _VAL2FLD(IOM0_CLKCFG_LOWPER, ui32LowPer) |
                          _VAL2FLD(IOM0_CLKCFG_TOTPER, ui32TotPer - 1);

    //
    // Now, compute the actual frequency, which will be returned.
    //
    ui32ClkFreq = compute_freq(ui32HFRCfreqHz, ui32Fsel, ui32Div3, ui32DivEn, ui32TotPer - 1);

    //
    // Determine if the actual frequency is a power of 2 (MHz).
    //
    if ((ui32ClkFreq % 250000) == 0)
    {
        //
        // If the actual clock frequency is a power of 2 ranging from 250KHz up,
        // we can simplify the CLKCFG value using DIV3 (which also results in a
        // better duty cycle).
        //
        ui32Denomfinal = ui32ClkFreq / (uint32_t) 250000;

        if (onebit(ui32Denomfinal))
        {
            //
            // These configurations can be simplified by using DIV3.  Configs
            // using DIV3 have a 50% duty cycle, while those from DIVEN will
            // have a 66/33 duty cycle.
            //
            ui32TotPer = ui32LowPer = ui32DivEn = 0;
            ui32Div3   = 1;

            //
            // Now, compute the return values.
            //
            ui32ClkFreq = compute_freq(ui32HFRCfreqHz, ui32Fsel, ui32Div3, ui32DivEn, ui32TotPer);

            ui32ClkCfg = _VAL2FLD(IOM0_CLKCFG_FSEL, ui32Fsel)  |
                         _VAL2FLD(IOM0_CLKCFG_DIV3, 1)   |
                         _VAL2FLD(IOM0_CLKCFG_DIVEN, 0)  |
                         _VAL2FLD(IOM0_CLKCFG_LOWPER, 0) |
                         _VAL2FLD(IOM0_CLKCFG_TOTPER, 0);
        }
    }

    return (((uint64_t) ui32ClkFreq) << 32) | (uint64_t) ui32ClkCfg;

} //iom_get_interface_clock_cfg()


//*****************************************************************************
//
//! @brief configure spi/i2c clock speed
//!
//! @param pHandle
//! @param psConfig
//!
//!
//! @return
//
//*****************************************************************************
uint32_t
am_hal_iom_configure_clk_tst(void *pHandle, const am_hal_iom_config_t *psConfig )
{
    uint32_t            ui32ClkCfg;
    am_hal_iom_state_t  *pIOMState;
    uint32_t            status;
    uint32_t            ui32Module;
    uint32_t            ui32ClockFreq ;

    pIOMState = (am_hal_iom_state_t *) pHandle;

    ui32Module = pIOMState->ui32Module;
    status     = AM_HAL_STATUS_SUCCESS;

    //ui32Module = pIOMState->ui32Module;
    //
    // Save the interface mode and chip select in the global handle.
    //
    pIOMState->eInterfaceMode = psConfig->eInterfaceMode;

    ui32ClockFreq             = psConfig->ui32ClockFreq;

    if (psConfig->eInterfaceMode == AM_HAL_IOM_SPI_MODE)
    {
#ifndef AM_HAL_DISABLE_API_VALIDATION
        //
        // Validate the SPI mode
        //
        if (psConfig->eSpiMode > AM_HAL_IOM_SPI_MODE_3)
        {
            return AM_HAL_STATUS_INVALID_ARG;
        }
        if (ui32ClockFreq > AM_HAL_IOM_MAX_FREQ)
        {
            return AM_HAL_STATUS_INVALID_ARG;
        }
#endif // AM_HAL_DISABLE_API_VALIDATION

        if (g_ui32LastClockFreq[ui32Module] == psConfig->ui32ClockFreq)
        {
            //
            // no change
            //
            return status;
        }

        //
        // ensure iom isn't busy with something else
        //
        status = am_iom_clock_check_busy(pIOMState);
        if (status)
        {
            return status;
        }

        g_ui32LastClockFreq[ui32Module] = ui32ClockFreq;

        //
        // Determine the CLKCFG value for SPI.
        //
        ui32ClkCfg = iom_get_interface_clock_cfg(ui32ClockFreq, (psConfig->eSpiMode & 2) >> 1);
    }
    else if (psConfig->eInterfaceMode == AM_HAL_IOM_I2C_MODE)
    {
        return AM_HAL_STATUS_INVALID_ARG;
#ifdef GONE  // turn off I2C
        switch (psConfig->ui32ClockFreq)
        {
            case AM_HAL_IOM_100KHZ:
                //
                // settings below should give ~100 kHz
                //
                ui32ClkCfg = _VAL2FLD(IOM0_CLKCFG_TOTPER, 0x77)                     |
                             _VAL2FLD(IOM0_CLKCFG_LOWPER, 0x3B)                     |
                             _VAL2FLD(IOM0_CLKCFG_DIVEN, IOM0_CLKCFG_DIVEN_EN)      |
                             _VAL2FLD(IOM0_CLKCFG_DIV3, IOM0_CLKCFG_DIV3_DIS)       |
                             _VAL2FLD(IOM0_CLKCFG_FSEL, IOM0_CLKCFG_FSEL_HFRC24MHZ) |
                             _VAL2FLD(IOM0_CLKCFG_IOCLKEN, 1);
                IOMn(ui32Module)->MI2CCFG = _VAL2FLD(IOM0_MI2CCFG_STRDIS, 0)                            |
                                            _VAL2FLD(IOM0_MI2CCFG_SMPCNT, 3)                            |
                                            _VAL2FLD(IOM0_MI2CCFG_SDAENDLY, 15)                         |
                                            _VAL2FLD(IOM0_MI2CCFG_SCLENDLY, 0)                          |
                                            _VAL2FLD(IOM0_MI2CCFG_MI2CRST, 1)                           |
                                            _VAL2FLD(IOM0_MI2CCFG_SDADLY, 3)                            |
                                            _VAL2FLD(IOM0_MI2CCFG_ARBEN, IOM0_MI2CCFG_ARBEN_ARBDISABLE)     |
                                            _VAL2FLD(IOM0_MI2CCFG_I2CLSB, IOM0_MI2CCFG_I2CLSB_MSBFIRST) |
                                            _VAL2FLD(IOM0_MI2CCFG_ADDRSZ, IOM0_MI2CCFG_ADDRSZ_ADDRSZ7);
                break;
            case AM_HAL_IOM_400KHZ:
                //
                // settings below should give ~400 kHz
                //
                ui32ClkCfg = _VAL2FLD(IOM0_CLKCFG_TOTPER, 0x1D)                     |
                             _VAL2FLD(IOM0_CLKCFG_LOWPER, 0x0E)                     |
                             _VAL2FLD(IOM0_CLKCFG_DIVEN, IOM0_CLKCFG_DIVEN_EN)      |
                             _VAL2FLD(IOM0_CLKCFG_DIV3, IOM0_CLKCFG_DIV3_DIS)       |
                             _VAL2FLD(IOM0_CLKCFG_FSEL, IOM0_CLKCFG_FSEL_HFRC24MHZ) |
                             _VAL2FLD(IOM0_CLKCFG_IOCLKEN, 1);
                IOMn(ui32Module)->MI2CCFG = _VAL2FLD(IOM0_MI2CCFG_STRDIS, 0)                            |
                                            _VAL2FLD(IOM0_MI2CCFG_SMPCNT, 3)                            |
                                            _VAL2FLD(IOM0_MI2CCFG_SDAENDLY, 15)                         |
                                            _VAL2FLD(IOM0_MI2CCFG_SCLENDLY, 2)                          |
                                            _VAL2FLD(IOM0_MI2CCFG_MI2CRST, 1)                           |
                                            _VAL2FLD(IOM0_MI2CCFG_SDADLY, 3)                            |
                                            _VAL2FLD(IOM0_MI2CCFG_ARBEN, IOM0_MI2CCFG_ARBEN_ARBDISABLE)     |
                                            _VAL2FLD(IOM0_MI2CCFG_I2CLSB, IOM0_MI2CCFG_I2CLSB_MSBFIRST) |
                                            _VAL2FLD(IOM0_MI2CCFG_ADDRSZ, IOM0_MI2CCFG_ADDRSZ_ADDRSZ7);
                break;
            case AM_HAL_IOM_1MHZ:
                //
                // settings below should give ~860 kHz
                //
// #### INTERNAL BEGIN ####
                // For 1 MHz - we need to set SMPCNT to 2, as otherwise for IOM 1,3,4 we get much lower clock
// #### INTERNAL END ####
                ui32ClkCfg = _VAL2FLD(IOM0_CLKCFG_TOTPER, 0x0B)                     |
                             _VAL2FLD(IOM0_CLKCFG_LOWPER, 0x05)                     |
                             _VAL2FLD(IOM0_CLKCFG_DIVEN, IOM0_CLKCFG_DIVEN_EN)      |
                             _VAL2FLD(IOM0_CLKCFG_DIV3, IOM0_CLKCFG_DIV3_DIS)       |
                             _VAL2FLD(IOM0_CLKCFG_FSEL, IOM0_CLKCFG_FSEL_HFRC24MHZ) |
                             _VAL2FLD(IOM0_CLKCFG_IOCLKEN, 1);
                IOMn(ui32Module)->MI2CCFG = _VAL2FLD(IOM0_MI2CCFG_STRDIS, 0)                            |
                                            _VAL2FLD(IOM0_MI2CCFG_SMPCNT, 0x2)                         |
                                            _VAL2FLD(IOM0_MI2CCFG_SDAENDLY, 3)                          |
                                            _VAL2FLD(IOM0_MI2CCFG_SCLENDLY, 0)                          |
                                            _VAL2FLD(IOM0_MI2CCFG_MI2CRST, 1)                           |
                                            _VAL2FLD(IOM0_MI2CCFG_SDADLY, 0)                            |
                                            _VAL2FLD(IOM0_MI2CCFG_ARBEN, IOM0_MI2CCFG_ARBEN_ARBDISABLE)     |
                                            _VAL2FLD(IOM0_MI2CCFG_I2CLSB, IOM0_MI2CCFG_I2CLSB_MSBFIRST) |
                                            _VAL2FLD(IOM0_MI2CCFG_ADDRSZ, IOM0_MI2CCFG_ADDRSZ_ADDRSZ7);
                break;
            default:
                return AM_HAL_STATUS_INVALID_ARG;
        }
#endif
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Enable and set the clock configuration.
    //
    IOMn(ui32Module)->CLKCFG &= ~(_VAL2FLD(IOM0_CLKCFG_IOCLKEN, 1));

    IOMn(ui32Module)->CLKCFG = ui32ClkCfg;

    // @todo try this next  IOMn(ui32Module)->CLKCFG = ui32ClkCfg | (_VAL2FLD(IOM0_CLKCFG_IOCLKEN, 1));
    //IOMn(ui32Module)->CLKCFG |= (_VAL2FLD(IOM0_CLKCFG_IOCLKEN, 1));

    IOMn(ui32Module)->CLKCFG = ui32ClkCfg | (_VAL2FLD(IOM0_CLKCFG_IOCLKEN, 1));

    //
    // Get the current clock frequency and calculate bit time based on it.
    // NOTE: This will work only if I2C freq <= 1MHz, as the case currently
    //
    pIOMState->ui32BitTimeUs = 1000000 / psConfig->ui32ClockFreq;

    //
    // Set the delay timeout value to the default maximum value.
    //
    pIOMState->waitTimeout = 1000;

    //
    // apparently need a small delay to ensure clock has time to switch?
    //
    am_hal_delay_us(10);

    //
    // Return the status.
    //
    return status;

} // am_hal_iom_configure_clk_tst()

//*****************************************************************************
//
//! @brief check if iom channel is busy
//!
//! @param pIOMState        - pointer to device handle
//!
//! @return non zero if IOM channel is busy
//
//*****************************************************************************
static uint32_t
am_iom_clock_check_busy(am_hal_iom_state_t *pIOMState)
{

    uint32_t ui32Module = pIOMState->ui32Module ;
    //
    // Make sure any previous non-blocking transfers have completed.
    //
    uint32_t ui32Status = am_hal_delay_us_status_check(pIOMState->waitTimeout,
                                              (uint32_t)&pIOMState->ui32NumPendTransactions,
                                              0xFFFFFFFF,
                                              0,
                                              true);
    if (ui32Status == AM_HAL_STATUS_SUCCESS)
    {
        //
        // Make sure any previous blocking transfer has been completed.
        // This check is required to make sure previous transaction has cleared if the blocking call
        // finished with a timeout
        //
        ui32Status = am_hal_delay_us_status_check(pIOMState->waitTimeout,
                                                  (uint32_t) &IOMn(ui32Module)->STATUS,
                                                  (IOM0_STATUS_IDLEST_Msk | IOM0_STATUS_CMDACT_Msk),
                                                  IOM0_STATUS_IDLEST_Msk,
                                                  true);

    }

    return ui32Status ;
} // am_iom_clock_check_busy

//*****************************************************************************
//
//! @brief set the inital clock speed
//!
//! @note calling normal IOM config, this global clock history parameter
//! is not populated, call this to populate the field
//!
//! @param ui32Module        - IOM module number
//! @param ui32ClockFreq     - current IOM clock freq
//!
//! @return generic call status
//
//*****************************************************************************
uint32_t
am_hal_iom_set_intial_clk(uint32_t  ui32Module, uint32_t ui32ClockFreq)
{

    if ( ui32Module >= AM_REG_IOM_NUM_MODULES || ui32ClockFreq == 0 )
    {
        return AM_HAL_STATUS_INVALID_ARG ;
    }

    g_ui32LastClockFreq[ui32Module] = ui32ClockFreq ;

    return AM_HAL_STATUS_SUCCESS ;
} // am_hal_iom_set_intial_clk
