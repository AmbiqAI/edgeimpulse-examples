//*****************************************************************************
//
//! @file ai_callback.h
//!
//! @brief AI callback function example
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef __AI_CALLBACK_H__
#define __AI_CALLBACK_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief AI NNSE initialization callback function.
     *
     * @param para Pointer of parameters.
     * @param size Size of parameters.
     * @return Return 0 for success. Return other for fail.
     */
    int32_t nnse_init(void *para, uint32_t size);

    /**
     * @brief AI NNSE callback function.
     *
     * @param in Pointer of input data
     * @param out Pointer of output data
     * @param samples Sample number. One sample data is 16bit width.
     * @return Return 0 for success. Return other for fail.
     */
    int32_t nnse_process(int16_t *in, int16_t *out, uint32_t samples);

    /**
     * @brief AI NNSE de-initialization callback function.
     *
     */
    void nnse_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // __AI_CALLBACK_H__
