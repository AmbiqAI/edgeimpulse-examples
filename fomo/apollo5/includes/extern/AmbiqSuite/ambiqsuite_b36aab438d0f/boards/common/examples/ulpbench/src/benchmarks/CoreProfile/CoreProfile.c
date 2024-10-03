/*********************************************************
(C) 2014 EEMBC(R) and ULPBench(TM).  All rights reserved.                            

EEMBC ULPBench Software is a product of EEMBC and is provided 
under the terms of the ULPBench License that is distributed 
with the official EEMBC ULPBench Software release.  
The Software is the proprietary intellectual property of EEMBC 
and its members and is protected under all applicable laws, 
including all applicable copyright laws.  
If you received this EEMBC ULPBench Software 
without the accompanying ULPBench License, 
you must discontinue use and download the official release 
from http://www.eembc.org/benchmark/ulp_sl.php.  
**********************************************************/

/*
File: CoreProfile.c
ULPBench CoreProfile, version 1.1.X
*/

/*******************************************************************************
*
* Purpose : EEMBC ULP Benchmark Workload Function for Step 1
* Author :   Stefan Schauer @ Texas Instruments, Shay Gal-On @ EEMBC
*
*******************************************************************************/

#include "Platform.h"
#include "CoreProfile.h"

#if DEBUG|GEN_REF
#include <stdio.h>
#endif

#if CALLGRIND_RUN
#include <valgrind/callgrind.h>
#endif


/* ============================================================================ */
/* Permutation defines and declarations */
/* ============================================================================ */
#define PERMUTE_BUF_SIZE 8
#define PERMUTE_BUF_MASK 7
RESTORE_ON_WAKEUP volatile ulp_u1 rxBuffer_ini[PERMUTE_BUF_SIZE] = {83,104,97,121,32,73,83,33};
RESTORE_ON_WAKEUP volatile ulp_u1 ipOut_ini[PERMUTE_BUF_SIZE] = {32,71,97,108,45,79,110,32};
RESTORE_ON_WAKEUP volatile ulp_u1 rxBuffer[PERMUTE_BUF_SIZE];
RESTORE_ON_WAKEUP volatile ulp_u1 ipOut[PERMUTE_BUF_SIZE];

/* ============================================================================ */
/* Variable defines */
/* declared as volatile to avoid optimizations */
/* ============================================================================ */
#define DISPLAYSIZE 16
#define DISPLAYMASK 15
RESTORE_ON_WAKEUP volatile ulp_u1 display_out[DISPLAYSIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile unsigned int errors=0;
RESTORE_ON_WAKEUP volatile ulp_u4 last_state=0;

RESTORE_ON_WAKEUP volatile ulp_u2 ulp_data[] = {
  0x1234, 0x2345, 0x3456 , 0x4567,
  0x9234, 0xA345, 0xB456 , 0xC567,
  0x165C, 0x2765, 0x3765 , 0x4765,
  0x9765, 0xA765, 0xB765 , 0xC765,
  0x1234, 0x2345, 0x3456 , 0x4567,
  0x9234, 0xA345, 0xB456 , 0xC567
};

RESTORE_ON_WAKEUP volatile ulp_u1 *string_a[4] = {
	"THE BROWN FOX JUMPS OVER THE LAZY DOG! The Brown Fox jumps Over The Lazzy Dog!",
	"THE WETTE CAT JUMPS OVER THE LAZY DOG! The Brown Fox jumps Over The Lazzy Dog!",
	"THE WHITE FOX JUMPS OVER THE LAZY DOG! The Brown Fox jumps Over The Lazzy Dog!",
	"THE STAMP CAT JUMPS OVER THE LAZY DOG! The Brown Fox jumps Over The Lazzy Dog!",
};
volatile ulp_u1 *string=NULL;

/* Structure: vfacts
	This structure is for holding values setting up execution of the code and verification.
	
*/
typedef struct vfacts_s {
/* Field: datasize
	Size of data for <math_int8> and <math_int16> functions, and iteration count for math functions and state machine
*/
	ulp_u2 datasize;
/* Field: displaysize
	Size of LCD display for <bin2LCD>
*/
	ulp_u2 displaysize;
/* Field: needle_pos
	Expected position for needle search in <tesStringSearch>
*/
	ulp_u2 needle_pos[4];
/* Field: sort_res
	Expected output from the <bubblesort> function
*/
	ulp_u1 sort_res[4];
/* Field: needle
	Bytes to search for in <tesStringSearch>
*/
	ulp_u1 *needle;
/* Field: sort_max
	Size of array to sort in <bubblesort>
*/
	unsigned int sort_max;
/* Field: sort_idx
	Index of value to return from <bubblesort>
*/
	ulp_u1 sort_idx;
/* Field: permutes
	Number of permutes to do in <permute>
*/
	ulp_u1 permutes;
/* Field: last_state
	Verification value required at end of execution if all values start at default
*/
	ulp_u4 last_state;
/* Field: iterations
	Number of workload iterations to execute on each call to the <workload active function>
*/
	ulp_u4 iterations;
/* Field: slots
	Slots to wake up and execute the workload in.
*/
	int slots[8];
} vfacts;

/* Array: defaults
	This array contains default setup values.
*/
static vfacts defaults[] = {
	/* data	display	 strstr pos			sort res 		 	needle 	sort max	sort_idx	permutes last state iterations slots*/
#if TES_NUM_TIME_SLOTS==8
/*0*/	{24 	,8,		{34,34,34,34},	{'N','H','H','M'},	"DOG",	10,			5,			16		,370		,1 	,{1,0,1,0,1,0,1,0}},
/*1*/	{24 	,8,		{34,34,34,34},	{'N','H','H','M'},	"DOG",	10,			5,			16		,1173		,4 	,{1,0,0,0,0,0,0,0}},
/*2*/	{100 	,16,	{39,39,39,39},	{'F','E','F','E'},	"The",	40,			15,			66		,12126		,44 ,{1,1,1,0,1,1,1,0}},
/*3*/	{10 	,8,		{0,0,0,0},		{'B','E','E','E'},	"THE",	5,			1,			6		,324		,1 	,{1,0,0,0,0,0,0,0}}
#endif
#if TES_NUM_TIME_SLOTS==1
/*0*/	{24 	,8,		{34,34,34,34},	{'N','H','H','M'},	"DOG",	10,			5,			16		,364		,1 	,{1,0,1,0,1,0,1,0}},
/*1*/	{24 	,8,		{34,34,34,34},	{'N','H','H','M'},	"DOG",	10,			5,			16		,630		,2 	,{1,0,0,0,0,0,0,0}},
/*2*/	{100 	,16,	{39,39,39,39},	{'F','E','F','E'},	"The",	40,			15,			66		,12475		,44 ,{1,1,1,0,1,1,1,0}},
/*3*/	{10 	,8,		{0,0,0,0},		{'B','E','E','E'},	"THE",	5,			1,			6		,327		,1 	,{1,0,0,0,0,0,0,0}},
#endif
};
#ifndef V1_WORKLOAD_ID
#define V1_WORKLOAD_ID 1
#endif
volatile unsigned int work_id=V1_WORKLOAD_ID;


RESTORE_ON_WAKEUP static volatile unsigned int loop_count=0;
RESTORE_ON_WAKEUP volatile unsigned int loop_param=0;
/* ============================================================================ */
/* Section: Functions */
/* Function: workload_func_1
	This is the <workload active function> that executes on timeslots set up by the <workload_main> function.
	

	Parameters:
	
		repeat - Number of time to iterate.
		facts - Inputs and setup values.

	Description:
	
- Blink an indication port 20 times quickly using <clear_Indication_Port> and <set_Indication_Port>, 
ending with active indicator.
- Run 24 times the <math_int8> <math_int16> and <state_machine>, 
using the cumulative result of the math functions as the input to the state function.
- Run the <bin2LCD> function using the output of the state function as input, with display size set to 8
- Run the <tesStringSearch> function on a string chosen based on the the previous step. Test the output value.
- Run the <tesBytesCopy> for <tesStringLength> over the size of the chosen string.
- Run the <bubblesort> function on the result of the previous step, and test the result.
- Run the <permute> function for 16 permutes
- Indicate end of workload iteration by calling <clear_Indication_Port>
		
*/
void workload_func_1(ulp_u1 repeat, vfacts *facts) {

   unsigned int i,sid;
   unsigned int innerloop;
   ulp_u4 math_cumu=last_state&0xffff;
   unsigned int state=0;
   volatile ulp_u1 * p_str1;
#if STATIC_LONG_ARRAY
   static ulp_u1 s[128];
#else
   ulp_u1 s[128];
#endif
   unsigned int DATASIZE=sizeof(ulp_data)/sizeof(ulp_u2);
   
   for (innerloop=0; innerloop < repeat; innerloop++)
   {
     for(i=0;i<20;i++){  
         PLT_STATUS_PIN_CLEAR;  /* Set on port pin to low. Used to make some peripheral access but can */
                                /* also be used to measure the correction repeat frequency and duration of the */
                                /* Workload function */
         PLT_STATUS_PIN_SET;    /* Set on port pin to high. */
     }
     /* we end with port high so during the processing of the following part the pin is high. */

     
     for(i=0;i<facts->datasize;i++){  
        /* Math simulating simple PID */
       if (math_cumu >= DATASIZE){
		  ulp_u1 cur_param=(ulp_u1)loop_param;
          math_cumu += math_int8(cur_param, (ulp_u1)ulp_data[cur_param & 0x07], (ulp_u1)ulp_data[0], (ulp_u1)ulp_data[i]);
       }else{
		  ulp_u1 cur_param=(ulp_u1)loop_param;
          math_cumu += math_int8(cur_param, (ulp_u1)ulp_data[cur_param & 0x07], (ulp_u1)ulp_data[math_cumu], (ulp_u1)ulp_data[i]);
       }
#if DEBUG>2
		printf("math8,%x\n",math_cumu);
#endif     
       if (math_cumu >= DATASIZE){
		  ulp_u2 cur_param=(ulp_u2)loop_param;
          math_cumu += math_int16(cur_param, ulp_data[cur_param & 0x07], ulp_data[0], ulp_data[i]);
       }else{
		  ulp_u2 cur_param=(ulp_u2)loop_param;
          math_cumu += math_int16(cur_param, ulp_data[cur_param & 0x07], ulp_data[math_cumu], ulp_data[i]);
       }
#if DEBUG>2
		printf("math16,%x\n",math_cumu);
#endif     
		/* State change based on PID output */
        state = state_machine(math_cumu);
#if DEBUG>2
		printf("state,%x\n",state);
#endif     
       
     } // for
     
     /* prepare the values for the display */
     /* here we have some table processing */
     bin2LCD((ulp_u1 *)display_out, facts->displaysize, ((ulp_u4)state<<16)|loop_param);
	 {
		ulp_u4 cur_state=last_state;
		last_state=cur_state+display_out[(innerloop+state)&DISPLAYMASK];
	 }
#if DEBUG>2
		printf("lcd,%x\n",last_state);
#endif     
	 /* setup flow to continue to next step input for verification and to make sure compiler does not optimize call away */
	 sid=last_state&0x3;
	 string=string_a[sid];     
     
     /* some String operation */
     /* find a sub string in a string - usecase for communications */
     p_str1 = (volatile ulp_u1 *) tesStringSearch((const ulp_u1 *)string, facts->needle);
#if DEBUG>2
		printf("sidx,%x\n",string + facts->needle_pos[sid] - p_str1,p_str1-string);
#endif     
     if (p_str1 != (string + facts->needle_pos[sid])){
         PLT_STATUS_PIN_CLEAR;      /* Set on port pin to low. */
    	 PLT_STATUS_PIN_SET;        /* Set on port pin to high again. */
	   errors++;
#if DEBUG
		printf("%d:string + facts->needle_pos[sid] - p_str1 = %d/%d\n",sid,string + facts->needle_pos[sid] - p_str1,p_str1-string);
#endif
#if GEN_REF
		facts->needle_pos[sid]=p_str1-string;
#endif
     }
	 last_state+= (p_str1==NULL) ? 256 : p_str1-string;
     tesBytesCopy(s,(void *)string,tesStringLength((ulp_u1 *)string));
     i = bubblesort(s,(int)facts->sort_max,(unsigned int)facts->sort_idx);
#if DEBUG>2
		printf("sres,%x\n",i);
#endif     
     if (i != facts->sort_res[sid]){
         PLT_STATUS_PIN_CLEAR;      /* Set on port pin to low. */
    	 PLT_STATUS_PIN_SET;        /* Set on port pin to high again. */
	   errors++;
#if DEBUG
		printf("%d:i=%c facts->sort_res[sid]=%c\n s:%s\n o:%s\n",sid,i,facts->sort_res[sid],s,string);
#endif
#if GEN_REF
		facts->sort_res[sid]=i;
#endif
     }
	 last_state+=i;

     permute((ulp_u1 *)ipOut, (ulp_u1 *)rxBuffer, ip, facts->permutes, PERMUTE_BUF_MASK);
	 {
		ulp_u4 cur_state=last_state;
		last_state=cur_state+ipOut[0];				/* Take an output of the permute into the compute chain */
	 }
#if DEBUG>2
		printf("slast,%x\n",last_state);
#endif     
     if (errors==0)
         PLT_STATUS_PIN_CLEAR;   /* Set on port pin to low. Used to make some peripheral access but can */
                                 /* also be used to measure the correction repeat frequency and duration of the */
                                 /* Workload function */
     
     loop_count++;
     loop_param++;
   } // for innerloop
   last_state+=state;
#if DEBUG>2
		printf("send,%x\n",last_state);
#endif     
}

//=============================================================================
// Event (Timer) structs
TesEvent tesTimer;

#if TES_NUM_TIME_SLOTS==8
TesEvent timer_1;
TesEvent timer_2;
TesEvent timer_3;
TesEvent timer_4;
TesEvent timer_5;
TesEvent timer_6;
TesEvent timer_7;
TesEvent *timers[TES_NUM_TIME_SLOTS] = {&tesTimer,&timer_1,&timer_2,&timer_3,&timer_4,&timer_5,&timer_6,&timer_7};
#elif TES_NUM_TIME_SLOTS==1
TesEvent *timers[TES_NUM_TIME_SLOTS] = {&tesTimer};
#else
TesEvent *timers[TES_NUM_TIME_SLOTS] = {&tesTimer};
#endif

/*=============================================================================
Function: setTimers
 Set timers for wakeups
*/
void setTimers(vfacts *facts)
{
	int i;
	tesSlotTimerSet(timers[0], 0 /*slot*/, 0 /*param*/);  /* Always register for slot 0 */
	for (i=1; i<TES_NUM_TIME_SLOTS; i++) {
		if (facts->slots[i])
			tesSlotTimerSet(timers[i], (TesTimeSlot)i /*slot*/, (unsigned char)i /*param*/);
	}
}


/*=============================================================================
 Function: CoreProfileMain
	Core Profile Workload Function
*/

void CoreProfileMain(void)
{
#if (CALLGRIND_RUN || DEBUG)
	unsigned int outer_iterations=0;
#endif
	RESTORE_ON_WAKEUP vfacts *facts;
	tesBytesCopy((void *)rxBuffer,(void *)rxBuffer_ini,PERMUTE_BUF_SIZE);
	tesBytesCopy((void *)ipOut,(void *)ipOut_ini,PERMUTE_BUF_SIZE);
#if CALLGRIND_RUN
	CALLGRIND_START_INSTRUMENTATION;
#endif
#if DO_MICA
 #if defined(_MSC_VER)
	__asm int 3;
 #else
	__asm volatile("int3");
 #endif
#endif
	facts=&(defaults[work_id]);
    setTimers(facts);

    // event loop
    for (;;) {
        TesEvent * event = tesEventWait();
		facts=&(defaults[work_id]);
        if ((event == &tesTimer) || (facts->slots[0]==-1)) {
			/* Reset values so next duty cycle is going to be the same. */
			/* Note : slot 0 must be registered for this to work! */
			last_state=0;
			loop_param=0;
			tesBytesCopy((void *)rxBuffer,(void *)rxBuffer_ini,PERMUTE_BUF_SIZE);
			tesBytesCopy((void *)ipOut,(void *)ipOut_ini,PERMUTE_BUF_SIZE);
			math_int_init();
			reset_state();
		}
        workload_func_1((ulp_u1)facts->iterations,facts);
        if (event == &tesTimer) {
#if GEN_REF && !defined(CALLGRIND_RUN)
		printf("\n/*%d*/\t{%d 	,%d,		{%d,%d,%d,%d},	{'%c','%c','%c','%c'},	\"%s\",	%d,			%d,			%d		,%d		,%d 	,{%d,%d,%d,%d,%d,%d,%d,%d}},\n",
				work_id,facts->datasize,facts->displaysize,
				facts->needle_pos[0],facts->needle_pos[1],facts->needle_pos[2],facts->needle_pos[3],
				facts->sort_res[0],facts->sort_res[1],facts->sort_res[2],facts->sort_res[3],
				facts->needle,facts->sort_max,facts->sort_idx,facts->permutes,last_state,facts->iterations,
				facts->slots[0],facts->slots[1],facts->slots[2],facts->slots[3],
				facts->slots[4],facts->slots[5],facts->slots[6],facts->slots[7]);
			work_id++;
			if (work_id>=(sizeof(defaults)/sizeof(vfacts)))
				exit(0);
            setTimers(facts);      
#else
		/* A0 slot, after workload, verify final result of previous iterations */
		if (last_state!=facts->last_state) {
		    PLT_STATUS_PIN_CLEAR;   /* Set on port pin to low. */
			PLT_STATUS_PIN_SET;     /* Set on port pin to high again. */
			errors++;
 #if DEBUG
		printf("%d:last_state=%x facts->last_state=%x\n",work_id,last_state,facts->last_state);
            setTimers(facts); 
 #endif
		}
 #if DEBUG
			if (++outer_iterations>16)
				exit(0);
 #endif
			if (work_id>=(sizeof(defaults)/sizeof(vfacts)))
				work_id=0;
#endif
		}
		/* A0 slot : Make sure once per duty cycle that we are registered for all required slots. */
        if (event == &tesTimer) {
            setTimers(facts);
#if DO_MICA
 #if defined(_MSC_VER)
	__asm int 3;
 #else
	__asm volatile("int3");
 #endif
	exit(1);
#endif
        }
#if CALLGRIND_RUN
		if (++outer_iterations>10) {
	 CALLGRIND_STOP_INSTRUMENTATION;
	exit(1);
		}
#endif
    }
}
  
