
#ifndef AUDADC_DMACPL_TEST_CASES_H_
#define AUDADC_DMACPL_TEST_CASES_H_

#define MAX_TEST_NUM           1024
#define XTALHS_EN              1  // 1: use 32 MHz XTAL for AUDADC, 0: use HFRC
#define DECIMATION_RATE        1 // 1 for no decimation; 3 for 16 kHz

#define HFADJ_EN               0  // enable HFADJ for better frequency accuracy when using HFRC (uses 32 kHz crystal to adjust HFRC)

#define AUDADC_ANALYZE_SAMPLES 0  // enable to analyze samples
#define AUDADC_DUMP_SAMPLES    0  // enable to dump samples for debug purposes
#define AUDADC_DUMP_FACTOR     2  // dump FFT_LENGTH times this factor

#define AUDADC_NUM_BITS       12  // number of bits in AUDADC output
#define AUDADC_DATA_OFFSET  2048 // middle code for AUDADC output

#define AUDADC_SAMPLE_BUF_SIZE  128

#if XTALHS_EN
// nominal XTALHS is 32 MHz
#define CLKFREQ            24576000
#define CLKDIV             2 // NOTE: if you change this you must update AUDADC_SAMPLE_RATE accordingly
#define TIMERMAX           333  // ~48000 Hz
#else
// when using HFRC2_48
#define CLKFREQ            49152000
#define CLKDIV             1 // NOTE: if you change this you must update AUDADC_SAMPLE_RATE accordingly
#define TIMERMAX           511  // ~48000 Hz
#endif

#define AUDADC_SAMPLE_RATE CLKFREQ/(2*(TIMERMAX+1)) // sample rate of AUDADC in Hz - assumes CLKDIV 1

#define CH_A0_EN               1  // enable LPADC01 inputs with PGA channel A0 on AUDADC SLOT0
#define CH_A1_EN               1  // enable LPADC01 inputs with PGA channel A1 on AUDADC SLOT1
#define CH_B0_EN               0
#define CH_B1_EN               0


// when ATE_VERSION==0, the following gains apply for the channels enabled above
#define CH_A0_GAIN_DB       0.0  // PGA channel A0 gain in dB (valid range: -6.0 to 36.0)
#define CH_A1_GAIN_DB       0.0  // PGA channel A1 gain in dB (valid range: -6.0 to 36.0)

// when ATE_VERSION==0, these hard-coded values are used instead of GPIO pin or TB_DEBUG_REG
#define AUTOCAL_EN 0 // 0: disable autocal, 1 (or greater): enable autocal
#define LPMODE_SET 1 // 0: LPMODE0, 1 (or greater): LPMODE1
#if XTALHS_EN
#define TRKCYC_SET 11 // trkcyc setting (config so that (TRKCYC+5)/CLKFREQ >= 500ns)
#else // assume HFRC2_48
#define TRKCYC_SET 20 // trkcyc setting (config so that (TRKCYC+5)/CLKFREQ >= 500ns)
#endif
#define AUDADC_PWRDLY_PWR1_SET 4 // 0: min, 2: 5us? (default), 4: 10us?


#define EXCESS_TRACKING_FACTOR 1.0  // give more tracking time by scaling this up

#define NUM_CHANNELS        (CH_A0_EN + CH_A1_EN + CH_B0_EN + CH_B1_EN)

#endif // #ifndef AUDADC_DMACPL_TEST_CASES_H_
