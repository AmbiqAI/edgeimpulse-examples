//*****************************************************************************
//
//! @file pipeline.h
//!
//! @brief Functions and variables related to the button task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef PIPELINE_H
#define PIPELINE_H

#ifdef __cplusplus
extern "C"
{
#endif

    enum
    {
        ST_STOP,
        ST_PAUSE,
        ST_PLAYING
    };

    /**
     * @brief Create pipeline.
     *
     * @return true if create successfully, otherwise false.
     */
    bool pipeline_create(void);

    /**
     * @brief Set pipeline state.
     *
     * @param state Pipeline state(ST_STOP | ST_PAUSE | ST_PLAYING) to be set.
     * @retval 0 Pipeline set state successfully.
     * @retval -ENODEV Pipeline has not created.
     * @retval -EINVAL Parameter state is invalid.
     * @retval -ETIMEDOUT Pipeline set state timeout.
     */
    int pipeline_set_state(int state);

    /**
     * @brief Destroy pipeline.
     */
    void pipeline_destroy(void);

#ifdef __cplusplus
}
#endif

#endif // PIPELINE_H
