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
*
*      Revision            : $Revision: 141832 $
*
*      Release information : cortexm4_r0p1_00rel0
*-----------------------------------------------------------------------------
*/

////////////////////////////////////////////////////////////////////////////////
//
// Cortex-M4 Integration Kit ETM trace test
//
////////////////////////////////////////////////////////////////////////////////
//
// If DBG is present, this test checks:
//
// Generation of trace data from ETM
// Generation of Data value trace using DWT
//
// The debug core is used to sample and interpret trace data
//
// Trace output is using TracePort mode

#include "cm4ikmcu.h"
#include "core_cm4.h"
#include "MCUConfig.h"

//#include "IKtests.h"
//#include "am_util.h"


//
// Prototypes
//

void Start(void);
void Stop(void);

//
// Global Variables to track test progress
//

volatile uint32_t Errors = 0;
volatile uint32_t ExpectHardFault = 0;
volatile uint32_t HardFaultTaken = 0;

__IO uint32_t testdata = 0;

typedef void (* FuncPtr)(void);

int etm_trace (void)
{
  uint32_t tmp;
  
  // Enable Trace and TPIU
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA;
    
  tmp = TPIU->PID3;
  tmp = (tmp<<8) | TPIU->PID2;
  tmp = (tmp<<8) | TPIU->PID1;
  tmp = (tmp<<8) | TPIU->PID0;
//  MSG(("TPIU Peripheral ID is %08x. Part %03x, Revison %x\n",tmp,tmp&0xFFF, (tmp>>20)& 0xF));
  
  // Configure TPIU for trace port mode, 4 bits
  TPIU->CSPSR = (1<<3);
  TPIU->SPPR = TPIU_PIN_TRACEPORT;
  
  // Configure trace capture block after setting protocol
  // Set up control value for trace capture block
  //DEBUGDRIVERDATA[0] = TRACE_CONFIG_PORT4;
  //CallDebugDriver(FnTrace);
  
  // Enable synchronisation counter for TPIU
  DWT->CTRL |= DWT_CTRL_CYCCNTENA | DWT_CTRL_SYNCTAP24;
  
  // Enable ETM
  tmp = ETM->PID3;
  tmp = (tmp<<8) | ETM->PID2;
  tmp = (tmp<<8) | ETM->PID1;
  tmp = (tmp<<8) | ETM->PID0;
//  MSG(("ETM  Peripheral ID is %08x. Part %03x, Revison %x\n",tmp,tmp&0xFFF, (tmp>>20)& 0xF));
  ETM->LAR = CS_UNLOCK;
  
  // Power Up ETM and initialise registers
  ETM->CR &= ~ETM_CR_PWRDN;
  ETM->CR |= ETM_CR_PROGBIT | ETM_CR_ETMEN | ETM_CR_TSEN;
  ETM->FFLR  = 0;                           // FifoFull not used
  ETM->CNTRLDVR1 = 0;                       // Counter1 not used
  ETM->TSEVR = ETM_EVT_NOTA | ETM_EVT_TRUE; // timestamp event never
  ETM->TRIGGER = ETM_EVT_A  | ETM_EVT_DWT0; // Trigger on DWT0
  ETM->TEEVR = ETM_EVT_A | ETM_EVT_TRUE;    // Trace Always
  ETM->TRACEIDR = CM4IKMCU_ETM_ID;          // 7 bit trace ID. Must be valid
  
  // At least there is one DWT comparator, use as trigger
  DWT->COMP0 = (uint32_t) &testdata; // testdata address
  DWT->MASK0 = 0x2;                  // Mask(0xFFFFFFFC) for Start address
  DWT->FUNCTION0 = DWT_FUNC_TRIG_WR; // DWT_ETM_TRIG_WR, Function set to Trigger on data write.
  
  // if EXPECTED_WPT = 4, 4 DWT comparators exist,
  // use COMP1 and COMP2 to set TraceEnable Start/Stop Embedded ICE control register
  if (EXPECTED_WPT == 4)
  {
    ETM->TECR1 |= ETM_TECR1_USE_SS;           // TraceEnable address range comparators
    ETM->TESSEICR  = (ETM_TESSEICR_ICE2 << ETM_TESSEICR_STOP) | ETM_TESSEICR_ICE1;
    // initialize the TraceEnable Start/Stop Embedded ICE control register
    
    // set DWT start/stop address
    DWT->COMP1 = (uint32_t)&Start; // start address
    DWT->MASK1 = 0x2;              // Mask(0xFFFFFFFC) for Start address
    DWT->COMP2 = (uint32_t)&Stop;  // stop address
    DWT->MASK2 = 0x2;              // Mask(0xFFFFFFFC) for Stop address
    DWT->FUNCTION1 = DWT_FUNC_TRIG_PC; // Function set to Trigger on PC match
    DWT->FUNCTION2 = DWT_FUNC_TRIG_PC; // Function set to Trigger on PC match
  }
  else
  {
    ETM->TECR1 &= ~ETM_TECR1_USE_SS;
    ETM->TESSEICR = 0x0;
  }
  
  // Clear programming bit
  //MSG(("Tracing is now enabled\n"));
  ETM->CR &= ~ETM_CR_PROGBIT;
  
  //
  // Interrupts
  //
  
  // Ensure CPU interrupts are enabled (PRIMASK clear)
  __enable_irq();
  
  // INT0 is asserted by the GPIO interrupt on pin change feature
  // Use loopback connection on internal GPIO to create the stimulus
  //
  
  // Trace start address has been set to the address of Function Start(),
  // therefore, the ETM trace starts here.
  Start();
  // Ensure Loopback output pin is low
  //GPIO1->DATA[GPIO_LOOPOUT].BYTE[INTDBG_BYTE] = 0;
  
  // Enable GPIO interrupt on change feature
  //GPIO1->IE.BYTE[INTDBG_BYTE] = GPIO_LOOPIN;
  
  // Enable interrupt
  NVIC_EnableIRQ(GPIO_IRQn);
  
  // Generate the interrupt
  //GPIO1->DATA[GPIO_LOOPOUT].BYTE[INTDBG_BYTE] = GPIO_LOOPOUT;
  
  // Wait for interrupt
  //__WFI();
  
  // Generate timestamp
  __ISB();
  
  // check ETM status register start/stop bit.
  // It should be set if EXPECTED_WPT == 4 and start/stop is programmed
  // otherwise, it should be clear.
  tmp = ETM->SR & ETM_SR_SSTOP;
  if (EXPECTED_WPT == 4 ){
    if (tmp == 0) {
      //MSG(("ERROR - ETM status registor start/stop bit is not set.\n"));
      Errors++;
    }
  } else {
    if (tmp != 0) {
      // MSG(("ERROR - ETM status registor start/stop bit is not clear 0x%x.\n", tmp));
      Errors++;
    }
  }
  
  // Trace stop address has been set to the address of Function Stop(),
  // therefore, the ETM trace stops here.
  Stop();
  
  // Disable GPIO interrupt on change feature and deassert the GPIO output
  //GPIO1->IE.BYTE[INTDBG_BYTE] = 0;
  //GPIO1->DATA[GPIO_LOOPOUT].BYTE[INTDBG_BYTE] = 0;
  
  // Disable Interrupt
  NVIC_DisableIRQ(GPIO_IRQn);
  
  // Set programming bit to stop trace
  ETM->CR |= ETM_CR_PROGBIT;
  // Allow trace to drain. Have at most 32 bytes from ETM
  tmp = 3000;
  while(tmp !=0)
  {
    tmp --;
    if ((ETM->SR & ETM_SR_PROGBIT) !=0)
    {
      tmp = 0;
    }
  }
  if ((ETM->SR & ETM_SR_PROGBIT) ==0) {
//    MSG(("ERROR - ETM has not acknowledged Progbit.\n"));
//    MSG(("        Status Register %8x\n",ETM->SR));
    Errors++;
  }
  else
  {
    // allow time for 6 bytes in tpiu to drain
    testdata = 50;
    while(testdata !=0)
    {
      testdata--;
    }
    // Ensure tpiu is drained by forcing toggle of bit[24]
    DWT->CYCCNT = 1<<24;
    DWT->CYCCNT = 0;
    //MSG(("ETM stopped and drained\n"));
    
#if 0
    // Call debug driver here to validate captured trace
    if(CallDebugDriver(FnTrace) != TEST_PASS)
    {
      Errors++;
      MSG(("Checking trace failed\n"));
    }
    
    // just print following results, no need to check
    MSG(("Frames %d\n",DEBUGDRIVERDATA[0] & 0xFF));
    MSG(("(ETM)  %d\n",DEBUGDRIVERDATA[0] >> 8  & 0xFFF));
    MSG(("(ITM)  %d\n",DEBUGDRIVERDATA[0] >> 20 & 0x3FF));
    
    // print and check frame trigger
    tmp = DEBUGDRIVERDATA[0] >> 30 & 0x3;
    MSG(("(Trig) %d", tmp));
    if (tmp != 1) {
      Errors++;
      MSG((", expected 1\n"));
    } else {
      MSG(("\n"));
    }
    
    // just print following results, no need to check
    MSG(("ETM Stream:\n"));
    MSG(("      Sync: %d\n",DEBUGDRIVERDATA[1] >> 14 & 0x3F));
    MSG(("  Pheaders: %d\n",DEBUGDRIVERDATA[1] & 0x3FFF));
    MSG(("  branches: %d\n",DEBUGDRIVERDATA[1] >>20 & 0xFF));
    
    // print and check exception taken
    tmp = DEBUGDRIVERDATA[1] >>28;
    MSG(("exceptions: %d",tmp));
    if (tmp != 1) {
      Errors++;
      MSG((", expected 1\n"));
    } else {
      MSG(("\n"));
    }
    
    // print and check exception return
    tmp = DEBUGDRIVERDATA[2] & 0xF;
    MSG((" ex-return: %d",tmp));
    if (tmp != 1) {
      Errors++;
      MSG((", expected 1\n"));
    } else {
      MSG(("\n"));
    }
    
    // print and check ETM trigger
    tmp = DEBUGDRIVERDATA[2] >>14 & 0x3;
    MSG(("   trigger: %d", tmp));
    if (tmp != 1) {
      Errors++;
      MSG((", expected 1\n"));
    } else {
      MSG(("\n"));
    }
    
    // just print the result, no need to check
    MSG(("    ignore: %d\n",DEBUGDRIVERDATA[2] >> 12 & 0x3));
    
    // print and check ETM timestamp
    tmp = DEBUGDRIVERDATA[2] >>4 & 0xFF;
    MSG(("Timestamps: %d", tmp));
    if (tmp != 2) {
      Errors++;
      MSG((", expected 2\n"));
    } else {
      MSG(("\n"));
    }
#endif
    
  }
  
  return Errors;
}


//
// Handlers
//

void HardFault_Handler()
{
  //
  // The HardFault Handler is used for testing LOCKUP in this test
  //
  if(ExpectHardFault == 1)
  {
    // Tell main() that we took the HardFault
    HardFaultTaken = 1;
    
    // Access AHB Default Slave to take core into LOCKUP
    //((FuncPtr) (1 + (uint32_t)(&(DEBUGDRIVERDATA[3])))) ();
  }
  else
  {
    // Not expecting a fault
    //MSG(("Unexpected HardFault - FAIL\n"));
  }
}

void Start()
{
  testdata++;
  return;
}

void Stop()
{
  testdata++;
  return;
}

void GPIO_IRQHandler()
{
  
  // Disable interrupt
  NVIC_DisableIRQ(GPIO_IRQn);
  
}


void Default_IRQHandler()
{
  // Clear internal interupt sources by clearing GPIO2 output
  GPIO2->DATA[0].WORD = 0;
  
}


