//*****************************************************************************
//
//! @file emmc_raw_block_read_write.c
//!
//! @brief emmc raw block read and write example.
//!
//! Purpose: This Test Case exercise raw DMA based I/O to an eMMC device.
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//!
//! AM_DEBUG_PRINTF
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//!
//! Note that when these macros are defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "sdio_test_common.h"
#include "unity.h"


//*****************************************************************************
//
// Macro definitions
//

#define FMC_BOARD_EMMC_TEST
//! #define TEST_POWER_SAVING

#define LOOP_COUNT        4 //! SDR_4bit,SDR_8bit,DDR_4bit,DDR_8bit
//
// Test different RAMs
//
#define DSP_RAM0_WORKAROUND

#define DSP_RAM0_START_ADDR 0x10164000
#define SSRAM_START_ADDR    0x10064000

#define START_BLK 3000

#define NUM_OF_BLKS 256  //!. for 128KByte buffers

#define BUFFER_LENGTH 512*NUM_OF_BLKS
#define NUM_WRITES 100  

// at the moment results under 1ms are not reliable.
#define WAKE_INTERVAL_IN_MS     1  
                      
#define XT_PERIOD               32768

#define WAKE_INTERVAL           (XT_PERIOD * WAKE_INTERVAL_IN_MS / 1000)

#ifdef APOLLO4_FPGA
// The FPGA XT appears to run at about 1.5MHz (1.5M / 32K = 48).
#undef  WAKE_INTERVAL
#define WAKE_INTERVAL           (XT_PERIOD * WAKE_INTERVAL_IN_MS / 1000 * 48)
#endif

uint32_t g_ui32Count = 0; 

am_hal_card_host_t *pSdhcCardHost = NULL;

am_hal_card_t eMMCard;

uint8_t *pui8RdBufSSRAM;
uint8_t *pui8WrBufSSRAM;
uint8_t *pui8RdBufDspRam0;
uint8_t *pui8WrBufDspRam0;

uint32_t g_ui32TimerTickBefore;
uint32_t g_ui32TimerTickAfter;

volatile bool bAsyncWriteIsDone = false;
volatile bool bAsyncReadIsDone  = false;

bool bTestPass = true;

//*****************************************************************************
//
// Optional setup/tear-down functions
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
  
}

void
tearDown(void)
{
 
}


//*****************************************************************************
//
//! @brief Check whether read data matches with write data.
//!
//! @param rdbuf ptr to read buffer
//!
//! @param wrbuf ptr to write buffer
//!
//! @param len lenght of data to be compared
//!
//! @return none
//
//*****************************************************************************

void
check_if_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len)
{
    uint32_t i;
    for ( i = 0; i < ui32Len; i++ )
    {
        if ( pui8RdBuf[i] != pui8WrBuf[i] )
        {
            am_util_stdio_printf("pui8RdBuf[%d] = %d and pui8WrBuf[%d] = %d\n", i, pui8RdBuf[i], i, pui8WrBuf[i]);
            am_util_stdio_printf("\ndata match error when reading back \n");
            bTestPass = false;
            break;
        }
    }

    if ( i == ui32Len )
    {
        am_util_stdio_printf("data matched\n");
    }

}




//*****************************************************************************
//
//! @brief Calibration function for DDR50 Memory
//!
//! @param  ui8TxRxDelays - array that holds delay values
//!
//! @return false to force the calibration
//
//***************************************************************************** 

bool
custom_load_ddr50_calib(uint8_t ui8TxRxDelays[2])
{
    //
    // Load ui8TxRxDelays from the somewhere - for example non-volatile memory ...
    // here simply hardcoding these values.
    //
    ui8TxRxDelays[0] = 9;
    ui8TxRxDelays[1] = 6;

    //
    // Return true not to force the calibration
    //
    return false;
}

    
//*****************************************************************************
//
// Init function for Timer A0.
//
//*****************************************************************************
void
stimer_init(void)
{
    //
    // Enable compare A interrupt in STIMER
    //
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(STIMER_CMPR0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(STIMER_CMPR0_IRQn);

    //
    // Configure the STIMER and run
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ |
                         AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);

}


//*****************************************************************************
//
// Timer Interrupt Service Routine (ISR)
//
//*****************************************************************************
void
am_stimer_cmpr0_isr(void)
{
    //
    // Check the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);

    g_ui32Count++;
}



void raw_emmc_write_read_test()
{
    int i,j,k;
    uint32_t start_blk;
    uint32_t num_of_ms;
    uint32_t volume_data_written;
    uint32_t volume_data_read;
    uint32_t Bytes_per_ms;
    uint8_t ui8TxRxDelays[2];
    bool bValid;
    uint32_t test_type;
    uint32_t NumOfBlks;
    uint32_t Buffer_Length;
        
    uint32_t SDR_Or_DDR[4] = {SDR_4bit,SDR_8bit,DDR_4bit,DDR_8bit};
    uint32_t Number_of_Blocks[6] = {4,8,16,32,64,128};
    pui8RdBufSSRAM = (uint8_t *)SSRAM_START_ADDR;
      
    //
    // looping through all 4 scenarios.   
    // SDR_4bit,SDR_8bit,DDR_4bit,DDR_8bit
    //
    
   stimer_init();
    
   am_hal_interrupt_master_enable();
  
   for ( i = 0; i < LOOP_COUNT; i++ )  
   {
     //
    // initialize the test read and write buffers
    //
       for( k = 0; k < 6; k++ )  // 6 different buffer sizes
       {
           NumOfBlks = Number_of_Blocks[k];
           Buffer_Length = NumOfBlks * 512;

           pui8WrBufSSRAM = (uint8_t *)SSRAM_START_ADDR + Buffer_Length;
   
           for ( int i = 0; i < Buffer_Length; i++ )
           {
               pui8WrBufSSRAM[i] = i % 256;
           }

    //
    // Get the uderlying SDHC card host instance
    //
           pSdhcCardHost = am_hal_get_card_host(AM_HAL_SDHC_CARD_HOST, true);

           if (pSdhcCardHost == NULL)
           {
               am_util_stdio_printf("No such card host and stop\n");
               bTestPass = false;
           }
           am_util_stdio_printf("card host is found\n");
    
    //
    // going to rotate through the 4 different memory scenarios
    //

           test_type = SDR_Or_DDR[i];
    
           if ( test_type == DDR_4bit )  //! this has to be done before the eMMC is initialized
           {
              bValid = custom_load_ddr50_calib(ui8TxRxDelays);
              if ( bValid )
              {
                  am_hal_card_host_set_txrx_delay(pSdhcCardHost, ui8TxRxDelays);
              }
              else
              {
                  am_hal_card_emmc_calibrate(AM_HAL_HOST_UHS_DDR50, 50000000, AM_HAL_HOST_BUS_WIDTH_4,  
                   (uint8_t *)pui8WrBufSSRAM, START_BLK, 2, ui8TxRxDelays);                                

                  am_util_stdio_printf("SDIO TX delay - %d, RX Delay - %d\n", ui8TxRxDelays[0], ui8TxRxDelays[1]);
              }
           }
           else if ( test_type == DDR_8bit )
           {
               bValid = custom_load_ddr50_calib(ui8TxRxDelays);
               if ( bValid )
               {
                   am_hal_card_host_set_txrx_delay(pSdhcCardHost, ui8TxRxDelays);
               }
               else
               {
                   am_hal_card_emmc_calibrate(AM_HAL_HOST_UHS_DDR50, 50000000, AM_HAL_HOST_BUS_WIDTH_8,   
                    (uint8_t *)pui8WrBufSSRAM, START_BLK, 2, ui8TxRxDelays);                               

                   am_util_stdio_printf("SDIO TX delay - %d, RX Delay - %d\n", ui8TxRxDelays[0], ui8TxRxDelays[1]);
               }
           }

    //
    // check if card is present
    //
            while ( am_hal_card_host_find_card(pSdhcCardHost, &eMMCard) != AM_HAL_STATUS_SUCCESS )
            {
                am_util_stdio_printf("No card is present now\n");
                am_util_delay_ms(1000);
                am_util_stdio_printf("Checking if card is available again\n");
            }

            while ( am_hal_card_init(&eMMCard, AM_HAL_CARD_TYPE_EMMC, NULL, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) != AM_HAL_STATUS_SUCCESS )
            {
                am_util_delay_ms(1000);
                am_util_stdio_printf("card init failed, try again\n");
            }
  
        switch( test_type )
        {
           case SDR_4bit:
               am_util_stdio_printf("\n testing eMMMC with SDR at 48MHZ, 4 bits\n");
               while ( am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
               AM_HAL_HOST_BUS_WIDTH_4, 48000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
               AM_HAL_HOST_UHS_NONE) != AM_HAL_STATUS_SUCCESS )
               {
                   am_util_delay_ms(1000);
                   am_util_stdio_printf("setting SDR48 4 bit failed\n");
               }
               break;
           case SDR_8bit: 
               am_util_stdio_printf("\n testing eMMMC with SDR at 48MHZ, 8 bits\n");
               while ( am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
               AM_HAL_HOST_BUS_WIDTH_8, 48000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
               AM_HAL_HOST_UHS_NONE) != AM_HAL_STATUS_SUCCESS )
               {
                   am_util_delay_ms(1000);
                   am_util_stdio_printf("setting SDR48 8 bit failed\n");
               }
               break;
           case DDR_4bit:
               am_util_stdio_printf("\n testing eMMMC with DDR at 50MHZ, 4 bits\n");
               while ( am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
               AM_HAL_HOST_BUS_WIDTH_4, 50000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
               AM_HAL_HOST_UHS_DDR50) != AM_HAL_STATUS_SUCCESS )
               {
                   am_util_delay_ms(1000);
                   am_util_stdio_printf("setting DDR50 4 bit failed\n");
              }
              break;
          case DDR_8bit:
             am_util_stdio_printf("\n testing eMMMC with DDR at 50MHZ, 8 bits\n");
             while ( am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
             AM_HAL_HOST_BUS_WIDTH_8, 50000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
             AM_HAL_HOST_UHS_DDR50) != AM_HAL_STATUS_SUCCESS )
             {
                 am_util_delay_ms(1000);
                 am_util_stdio_printf("setting DDR50 8 bit failed\n");
             }
             break;
        default:
             break;
        }
    
        am_util_stdio_printf("\n Multiple block sync writes and reads and the number of 512 byte blocks");
        am_util_stdio_printf("\n being written per write/read cycle is %d \n",Number_of_Blocks[k]);
        g_ui32TimerTickBefore = g_ui32Count; 
        
        for( j=0, start_blk=START_BLK; j < NUM_WRITES; j++, start_blk += NumOfBlks )
        {
             am_hal_card_block_write_sync(&eMMCard, start_blk, NumOfBlks, (uint8_t *)pui8WrBufSSRAM);
        }
        
        g_ui32TimerTickAfter = g_ui32Count;
        num_of_ms = g_ui32TimerTickAfter - g_ui32TimerTickBefore;
        volume_data_written = NUM_WRITES * Buffer_Length;

        Bytes_per_ms = volume_data_written/num_of_ms;  // Bytes per MS
        am_util_stdio_printf("\nMegabytes per second for eMMC writes is %d \n",(Bytes_per_ms/1000));
        
        memset((void *)pui8RdBufSSRAM, 0x0, Buffer_Length);  // clearing read buffer early
        
        am_util_stdio_printf("\ntesting a loop of reads\n");

        g_ui32TimerTickBefore = g_ui32Count;
        
        for( j=0, start_blk=START_BLK; j < NUM_WRITES; j++, start_blk += NumOfBlks )
        {
           am_hal_card_block_read_sync(&eMMCard, start_blk, NumOfBlks, (uint8_t *)pui8RdBufSSRAM);
        }
        
        g_ui32TimerTickAfter = g_ui32Count;
        num_of_ms = g_ui32TimerTickAfter - g_ui32TimerTickBefore;
        volume_data_read = NUM_WRITES * Buffer_Length;

        Bytes_per_ms = volume_data_read/num_of_ms;  // Bytes per ms
        am_util_stdio_printf("\nMegabytes per second for eMMC read is %d \n",(Bytes_per_ms/1000));
         
     //
     // check if block data match or not
     //
        check_if_data_match((uint8_t *)pui8RdBufSSRAM, (uint8_t *)pui8WrBufSSRAM, Buffer_Length);
        am_util_stdio_printf("============================================================\n\n");
        am_util_delay_ms(1000);  // wait between cycles
   
     }  // big for loop for 6 different buffer sizes
     
  }  // end big for loop for SDR_4bit,SDR_8bit,DDR_4bit,DDR_8bit

  TEST_ASSERT_TRUE(bTestPass);
}