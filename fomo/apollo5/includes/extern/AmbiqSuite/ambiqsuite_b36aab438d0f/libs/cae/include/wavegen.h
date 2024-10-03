/**
 ****************************************************************************************
 *
 * @file wavegen.h
 *
 * @brief Declaration of Audio sine wave generator API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _WAVE_GEN_H_
#define _WAVE_GEN_H_

#if defined __cplusplus
extern "C"
{
#endif

    typedef struct wavegen_t *wavegen_inst;

    /**
     ****************************************************************************************
     * @brief Create wave generator instance
     *
     * @param  wave_fs      Audio wave frequency. Unit: Hz
     * @param  sample_rate  Sample rate. Unit: Hz
     * @param  gain    		Audio gain
     * @return              Wave generator instance
     ****************************************************************************************
     */
    wavegen_inst wavegen_create(unsigned int wave_fs, unsigned int sample_rate,
                                float gain);

    /**
     ****************************************************************************************
     * @brief Get buffer size required according to duration
     *
     * @param  inst      Wave generator instance
     * @param  duration  Duration of data. Unit: ms
     * @return           Buffer size
     ****************************************************************************************
     */
    int wavegen_get_buf_size(wavegen_inst inst, unsigned int duration);

    /**
     ****************************************************************************************
     * @brief Generate wave data by size
     *
     * @param  inst      Wave generator instance
     * @param  output_size  Output data size. Unit: Byte
     * @param  buf       Output buffer pointer
     * @param  buf_size  Buffer size
     * @return           Actual audio data output
     ****************************************************************************************
     */
    int wavegen_generate(wavegen_inst inst, unsigned int output_size, char *buf,
                         unsigned int buf_size);

    /**
     ****************************************************************************************
     * @brief Destroy wave generator instance
     *
     * @param  inst      Wave generator instance
     ****************************************************************************************
     */
    void wavegen_destroy(wavegen_inst inst);

#if defined __cplusplus
}
#endif

#endif // _WAVE_GEN_H_
