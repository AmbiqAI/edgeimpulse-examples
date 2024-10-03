//*****************************************************************************
//
//! @file burst_mode_task.c
//!
//! @brief Example test case program.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "freertos_fit_test_cases.h"

#define NUM_OF_PRIMES_IN 5000
#define EXP_PRIMES 9592
#define BURST_TIMER_FREQ 5
//*****************************************************************************
//
// Timer handling to emulate sensor data
//
//*****************************************************************************
static am_hal_ctimer_config_t g_sBurstTimer =
{
    // Don't link timers.
    0,

    // Set up TimerA.
    (AM_HAL_CTIMER_FN_REPEAT |
     AM_HAL_CTIMER_INT_ENABLE    |
     AM_HAL_CTIMER_HFRC_12KHZ),

    // No configuration for TimerB.
    0,
};

//*****************************************************************************
//
// Burst mode task handle.
//
//*****************************************************************************
TaskHandle_t Burst_mode_task_handle;
bool g_bBurstModeTest = false;


void burst_timer_handler(void)
{
    g_bBurstModeTest = true;
}

void stop_burst_timer(void)
{
    //
    // Stop timer A1
    //
    am_hal_ctimer_stop(1, AM_HAL_CTIMER_TIMERA);
}

void start_burst_timer(void)
{
    stop_burst_timer(); // Just in case host died without sending STOP last time
    //
    // Start timer A1
    //
    am_hal_ctimer_start(1, AM_HAL_CTIMER_TIMERA);
}

void init_burst_timer(void)
{
    uint32_t ui32Period;

    //
    // Set up timer A1.
    //
    am_hal_ctimer_clear(1, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_config(1, &g_sBurstTimer);

    //
    // Set up timerA1 for Burst Timer Freq
    //
    ui32Period = 12000 / BURST_TIMER_FREQ ;
    am_hal_ctimer_period_set(1, AM_HAL_CTIMER_TIMERA, ui32Period,
                             (ui32Period >> 1));

    //
    // Clear the timer Interrupt
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA1);

    //
    // Enable the timer Interrupt.
    //
    am_hal_ctimer_int_register(AM_HAL_CTIMER_INT_TIMERA1,
                               burst_timer_handler);
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA1);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_EnableIRQ(CTIMER_IRQn);
    am_hal_interrupt_master_enable();
}

//*****************************************************************************
//
// Init function for the STimer.
//
//*****************************************************************************
void
stimer_init(void)
{
    //
    // Configure the STIMER and run
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ);

}

//*****************************************************************************
//
//  Purpose:
//
//    prime_number() returns the number of primes between 1 and N.
//
//  Discussion:
//
//    A naive algorithm is used.
//
//    Mathematica can return the number of primes less than or equal to N
//    by the command PrimePi[N].
//
//                N  PRIME_NUMBER
//
//                1           0
//               10           4
//              100          25
//            1,000         168
//           10,000       1,229
//          100,000       9,592
//        1,000,000      78,498
//       10,000,000     664,579
//      100,000,000   5,761,455
//    1,000,000,000  50,847,534
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    23 April 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the maximum number to check.
//
//    Output, int PRIME_NUMBER, the number of prime numbers up to N.
//
//*****************************************************************************
uint32_t
prime_number(int32_t i32n)
{
    uint32_t ui32Total, ui32Prime;
    int32_t ix, jx;
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

    ui32Total = 0;

    for ( ix = 2; ix <= i32n; ix++ )
    {
        ui32Prime = 1;
        for ( jx = 2; jx < ix; jx++ )
        {
            if ( (ix % jx) == 0 )
            {
                ui32Prime = 0;
                break;
            }
        }
        ui32Total += ui32Prime;
        
        if (0 == (ix%500))
        {
          vTaskDelay(xDelay);
        }
    }
    return ui32Total;
}

void BurstmodeSetup(void)
{
    uint32_t                      ui32Retval;
    am_hal_burst_avail_e          eBurstModeAvailable;

    // Initialize the STimer.
    stimer_init();
    
    // Check that the Burst Feature is available.
    ui32Retval = am_hal_burst_mode_initialize(&eBurstModeAvailable);
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == ui32Retval);
    TEST_ASSERT_TRUE(AM_HAL_BURST_AVAIL == eBurstModeAvailable);
}

//*****************************************************************************
//
// Test cases for the main program to run.
//
//*****************************************************************************
void
BurstmodeTask(void *pvParameters)
{
    uint32_t                      ui32Retval;
    uint32_t                      ui32StartTime, ui32StopTime;
    uint32_t                      ui32BurstModeDelta, ui32NormalModeDelta;
    am_hal_burst_mode_e           eBurstMode;
    const TickType_t xDelay = 5000 / portTICK_PERIOD_MS;

    am_util_stdio_printf("Burst Mode Task\r\n");

    init_burst_timer();
    start_burst_timer();

    while(1)
    {
        if(g_bBurstModeTest)
        {
            g_bBurstModeTest = false;
            
            // Make sure we are in "Normal" mode.
            ui32Retval = am_hal_burst_mode_disable(&eBurstMode);
            TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == ui32Retval);
            TEST_ASSERT_TRUE(AM_HAL_NORMAL_MODE == eBurstMode);
            // Capture the start time.
            ui32StartTime = am_hal_stimer_counter_get();

            // Execute the example algorithm.
            //am_util_stdio_printf("\nStarted calculating primes with Burst Mode off\n");
            prime_number(NUM_OF_PRIMES_IN);
            //am_util_stdio_printf("Finished calculating primes with Burst Mode off\n");

            // Stop the timer and calculate the elapsed time.
            ui32StopTime = am_hal_stimer_counter_get();

            // Calculate the Burst Mode delta time.
            ui32NormalModeDelta = ui32StopTime - ui32StartTime;
            //am_util_stdio_printf("Normal Mode Delta: %d\n",ui32NormalModeDelta);
            
            // Put the MCU into "Burst" mode.
            TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_burst_mode_enable(&eBurstMode));
            TEST_ASSERT_TRUE(AM_HAL_BURST_MODE == eBurstMode);

            // Capture the start time.
            ui32StartTime = am_hal_stimer_counter_get();

            // Execute the example algorithm.
            //am_util_stdio_printf("\nStarted calculating primes with Burst Mode on\n");
            prime_number(NUM_OF_PRIMES_IN);
            //am_util_stdio_printf("Finished calculating primes with Burst Mode on\n");

            // Stop the timer and calculate the elapsed time.
            ui32StopTime = am_hal_stimer_counter_get();

            // Disable Burst Mode.
            TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_burst_mode_disable(&eBurstMode));
            TEST_ASSERT_TRUE(AM_HAL_NORMAL_MODE == eBurstMode);

            // Calculate the Burst Mode delta time.
            ui32BurstModeDelta = ui32StopTime - ui32StartTime;
            //am_util_stdio_printf("Burst Mode Delta: %d\n",ui32BurstModeDelta);

            // Evaluate the ratio.
            // TEST_ASSERT_TRUE( (ui32BurstModeDelta < (0.55 * ui32NormalModeDelta)) );
            AM_UPDATE_TEST_PROGRESS();
            am_util_stdio_printf("BURST\n");
        }
        vTaskDelay(xDelay);
    }
}


