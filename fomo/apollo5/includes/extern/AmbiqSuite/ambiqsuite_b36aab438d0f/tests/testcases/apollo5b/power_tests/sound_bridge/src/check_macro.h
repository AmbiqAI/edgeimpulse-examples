#ifndef CHECK_MACRO_H
#define CHECK_MACRO_H

#define VALUE(x) STR(x)
#define STR(x) #x

#if ENUM_I2S_IN_MODULE == I2S0_MODULE
    #define i2s_in_isr              am_dspi2s0_isr
    #define i2s_out_isr             am_dspi2s1_isr
    #define I2S_OUT_MODULE          (I2S1_MODULE)
#else
    #define i2s_in_isr              am_dspi2s1_isr
    #define i2s_out_isr             am_dspi2s0_isr
    #define I2S_OUT_MODULE          (I2S0_MODULE)
#endif

#if (ENUM_TEST_MODE != I2S_IN_ONLY)    && \
    (ENUM_TEST_MODE != PDM_IN_ONLY)    && \
    (ENUM_TEST_MODE != AUDADC_IN_ONLY) && \
    (ENUM_TEST_MODE != I2S_OUT_ONLY)   && \
    (ENUM_TEST_MODE != I2S_TO_I2S)     && \
    (ENUM_TEST_MODE != PDM_TO_I2S)     && \
    (ENUM_TEST_MODE != AUDADC_TO_I2S)  && \
    (ENUM_TEST_MODE != DEBUG)
#error "Invaild ENUM_TEST_MODE."
#endif

#if (ENUM_CLOCK_SOURCE != CLKSRC_HFRC)   && \
    (ENUM_CLOCK_SOURCE != CLKSRC_HF2ADJ) && \
    (ENUM_CLOCK_SOURCE != CLKSRC_PLL)
#error "Invaild ENUM_CLOCK_SOURCE."
#endif

#if (ENUM_I2S_IN_MODULE != I2S0_MODULE) && \
    (ENUM_I2S_IN_MODULE != I2S1_MODULE)
#error "Invaild ENUM_I2S_IN_MODULE."
#endif

#if (ENUM_I2S_IN_ROLE != AM_HAL_I2S_IO_MODE_MASTER) && \
    (ENUM_I2S_IN_ROLE != AM_HAL_I2S_IO_MODE_SLAVE)
#error "Invaild ENUM_I2S_IN_ROLE."
#endif

#if (ENUM_I2S_OUT_ROLE != AM_HAL_I2S_IO_MODE_MASTER) && \
    (ENUM_I2S_OUT_ROLE != AM_HAL_I2S_IO_MODE_SLAVE)
#error "Invaild ENUM_I2S_OUT_ROLE."
#endif

#if (UINT_SYS_SAMPLE_RATES !=  8000) && \
    (UINT_SYS_SAMPLE_RATES != 16000) && \
    (UINT_SYS_SAMPLE_RATES != 24000) && \
    (UINT_SYS_SAMPLE_RATES != 32000)
#error "Invaild UINT_SYS_SAMPLE_RATES."
#endif

#if (UINT_I2S_IN_CHANNELS > 8) || \
    (UINT_I2S_IN_CHANNELS < 1)
#error "Invaild UINT_I2S_IN_CHANNELS."
#endif

#if (UINT_I2S_IN_BITS !=  8) && \
    (UINT_I2S_IN_BITS != 16) && \
    (UINT_I2S_IN_BITS != 24) && \
    (UINT_I2S_IN_BITS != 32)
#error "Invaild UINT_I2S_IN_BITS."
#endif

#if (UINT_I2S_OUT_CHANNELS > 8) || \
    (UINT_I2S_OUT_CHANNELS < 1)
#error "Invaild UINT_I2S_OUT_CHANNELS."
#endif

#if (UINT_I2S_OUT_BITS !=  8) && \
    (UINT_I2S_OUT_BITS != 16) && \
    (UINT_I2S_OUT_BITS != 24) && \
    (UINT_I2S_OUT_BITS != 32)
#error "Invaild UINT_I2S_OUT_BITS."
#endif

#if (UINT_PDM_CHANNELS > 2) || \
    (UINT_PDM_CHANNELS < 1)
#error "Invaild UINT_PDM_CHANNELS."
#endif

#if (UINT_PDM_SINCRATE != 16) && \
    (UINT_PDM_SINCRATE != 24) && \
    (UINT_PDM_SINCRATE != 32) && \
    (UINT_PDM_SINCRATE != 48) && \
    (UINT_PDM_SINCRATE != 64)
#error "Invaild UINT_PDM_SINCRATE."
#endif

// #if (ENUM_TEST_MODE == PDM_TO_I2S) && (UINT_I2S_OUT_CHANNELS != 2)
// #error "UINT_I2S_OUT_CHANNELS must be 2 in PDM_TO_I2S mode."
// #endif

// #if (ENUM_TEST_MODE == AUDADC_TO_I2S) && (UINT_AUDADC_RPTT_CLK_DIV != 2)
// #warning ""
// #endif

#if ENUM_TEST_MODE & I2S_IN
    #if UINT_I2S_IN_BITS == 8
        #define I2S_IN_WORD_WIDTH (AM_HAL_I2S_FRAME_WDLEN_8BITS)
        #define I2S_IN_BIT_DEPTH  (AM_HAL_I2S_SAMPLE_LENGTH_8BITS)
    #elif UINT_I2S_IN_BITS == 16
        #define I2S_IN_WORD_WIDTH (AM_HAL_I2S_FRAME_WDLEN_16BITS)
        #define I2S_IN_BIT_DEPTH  (AM_HAL_I2S_SAMPLE_LENGTH_16BITS)
    #elif UINT_I2S_IN_BITS == 24
        #define I2S_IN_WORD_WIDTH (AM_HAL_I2S_FRAME_WDLEN_24BITS)
        #define I2S_IN_BIT_DEPTH  (AM_HAL_I2S_SAMPLE_LENGTH_24BITS)
    #elif UINT_I2S_IN_BITS == 32
        #define I2S_IN_WORD_WIDTH (AM_HAL_I2S_FRAME_WDLEN_32BITS)
        #define I2S_IN_BIT_DEPTH  (AM_HAL_I2S_SAMPLE_LENGTH_32BITS)
    #endif
#endif

#if ENUM_TEST_MODE & I2S_OUT
    #if UINT_I2S_OUT_BITS == 8
        #define I2S_OUT_WORD_WIDTH (AM_HAL_I2S_FRAME_WDLEN_8BITS)
        #define I2S_OUT_BIT_DEPTH  (AM_HAL_I2S_SAMPLE_LENGTH_8BITS)
    #elif UINT_I2S_OUT_BITS == 16
        #define I2S_OUT_WORD_WIDTH (AM_HAL_I2S_FRAME_WDLEN_16BITS)
        #define I2S_OUT_BIT_DEPTH  (AM_HAL_I2S_SAMPLE_LENGTH_16BITS)
    #elif UINT_I2S_OUT_BITS == 24
        #define I2S_OUT_WORD_WIDTH (AM_HAL_I2S_FRAME_WDLEN_24BITS)
        #define I2S_OUT_BIT_DEPTH  (AM_HAL_I2S_SAMPLE_LENGTH_24BITS)
    #elif UINT_I2S_OUT_BITS == 32
        #define I2S_OUT_WORD_WIDTH (AM_HAL_I2S_FRAME_WDLEN_32BITS)
        #define I2S_OUT_BIT_DEPTH  (AM_HAL_I2S_SAMPLE_LENGTH_32BITS)
    #endif
#endif

#define AUDADC_IRTT_MIN_PER (650)

#if UINT_I2S_IN_CHANNELS == 1
#define I2S_IN_CLK_FREQ  (UINT_SYS_SAMPLE_RATES * (2) * UINT_I2S_IN_BITS)
#else
#define I2S_IN_CLK_FREQ  (UINT_SYS_SAMPLE_RATES * UINT_I2S_IN_CHANNELS * UINT_I2S_IN_BITS)
#endif

#if UINT_I2S_OUT_CHANNELS == 1
#define I2S_OUT_CLK_FREQ (UINT_SYS_SAMPLE_RATES * (2) * UINT_I2S_OUT_BITS)
#else
#define I2S_OUT_CLK_FREQ (UINT_SYS_SAMPLE_RATES * UINT_I2S_OUT_CHANNELS * UINT_I2S_OUT_BITS)
#endif

#define PDM_CLKO_FREQ    (UINT_SYS_SAMPLE_RATES * (2) * UINT_PDM_SINCRATE)

#if ENUM_TEST_MODE & I2S_IN
#pragma message "I2S IN clock frequency: " VALUE(I2S_IN_CLK_FREQ)
#endif

#if ENUM_TEST_MODE & I2S_OUT
#pragma message "I2S OUT clock frequency: " VALUE(I2S_OUT_CLK_FREQ)
#endif

#if ENUM_TEST_MODE & PDM_IN
#pragma message "PDM clock frequency: " VALUE(PDM_CLKO_FREQ)
#endif

#if ENUM_TEST_MODE & AUDADC_IN
#pragma message "AUDADC minium required clock frequency: " VALUE(UINT_SYS_SAMPLE_RATES * AUDADC_IRTT_MIN_PER)
#endif


//*****************************************************************************
//
// Clock selections.
//
//*****************************************************************************
#if ENUM_CLOCK_SOURCE == CLKSRC_HFRC

    #define CLK_RATIO_A (125)
    #define CLK_RATIO_B (128)
    #define PDM_CLK     (AM_HAL_PDM_CLK_HFRC_24MHZ)
    #define AUDADC_CLK  (AM_HAL_AUDADC_CLKSEL_HFRC_48MHz)

    #define I2S_CLKSEL_HFRC_24MHz               (24000000)
    #define I2S_CLKSEL_HFRC_12MHz               (12000000)
    #define I2S_CLKSEL_HFRC_6MHz                 (6000000)
    #define I2S_CLKSEL_HFRC_3MHz                 (3000000)
    #define I2S_CLKSEL_HFRC_1_5MHz               (1500000)
    #define I2S_CLKSEL_HFRC_750kHz                (750000)
    #define I2S_CLKSEL_HFRC_375kHz                (375000)
    #define PDM_CLK_HFRC_24MHZ                  (24000000)
    #define AUDADC_CLKSEL_HFRC_48MHz            (48000000)

    #if ENUM_TEST_MODE & I2S_IN
    #if (CLK_RATIO_A*I2S_IN_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_12MHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_12MHz)
        #define I2S_IN_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_6MHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_6MHz)
        #define I2S_IN_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_3MHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_3MHz)
        #define I2S_IN_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_1_5MHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_1_5MHz)
        #define I2S_IN_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_750kHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_750kHz)
        #define I2S_IN_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_375kHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_375kHz)
        #define I2S_IN_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_24MHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_24MHz)
        #define I2S_IN_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_12MHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_12MHz)
        #define I2S_IN_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_6MHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_6MHz)
        #define I2S_IN_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_3MHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_3MHz)
        #define I2S_IN_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_1_5MHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_1_5MHz)
        #define I2S_IN_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_750kHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_750kHz)
        #define I2S_IN_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_IN_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_375kHz)
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC_375kHz)
        #define I2S_IN_CLK_DIV3 (1)
    #else
        #define I2S_IN_CLK      ERROR_VALUE
        #define I2S_IN_CLK_DIV3 ERROR_VALUE
        #error "Wrong I2S_IN_CLK_FREQ."
    #endif
    #endif

    #if ENUM_TEST_MODE & I2S_OUT
    #if (CLK_RATIO_A*I2S_OUT_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_12MHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_12MHz)
        #define I2S_OUT_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_6MHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_6MHz)
        #define I2S_OUT_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_3MHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_3MHz)
        #define I2S_OUT_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_1_5MHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_1_5MHz)
        #define I2S_OUT_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_750kHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_750kHz)
        #define I2S_OUT_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_375kHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_375kHz)
        #define I2S_OUT_CLK_DIV3 (0)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_24MHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_24MHz)
        #define I2S_OUT_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_12MHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_12MHz)
        #define I2S_OUT_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_6MHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_6MHz)
        #define I2S_OUT_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_3MHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_3MHz)
        #define I2S_OUT_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_1_5MHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_1_5MHz)
        #define I2S_OUT_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_750kHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_750kHz)
        #define I2S_OUT_CLK_DIV3 (1)
    #elif (CLK_RATIO_A*I2S_OUT_CLK_FREQ*3) == (CLK_RATIO_B*I2S_CLKSEL_HFRC_375kHz)
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC_375kHz)
        #define I2S_OUT_CLK_DIV3 (1)
    #else
        #define I2S_OUT_CLK      ERROR_VALUE
        #define I2S_OUT_CLK_DIV3 ERROR_VALUE
        #error "Wrong I2S_OUT_CLK_FREQ."
    #endif
    #endif

    #if ENUM_TEST_MODE & PDM_IN
    #if (PDM_CLK_HFRC_24MHZ*CLK_RATIO_B) % (PDM_CLKO_FREQ*CLK_RATIO_A) == 0
        #define PDM_CLOCK_DIV_VALUE  ((PDM_CLK_HFRC_24MHZ*CLK_RATIO_B) / (PDM_CLKO_FREQ*CLK_RATIO_A))
    #else
        #define PDM_CLOCK_DIV_VALUE ERROR_VALUE
        #error "Wrong PDM_CLKO_FREQ."
    #endif
    #endif

    #if ENUM_TEST_MODE & AUDADC_IN
    #if ((AUDADC_CLKSEL_HFRC_48MHz / UINT_SYS_SAMPLE_RATES) < 2047) && ((AUDADC_CLKSEL_HFRC_48MHz / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV   (AM_HAL_AUDADC_RPTT_CLK_DIV2)
        #if (AUDADC_CLKSEL_HFRC_48MHz % (2 * UINT_SYS_SAMPLE_RATES)) == 0
            #define AUDADC_COUNT_MAX  ((AUDADC_CLKSEL_HFRC_48MHz / (2 * UINT_SYS_SAMPLE_RATES)) - 1)
        #else
            #define AUDADC_COUNT_MAX  ((((2 * AUDADC_CLKSEL_HFRC_48MHz) / (2 * UINT_SYS_SAMPLE_RATES) + 1) / 2) - 1)
        #endif
    #elif ((AUDADC_CLKSEL_HFRC_48MHz / UINT_SYS_SAMPLE_RATES) < 4094) && ((AUDADC_CLKSEL_HFRC_48MHz / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV   (AM_HAL_AUDADC_RPTT_CLK_DIV4)
        #if (AUDADC_CLKSEL_HFRC_48MHz % (4 * UINT_SYS_SAMPLE_RATES)) == 0
            #define AUDADC_COUNT_MAX  (AUDADC_CLKSEL_HFRC_48MHz / (4 * UINT_SYS_SAMPLE_RATES))
        #else
            #define AUDADC_COUNT_MAX  (((2 * AUDADC_CLKSEL_HFRC_48MHz) / (4 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
        #endif
    #elif ((AUDADC_CLKSEL_HFRC_48MHz / UINT_SYS_SAMPLE_RATES) < 8188) && ((AUDADC_CLKSEL_HFRC_48MHz / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV   (AM_HAL_AUDADC_RPTT_CLK_DIV8)
        #if (AUDADC_CLKSEL_HFRC_48MHz % (8 * UINT_SYS_SAMPLE_RATES)) == 0
            #define AUDADC_COUNT_MAX  (AUDADC_CLKSEL_HFRC_48MHz / (8 * UINT_SYS_SAMPLE_RATES))
        #else
            #define AUDADC_COUNT_MAX  (((2 * AUDADC_CLKSEL_HFRC_48MHz) / (8 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
        #endif
    #elif ((AUDADC_CLKSEL_HFRC_48MHz / UINT_SYS_SAMPLE_RATES) < 16376) && ((AUDADC_CLKSEL_HFRC_48MHz / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV   (AM_HAL_AUDADC_RPTT_CLK_DIV16)
        #if (AUDADC_CLKSEL_HFRC_48MHz % (16 * UINT_SYS_SAMPLE_RATES)) == 0
            #define AUDADC_COUNT_MAX  (AUDADC_CLKSEL_HFRC_48MHz / (16 * UINT_SYS_SAMPLE_RATES))
        #else
            #define AUDADC_COUNT_MAX  (((2 * AUDADC_CLKSEL_HFRC_48MHz) / (16 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
        #endif
    #elif ((AUDADC_CLKSEL_HFRC_48MHz / UINT_SYS_SAMPLE_RATES) < 32752) && ((AUDADC_CLKSEL_HFRC_48MHz / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV   (AM_HAL_AUDADC_RPTT_CLK_DIV32)
        #if (AUDADC_CLKSEL_HFRC_48MHz % (32 * UINT_SYS_SAMPLE_RATES)) == 0
            #define AUDADC_COUNT_MAX  (AUDADC_CLKSEL_HFRC_48MHz / (32 * UINT_SYS_SAMPLE_RATES))
        #else
            #define AUDADC_COUNT_MAX  (((2 * AUDADC_CLKSEL_HFRC_48MHz) / (32 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
        #endif
    #else
        #define AUDADC_RPTT_CLK_DIV_VALUE ERROR_VALUE
        #define AUDADC_COUNT_MAX          ERROR_VALUE
        #error "Can't find an appropirate AUDADC_RPTT_CLK_DIV_VALUE."
    #endif
    #endif

#elif ENUM_CLOCK_SOURCE == CLKSRC_HF2ADJ
    #define PDM_CLK (AM_HAL_PDM_CLK_HFRC2_31MHZ)
    #define AUDADC_CLK (AM_HAL_AUDADC_CLKSEL_HFRC2_APPROX_31MHz)

    #define I2S_CLKSEL_HFRC2_APPROX_31MHz_ADJ    (24576000)
    #define I2S_CLKSEL_HFRC2_APPROX_16MHz_ADJ    (12288000)
    #define I2S_CLKSEL_HFRC2_APPROX_8MHz_ADJ      (6144000)
    #define I2S_CLKSEL_HFRC2_APPROX_4MHz_ADJ      (3072000)
    #define I2S_CLKSEL_HFRC2_APPROX_2MHz_ADJ      (1536000)
    #define I2S_CLKSEL_HFRC2_APPROX_977kHz_ADJ     (768000)
    #define I2S_CLKSEL_HFRC2_APPROX_488kHz_ADJ     (384000)
    #define PDM_CLK_HFRC2_31MHZ_ADJ              (24576000)
    #define AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ (24576000)

    #if ENUM_TEST_MODE & I2S_IN
    #if I2S_IN_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_16MHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_16MHz)
        #define I2S_IN_CLK_DIV3  (0)
    #elif I2S_IN_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_8MHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_8MHz)
        #define I2S_IN_CLK_DIV3  (0)
    #elif I2S_IN_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_4MHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_4MHz)
        #define I2S_IN_CLK_DIV3  (0)
    #elif I2S_IN_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_2MHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_2MHz)
        #define I2S_IN_CLK_DIV3  (0)
    #elif I2S_IN_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_977kHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_977kHz)
        #define I2S_IN_CLK_DIV3  (0)
    #elif I2S_IN_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_488kHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_488kHz)
        #define I2S_IN_CLK_DIV3  (0)
    #elif (I2S_IN_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_31MHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_31MHz)
        #define I2S_IN_CLK_DIV3  (1)
    #elif (I2S_IN_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_16MHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_16MHz)
        #define I2S_IN_CLK_DIV3  (1)
    #elif (I2S_IN_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_8MHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_8MHz)
        #define I2S_IN_CLK_DIV3  (1)
    #elif (I2S_IN_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_4MHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_4MHz)
        #define I2S_IN_CLK_DIV3  (1)
    #elif (I2S_IN_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_2MHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_2MHz)
        #define I2S_IN_CLK_DIV3  (1)
    #elif (I2S_IN_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_977kHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_977kHz)
        #define I2S_IN_CLK_DIV3  (1)
    #elif (I2S_IN_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_488kHz_ADJ
        #define I2S_IN_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_488kHz)
        #define I2S_IN_CLK_DIV3  (1)
    #else
        #define I2S_IN_CLK      ERROR_VALUE
        #define I2S_IN_CLK_DIV3 ERROR_VALUE
        #error "Wrong I2S_IN_CLK_FREQ."
    #endif
    #endif

    #if ENUM_TEST_MODE & I2S_OUT
    #if I2S_OUT_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_16MHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_16MHz)
        #define I2S_OUT_CLK_DIV3  (0)
    #elif I2S_OUT_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_8MHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_8MHz)
        #define I2S_OUT_CLK_DIV3  (0)
    #elif I2S_OUT_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_4MHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_4MHz)
        #define I2S_OUT_CLK_DIV3  (0)
    #elif I2S_OUT_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_2MHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_2MHz)
        #define I2S_OUT_CLK_DIV3  (0)
    #elif I2S_OUT_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_977kHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_977kHz)
        #define I2S_OUT_CLK_DIV3  (0)
    #elif I2S_OUT_CLK_FREQ == I2S_CLKSEL_HFRC2_APPROX_488kHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_488kHz)
        #define I2S_OUT_CLK_DIV3  (0)
    #elif (I2S_OUT_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_31MHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_31MHz)
        #define I2S_OUT_CLK_DIV3  (1)
    #elif (I2S_OUT_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_16MHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_16MHz)
        #define I2S_OUT_CLK_DIV3  (1)
    #elif (I2S_OUT_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_8MHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_8MHz)
        #define I2S_OUT_CLK_DIV3  (1)
    #elif (I2S_OUT_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_4MHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_4MHz)
        #define I2S_OUT_CLK_DIV3  (1)
    #elif (I2S_OUT_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_2MHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_2MHz)
        #define I2S_OUT_CLK_DIV3  (1)
    #elif (I2S_OUT_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_977kHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_977kHz)
        #define I2S_OUT_CLK_DIV3  (1)
    #elif (I2S_OUT_CLK_FREQ*3) == I2S_CLKSEL_HFRC2_APPROX_488kHz_ADJ
        #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_488kHz)
        #define I2S_OUT_CLK_DIV3  (1)
    #else
        #define I2S_OUT_CLK      ERROR_VALUE
        #define I2S_OUT_CLK_DIV3 ERROR_VALUE
        #error "Wrong I2S_OUT_CLK_FREQ."
    #endif
    #endif

    #if ENUM_TEST_MODE & PDM_IN
    #if (PDM_CLK_HFRC2_31MHZ_ADJ) % (PDM_CLKO_FREQ) == 0
        #define PDM_CLOCK_DIV_VALUE  ((PDM_CLK_HFRC2_31MHZ_ADJ) / (PDM_CLKO_FREQ))
    #else
        #define PDM_CLOCK_DIV_VALUE ERROR_VALUE
        #error "Wrong PDM_CLKO_FREQ."
    #endif
    #endif

    #if ENUM_TEST_MODE & AUDADC_IN
    #if ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / UINT_SYS_SAMPLE_RATES) < 2047) && ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV   (AM_HAL_AUDADC_RPTT_CLK_DIV2)
        #if (AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ % (2 * UINT_SYS_SAMPLE_RATES)) == 0
            #define AUDADC_COUNT_MAX  ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / (2 * UINT_SYS_SAMPLE_RATES)) - 1)
        #else
            #define AUDADC_COUNT_MAX  ((((2 * AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ) / (2 * UINT_SYS_SAMPLE_RATES) + 1) / 2) - 1)
        #endif
    #elif ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / UINT_SYS_SAMPLE_RATES) < 4094) && ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV   (AM_HAL_AUDADC_RPTT_CLK_DIV4)
        #if (AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ % (4 * UINT_SYS_SAMPLE_RATES)) == 0
            #define AUDADC_COUNT_MAX  (AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / (4 * UINT_SYS_SAMPLE_RATES))
        #else
            #define AUDADC_COUNT_MAX  (((2 * AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ) / (4 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
        #endif
    #elif ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / UINT_SYS_SAMPLE_RATES) < 8188) && ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV   (AM_HAL_AUDADC_RPTT_CLK_DIV8)
        #if (AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ % (8 * UINT_SYS_SAMPLE_RATES)) == 0
            #define AUDADC_COUNT_MAX  (AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / (8 * UINT_SYS_SAMPLE_RATES))
        #else
            #define AUDADC_COUNT_MAX  (((2 * AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ) / (8 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
        #endif
    #elif ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / UINT_SYS_SAMPLE_RATES) < 16376) && ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV   (AM_HAL_AUDADC_RPTT_CLK_DIV16)
        #if (AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ % (16 * UINT_SYS_SAMPLE_RATES)) == 0
            #define AUDADC_COUNT_MAX  (AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / (16 * UINT_SYS_SAMPLE_RATES))
        #else
            #define AUDADC_COUNT_MAX  (((2 * AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ) / (16 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
        #endif
    #elif ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / UINT_SYS_SAMPLE_RATES) < 32752) && ((AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV   (AM_HAL_AUDADC_RPTT_CLK_DIV32)
        #if (AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ % (32 * UINT_SYS_SAMPLE_RATES)) == 0
            #define AUDADC_COUNT_MAX  (AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ / (32 * UINT_SYS_SAMPLE_RATES))
        #else
            #define AUDADC_COUNT_MAX  (((2 * AUDADC_CLKSEL_HFRC2_APPROX_31MHz_ADJ) / (32 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
        #endif
    #else
        #define AUDADC_RPTT_CLK_DIV_VALUE ERROR_VALUE
        #define AUDADC_COUNT_MAX          ERROR_VALUE
        #error "Can't find an appropirate AUDADC_RPTT_CLK_DIV_VALUE."
    #endif
    #endif

#elif ENUM_CLOCK_SOURCE == CLKSRC_PLL
    #define PDM_CLK (AM_HAL_PDM_CLK_PLL)
    #define AUDADC_CLK (AM_HAL_AUDADC_CLKSEL_PLL)

    #if ENUM_TEST_MODE == I2S_IN_ONLY
        #define PLL_FREQ         (I2S_IN_CLK_FREQ * 6)
        #define I2S_IN_CLK       (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
        #define I2S_IN_CLK_DIV3  (0)
    #elif ENUM_TEST_MODE == I2S_OUT_ONLY
        #define PLL_FREQ         (I2S_OUT_CLK_FREQ * 6)
        #define I2S_OUT_CLK      (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
        #define I2S_OUT_CLK_DIV3 (0)
    #elif ENUM_TEST_MODE == I2S_TO_I2S
        #if I2S_IN_CLK_FREQ == I2S_OUT_CLK_FREQ
            #define PLL_FREQ         (I2S_IN_CLK_FREQ * 6)
            #define I2S_IN_CLK       (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK      (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_IN_CLK_DIV3  (0)
            #define I2S_OUT_CLK_DIV3 (0)
        #elif (I2S_IN_CLK_FREQ * 3) == I2S_OUT_CLK_FREQ
            #define PLL_FREQ         (I2S_IN_CLK_FREQ * 6 * 3)
            #define I2S_IN_CLK       (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK      (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_IN_CLK_DIV3  (1)
            #define I2S_OUT_CLK_DIV3 (0)
        #elif I2S_IN_CLK_FREQ == (I2S_OUT_CLK_FREQ * 3)
            #define PLL_FREQ         (I2S_IN_CLK_FREQ * 6)
            #define I2S_IN_CLK       (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK      (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_IN_CLK_DIV3  (0)
            #define I2S_OUT_CLK_DIV3 (1)
        #elif (I2S_IN_CLK_FREQ * 3) == (I2S_OUT_CLK_FREQ * 4)
            #define PLL_FREQ         (I2S_IN_CLK_FREQ * 6)
            #define I2S_IN_CLK       (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK      (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_IN_CLK_DIV3  (0)
            #define I2S_OUT_CLK_DIV3 (0)
        #elif (I2S_IN_CLK_FREQ * 4) == (I2S_OUT_CLK_FREQ * 3)
            #define PLL_FREQ         (I2S_IN_CLK_FREQ * 8)
            #define I2S_IN_CLK       (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK      (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_IN_CLK_DIV3  (0)
            #define I2S_OUT_CLK_DIV3 (0)
        #elif (I2S_IN_CLK_FREQ * 3) == (I2S_OUT_CLK_FREQ * 12)
            #define PLL_FREQ         (I2S_IN_CLK_FREQ * 6)
            #define I2S_IN_CLK       (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK      (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_IN_CLK_DIV3  (0)
            #define I2S_OUT_CLK_DIV3 (1)
        #elif (I2S_IN_CLK_FREQ * 4) == (I2S_OUT_CLK_FREQ * 9)
            #define PLL_FREQ         (I2S_IN_CLK_FREQ * 8)
            #define I2S_IN_CLK       (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK      (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_IN_CLK_DIV3  (0)
            #define I2S_OUT_CLK_DIV3 (1)
        #elif (I2S_IN_CLK_FREQ * 9) == (I2S_OUT_CLK_FREQ * 4)
            #define PLL_FREQ         (I2S_IN_CLK_FREQ * 6 * 3)
            #define I2S_IN_CLK       (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK      (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_IN_CLK_DIV3  (1)
            #define I2S_OUT_CLK_DIV3 (0)
        #elif (I2S_IN_CLK_FREQ * 12) == (I2S_OUT_CLK_FREQ * 3)
            #define PLL_FREQ         (I2S_IN_CLK_FREQ * 8 * 3)
            #define I2S_IN_CLK       (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK      (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_IN_CLK_DIV3  (1)
            #define I2S_OUT_CLK_DIV3 (0)
        #else
            #define PLL_FREQ         ERROR_VALUE
            #define I2S_IN_CLK       ERROR_VALUE
            #define I2S_OUT_CLK      ERROR_VALUE
            #define I2S_IN_CLK_DIV3  ERROR_VALUE
            #define I2S_OUT_CLK_DIV3 ERROR_VALUE
            #error "The frequency of PLL cannot satisfy both I2S_IN and I2S_OUT."
        #endif
    #endif

    #if ENUM_TEST_MODE == PDM_IN_ONLY
        #define PLL_FREQ            (PDM_CLKO_FREQ * PDM_CLOCK_DIV_VALUE)
        #define PDM_CLOCK_DIV_VALUE (4)
    #elif ENUM_TEST_MODE == PDM_TO_I2S
        #if ((I2S_OUT_CLK_FREQ * 6) % (PDM_CLKO_FREQ)) == 0
            #define I2S_OUT_CLK (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define PDM_CLOCK_DIV_VALUE_TMP ((I2S_OUT_CLK_FREQ * 6) / (PDM_CLKO_FREQ))
            #if PDM_CLOCK_DIV_VALUE_TMP < 4
                #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6 * 3)
                #define I2S_OUT_CLK_DIV3    (1)
                #define PDM_CLOCK_DIV_VALUE (PDM_CLOCK_DIV_VALUE_TMP * 3)
            #else
                #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6)
                #define I2S_OUT_CLK_DIV3    (0)
                #define PDM_CLOCK_DIV_VALUE (PDM_CLOCK_DIV_VALUE_TMP)
            #endif
        #elif ((I2S_OUT_CLK_FREQ * 8) % (PDM_CLKO_FREQ)) == 0
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define PDM_CLOCK_DIV_VALUE_TMP ((I2S_OUT_CLK_FREQ * 8) / (PDM_CLKO_FREQ))
            #if PDM_CLOCK_DIV_VALUE_TMP < 4
                #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8 * 3)
                #define I2S_OUT_CLK_DIV3    (1)
                #define PDM_CLOCK_DIV_VALUE (PDM_CLOCK_DIV_VALUE_TMP * 3)
            #else
                #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8)
                #define I2S_OUT_CLK_DIV3    (0)
                #define PDM_CLOCK_DIV_VALUE (PDM_CLOCK_DIV_VALUE_TMP)
            #endif
        #elif ((I2S_OUT_CLK_FREQ * 6 * 3) % (PDM_CLKO_FREQ)) == 0
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (1)
            #define PDM_CLOCK_DIV_VALUE ((I2S_OUT_CLK_FREQ * 6 * 3) / (PDM_CLKO_FREQ))
        #elif ((I2S_OUT_CLK_FREQ * 8 * 3) % (PDM_CLKO_FREQ)) == 0
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (1)
            #define PDM_CLOCK_DIV_VALUE ((I2S_OUT_CLK_FREQ * 8 * 3) / (PDM_CLKO_FREQ))
        #else
            #define PLL_FREQ            ERROR_VALUE
            #define I2S_OUT_CLK         ERROR_VALUE
            #define I2S_OUT_CLK_DIV3    ERROR_VALUE
            #define PDM_CLOCK_DIV_VALUE ERROR_VALUE
            #error "The frequency of PLL cannot satisfy both PDM_IN and I2S_OUT."
        #endif
    #endif

    #if ENUM_TEST_MODE == AUDADC_IN_ONLY
        #define PLL_FREQ             (UINT_SYS_SAMPLE_RATES * AUDADC_IRTT_MIN_PER)
        #define AUDADC_RPTT_CLK_DIV  (AM_HAL_AUDADC_RPTT_CLK_DIV2)
        #define AUDADC_COUNT_MAX     ((AUDADC_IRTT_MIN_PER + (1 << (AUDADC_RPTT_CLK_DIV - 1)) - 2) / (1 << AUDADC_RPTT_CLK_DIV))
    #elif ENUM_TEST_MODE == AUDADC_TO_I2S
        #if (((I2S_OUT_CLK_FREQ * 6) / UINT_SYS_SAMPLE_RATES) < 2047) && (((I2S_OUT_CLK_FREQ * 6) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (0)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV2)
            #if (PLL_FREQ % (2 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  ((PLL_FREQ / (2 * UINT_SYS_SAMPLE_RATES)) - 1)
            #else
                #define AUDADC_COUNT_MAX  ((((2 * PLL_FREQ) / (2 * UINT_SYS_SAMPLE_RATES) + 1) / 2) - 1)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 6) / UINT_SYS_SAMPLE_RATES) < 4094) && (((I2S_OUT_CLK_FREQ * 6) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (0)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV4)
            #if (PLL_FREQ % (4 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (4 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (4 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 6) / UINT_SYS_SAMPLE_RATES) < 8188) && (((I2S_OUT_CLK_FREQ * 6) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (0)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV8)
            #if (PLL_FREQ % (8 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (8 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (8 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 6) / UINT_SYS_SAMPLE_RATES) < 16376) && (((I2S_OUT_CLK_FREQ * 6) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (0)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV16)
            #if (PLL_FREQ % (16 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (16 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (16 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 6) / UINT_SYS_SAMPLE_RATES) < 32752) && (((I2S_OUT_CLK_FREQ * 6) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (0)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV32)
            #if (PLL_FREQ % (32 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (32 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (32 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 8) / UINT_SYS_SAMPLE_RATES) < 2047) && (((I2S_OUT_CLK_FREQ * 8) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (0)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV2)
            #if (PLL_FREQ % (2 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  ((PLL_FREQ / (2 * UINT_SYS_SAMPLE_RATES)) - 1)
            #else
                #define AUDADC_COUNT_MAX  ((((2 * PLL_FREQ) / (2 * UINT_SYS_SAMPLE_RATES) + 1) / 2) - 1)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 8) / UINT_SYS_SAMPLE_RATES) < 4094) && (((I2S_OUT_CLK_FREQ * 8) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (0)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV4)
            #if (PLL_FREQ % (4 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (4 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (4 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 8) / UINT_SYS_SAMPLE_RATES) < 8188) && (((I2S_OUT_CLK_FREQ * 8) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (0)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV8)
            #if (PLL_FREQ % (8 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (8 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (8 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 8) / UINT_SYS_SAMPLE_RATES) < 16376) && (((I2S_OUT_CLK_FREQ * 8) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (0)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV16)
            #if (PLL_FREQ % (16 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (16 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (16 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 8) / UINT_SYS_SAMPLE_RATES) < 32752) && (((I2S_OUT_CLK_FREQ * 8) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (0)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV32)
            #if (PLL_FREQ % (32 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (32 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (32 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 6 * 3) / UINT_SYS_SAMPLE_RATES) < 2047) && (((I2S_OUT_CLK_FREQ * 6 * 3) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (1)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV2)
            #if (PLL_FREQ % (2 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  ((PLL_FREQ / (2 * UINT_SYS_SAMPLE_RATES)) - 1)
            #else
                #define AUDADC_COUNT_MAX  ((((2 * PLL_FREQ) / (2 * UINT_SYS_SAMPLE_RATES) + 1) / 2) - 1)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 6 * 3) / UINT_SYS_SAMPLE_RATES) < 4094) && (((I2S_OUT_CLK_FREQ * 6 * 3) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (1)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV4)
            #if (PLL_FREQ % (4 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (4 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (4 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 6 * 3) / UINT_SYS_SAMPLE_RATES) < 8188) && (((I2S_OUT_CLK_FREQ * 6 * 3) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (1)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV8)
            #if (PLL_FREQ % (8 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (8 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (8 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 6 * 3) / UINT_SYS_SAMPLE_RATES) < 16376) && (((I2S_OUT_CLK_FREQ * 6 * 3) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (1)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV16)
            #if (PLL_FREQ % (16 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (16 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (16 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 6 * 3) / UINT_SYS_SAMPLE_RATES) < 32752) && (((I2S_OUT_CLK_FREQ * 6 * 3) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 6 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT3)
            #define I2S_OUT_CLK_DIV3    (1)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV32)
            #if (PLL_FREQ % (32 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (32 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (32 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 8 * 3) / UINT_SYS_SAMPLE_RATES) < 2047) && (((I2S_OUT_CLK_FREQ * 8 * 3) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (1)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV2)
            #if (PLL_FREQ % (2 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  ((PLL_FREQ / (2 * UINT_SYS_SAMPLE_RATES)) - 1)
            #else
                #define AUDADC_COUNT_MAX  ((((2 * PLL_FREQ) / (2 * UINT_SYS_SAMPLE_RATES) + 1) / 2) - 1)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 8 * 3) / UINT_SYS_SAMPLE_RATES) < 4094) && (((I2S_OUT_CLK_FREQ * 8 * 3) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (1)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV4)
            #if (PLL_FREQ % (4 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (4 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (4 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 8 * 3) / UINT_SYS_SAMPLE_RATES) < 8188) && (((I2S_OUT_CLK_FREQ * 8 * 3) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (1)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV8)
            #if (PLL_FREQ % (8 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (8 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (8 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 8 * 3) / UINT_SYS_SAMPLE_RATES) < 16376) && (((I2S_OUT_CLK_FREQ * 8 * 3) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (1)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV16)
            #if (PLL_FREQ % (16 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (16 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (16 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #elif (((I2S_OUT_CLK_FREQ * 8 * 3) / UINT_SYS_SAMPLE_RATES) < 32752) && (((I2S_OUT_CLK_FREQ * 8 * 3) / UINT_SYS_SAMPLE_RATES) >= AUDADC_IRTT_MIN_PER)
            #define PLL_FREQ            (I2S_OUT_CLK_FREQ * 8 * 3)
            #define I2S_OUT_CLK         (eAM_HAL_I2S_CLKSEL_PLL_FOUT4)
            #define I2S_OUT_CLK_DIV3    (1)
            #define AUDADC_RPTT_CLK_DIV (AM_HAL_AUDADC_RPTT_CLK_DIV32)
            #if (PLL_FREQ % (32 * UINT_SYS_SAMPLE_RATES)) == 0
                #define AUDADC_COUNT_MAX  (PLL_FREQ / (32 * UINT_SYS_SAMPLE_RATES))
            #else
                #define AUDADC_COUNT_MAX  (((2 * PLL_FREQ) / (32 * UINT_SYS_SAMPLE_RATES) + 1) / 2)
            #endif
        #else
            #define PLL_FREQ            ERROR_VALUE
            #define I2S_OUT_CLK         ERROR_VALUE
            #define I2S_OUT_CLK_DIV3    ERROR_VALUE
            #define AUDADC_RPTT_CLK_DIV ERROR_VALUE
            #define AUDADC_COUNT_MAX    ERROR_VALUE
            #error "The frequency of PLL cannot satisfy both AUDADC_IN and I2S_OUT."
        #endif
    #endif

    #if PLL_FREQ != ERROR_VALUE
        #pragma message "PLL_FREQ: " VALUE(PLL_FREQ)
    #endif

#endif

#if ENUM_TEST_MODE & PDM_IN
    #if PDM_CLOCK_DIV_VALUE < 4
        #pragma message VALUE(PDM_CLOCK_DIV_VALUE)
        #error "PDM_CLOCK_DIV_VALUE shouldn't be less than 4."
    #elif PDM_CLOCK_DIV_VALUE > 64
        #pragma message VALUE(PDM_CLOCK_DIV_VALUE)
        #error "PDM_CLOCK_DIV_VALUE shouldn't be greater than 64."
    #endif

    #if (PDM_CLOCK_DIV_VALUE % 2) == 0
        #define PDM_MCLK_DIV_VALUE   (2)
        #define PDM_CLKO_DIV_VALUE   (PDM_CLOCK_DIV_VALUE / 2)
    #elif (PDM_CLOCK_DIV_VALUE % 3) == 0
        #define PDM_MCLK_DIV_VALUE   (3)
        #define PDM_CLKO_DIV_VALUE   (PDM_CLOCK_DIV_VALUE / 3)
    #elif (PDM_CLOCK_DIV_VALUE % 4) == 0
        #define PDM_MCLK_DIV_VALUE   (4)
        #define PDM_CLKO_DIV_VALUE   (PDM_CLOCK_DIV_VALUE / 4)
    #else
        #error "Wrong PDM_CLOCK_DIV_VALUE."
    #endif

    #if PDM_MCLK_DIV_VALUE == 2
        #define PDM_MCLK_DIV    (AM_HAL_PDM_MCLKDIV_1)
        #elif PDM_MCLK_DIV_VALUE == 3
    #define PDM_MCLK_DIV    (AM_HAL_PDM_MCLKDIV_2)
        #elif PDM_MCLK_DIV_VALUE == 4
        #define PDM_MCLK_DIV    (AM_HAL_PDM_MCLKDIV_3)
    #else
        #error "Wrong PDM MCLK DIV."
    #endif

    #if PDM_CLKO_DIV_VALUE == 2
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV1)
    #elif PDM_CLKO_DIV_VALUE == 3
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV2)
    #elif PDM_CLKO_DIV_VALUE == 4
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV3)
    #elif PDM_CLKO_DIV_VALUE == 5
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV4)
    #elif PDM_CLKO_DIV_VALUE == 6
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV5)
    #elif PDM_CLKO_DIV_VALUE == 7
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV6)
    #elif PDM_CLKO_DIV_VALUE == 8
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV7)
    #elif PDM_CLKO_DIV_VALUE == 9
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV8)
    #elif PDM_CLKO_DIV_VALUE == 10
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV9)
    #elif PDM_CLKO_DIV_VALUE == 11
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV10)
    #elif PDM_CLKO_DIV_VALUE == 12
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV11)
    #elif PDM_CLKO_DIV_VALUE == 13
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV12)
    #elif PDM_CLKO_DIV_VALUE == 14
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV13)
    #elif PDM_CLKO_DIV_VALUE == 15
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV14)
    #elif PDM_CLKO_DIV_VALUE == 16
        #define PDM_CLKO_DIV    (AM_HAL_PDM_PDMA_CLKO_DIV15)
    #else
        #error "Wrong PDM CLKO DIV."
    #endif

    #if UINT_PDM_CHANNELS == 1
        #define PDM_CHANNEL (AM_HAL_PDM_CHANNEL_LEFT)
    #elif UINT_PDM_CHANNELS == 2
        #define PDM_CHANNEL (AM_HAL_PDM_CHANNEL_STEREO)
    #endif
#endif


//*****************************************************************************
//
// DMA buffer size
//
//*****************************************************************************
#if UINT_I2S_IN_BITS == 8
#define I2S_IN_SAMPLE_SIZE  (1)
typedef int8_t i2sInSample;
#elif UINT_I2S_IN_BITS == 16
#define I2S_IN_SAMPLE_SIZE  (2)
typedef int8_t i2sInSample;
#elif UINT_I2S_IN_BITS == 24
#define I2S_IN_SAMPLE_SIZE  (4)
typedef int8_t i2sInSample;
#elif UINT_I2S_IN_BITS == 32
#define I2S_IN_SAMPLE_SIZE  (4)
typedef int32_t i2sInSample;
#endif

#if UINT_I2S_OUT_BITS == 8
#define I2S_OUT_SAMPLE_SIZE     (1)
#define AUDADC_SAMPLE_SHIFT(x)  ((x) >> 8)
#define PDM_SAMPLE_SHIFT(x)     ((x) >> 16)
typedef int8_t i2sOutSample;
#elif UINT_I2S_OUT_BITS == 16
#define I2S_OUT_SAMPLE_SIZE     (2)
#define AUDADC_SAMPLE_SHIFT(x)  (x)
#define PDM_SAMPLE_SHIFT(x)     ((x) >> 8)
typedef int16_t i2sOutSample;
#elif UINT_I2S_OUT_BITS == 24
#define I2S_OUT_SAMPLE_SIZE     (4)
#define AUDADC_SAMPLE_SHIFT(x)  ((x) << 8)
#define PDM_SAMPLE_SHIFT(x)     (x)
typedef int32_t i2sOutSample;
#elif UINT_I2S_OUT_BITS == 32
#define I2S_OUT_SAMPLE_SIZE     (4)
#define AUDADC_SAMPLE_SHIFT(x)  ((x) << 16)
#define PDM_SAMPLE_SHIFT(x)     ((x) << 8)
typedef int32_t i2sOutSample;
#endif

#define PDM_SAMPLE_SIZE      (4)
#define AUDADC_SAMPLE_SIZE   (4)

#define I2S_IN_DMA_BUF_SIZE     (I2S_IN_SAMPLE_SIZE * UINT_SYS_SAMPLE_RATES * UINT_I2S_IN_CHANNELS * UINT_DMA_FRAME_MS / 1000)
#define I2S_OUT_DMA_BUF_SIZE    (I2S_OUT_SAMPLE_SIZE * UINT_SYS_SAMPLE_RATES * UINT_I2S_OUT_CHANNELS * UINT_DMA_FRAME_MS / 1000)
#define PDM_DMA_BUF_SIZE        (PDM_SAMPLE_SIZE * UINT_SYS_SAMPLE_RATES * UINT_PDM_CHANNELS * UINT_DMA_FRAME_MS / 1000)
#define AUDADC_DMA_BUF_SIZE     (I2S_IN_SAMPLE_SIZE * UINT_SYS_SAMPLE_RATES * 1 * UINT_DMA_FRAME_MS / 1000)

#endif
