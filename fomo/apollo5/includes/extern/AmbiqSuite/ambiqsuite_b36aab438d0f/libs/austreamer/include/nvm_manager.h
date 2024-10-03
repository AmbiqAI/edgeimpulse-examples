/**
 ******************************************************************************
 *
 * @file nvm_manager.h
 *
 * @brief Audio config NVM APIs.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _NVM_MANAGER_H_
#define _NVM_MANAGER_H_

#include <stdint.h>

#define CONFIG_ADDR (0x600000U)

#if defined __cplusplus
extern "C"
{
#endif

    /**
     * @brief Get NVM audio config size.
     *
     * @return Audio config size.
     */
    uint32_t audio_nvm_get_config_size(void);

    /**
     * @brief Load audio config from NVM to out buffer.
     *
     * @param data Pointer to store the audio config from NVM to here.
     * @param size Data buffer size, should be that of get config size.
     *
     * @return Actual read size.
     */
    uint32_t audio_nvm_load_config(uint8_t *data, uint32_t size);

    /**
     * @brief Save audio config to NVM.
     *
     * @param data Data pointer to be saved from here to NVM.
     * @param size Data size.
     *
     * @return Actual stored size.
     */
    uint32_t audio_nvm_save_config(uint8_t *data, uint32_t size);

    /**
     * @brief Erase audio config from NVM.
     */
    void audio_nvm_erase_config(void);

#if defined __cplusplus
}
#endif

#endif /* _NVM_MANAGER_H_ */
