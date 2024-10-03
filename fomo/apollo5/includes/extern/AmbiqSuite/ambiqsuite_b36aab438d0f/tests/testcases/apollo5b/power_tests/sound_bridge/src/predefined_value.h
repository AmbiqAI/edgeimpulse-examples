#ifndef PREDEFINED_VALUE_H
#define PREDEFINED_VALUE_H

#define MOUDLE_0                (0)
#define MOUDLE_1                (1)

#define I2S0_MODULE             (MOUDLE_0)
#define I2S1_MODULE             (MOUDLE_1)
#define PDM0_MODULE             (MOUDLE_0)
#define AUDADC0_MODULE          (MOUDLE_0)

#define I2S_IN                  (0x01)
#define PDM_IN                  (0x02)
#define AUDADC_IN               (0x04)
#define I2S_OUT                 (0x10)

#define I2S_IN_ONLY             (I2S_IN)
#define PDM_IN_ONLY             (PDM_IN)
#define AUDADC_IN_ONLY          (AUDADC_IN)
#define I2S_OUT_ONLY            (I2S_OUT)
#define I2S_TO_I2S              (I2S_OUT | I2S_IN)
#define PDM_TO_I2S              (I2S_OUT | PDM_IN)
#define AUDADC_TO_I2S           (I2S_OUT | AUDADC_IN)
#define DEBUG                   (I2S_OUT | AUDADC_IN | PDM_IN | I2S_IN)

#define I2S_ROLE_MASTER         (1)
#define I2S_ROLE_SLAVE          (2)

#define CLKSRC_HFRC             (1)
#define CLKSRC_PLL              (2)
#define CLKSRC_HF2ADJ           (3)

#define I2S_IN_ISR_IO           (130)
#define I2S_OUT_ISR_IO          (131)
#define PDM_ISR_IO              (132)
#define AUDADC_ISR_IO           (133)

#define ERROR_VALUE             (0xFFFFFFFF)

#define AUDADC_NB_CHANNELS          (1)
#define AUDADC_NB_SAMPLES           (48*50)
#define PREAMP_FULL_GAIN            (12)
#define CH_A0_GAIN_DB               (12)
#define CH_A1_GAIN_DB               (12)

#endif // PREDEFINED_VALUE_H
