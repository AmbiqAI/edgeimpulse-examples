/**
 ******************************************************************************
 *
 * @file config_manager.h
 *
 * @brief Audio config manager APIs.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

#define CONFIG_HEAD_MAGIC (0x5B3A2D29)
#define CONFIG_TAIL_MAGIC (0x282D3A5D)

#define CONFIG_DEFAULT_BCD (0x14052001)

#define SUB_BLOCK_SIZE (16)

/**
 * @brief Sub block ID.
 * @note The value shouldn't be changed. If new sub block id is needed, append
 * it into then end of the list before SB_ID_MAX.
 */
typedef enum sub_block_id
{
    SB_ID_AEC = 0,
    SB_ID_NS,
    SB_ID_WNR,
    SB_ID_DRC_UL,
    SB_ID_DRC_DL,
    SB_ID_AGC_UL,
    SB_ID_AGC_DL,
    SB_ID_GAIN,
    SB_ID_PEQ_UL,
    SB_ID_PEQ_UL_1,
    SB_ID_PEQ_UL_2,
    SB_ID_PEQ_UL_3,
    SB_ID_PEQ_UL_4,
    SB_ID_PEQ_UL_5,
    SB_ID_PEQ_UL_6,
    SB_ID_PEQ_UL_7,
    SB_ID_PEQ_UL_8,
    SB_ID_PEQ_DL,
    SB_ID_PEQ_DL_1,
    SB_ID_PEQ_DL_2,
    SB_ID_PEQ_DL_3,
    SB_ID_PEQ_DL_4,
    SB_ID_PEQ_DL_5,
    SB_ID_PEQ_DL_6,
    SB_ID_PEQ_DL_7,
    SB_ID_PEQ_DL_8,
    SB_ID_MBDRC,
    SB_ID_MBDRC_1,
    SB_ID_MBDRC_2,
    SB_ID_MBDRC_3,
    SB_ID_MBDRC_4,
    SB_ID_MBDRC_5,
    SB_ID_MBDRC_6,
    SB_ID_MBDRC_7,
    SB_ID_MBDRC_8,
    SB_ID_MBDRC_9,
    SB_ID_MBDRC_10,
    SB_ID_MBDRC_11,
    SB_ID_MBDRC_12,
    SB_ID_MBDRC_13,
    SB_ID_MBDRC_14,
    SB_ID_MBDRC_15,
    SB_ID_MBDRC_16,
    SB_ID_IIR_FILTER,
    SB_ID_MIC_PGA,
    SB_ID_SPK_PGA,
    SB_ID_NNSE,
    SB_ID_MAX
} sub_block_id_t;

/**
 * @brief Data base structure of each algorithm parameters in one sub block.
 */
typedef union sub_block
{
    // Common data.
    uint8_t data[SUB_BLOCK_SIZE];

    // AEC algorithm.
    struct
    {
        uint16_t enable;
        uint16_t filter_len;
        uint16_t fixed_delay;
        uint16_t nlp_enable;
        uint16_t nlp_level;
        uint8_t  reserved[6];
    } __attribute__((packed)) aec;

    // NS algorithm.
    struct
    {
        uint16_t enable;
        uint16_t level;
        uint8_t  reserved[12];
    } __attribute__((packed)) ns;

    // WNR algorithm.
    struct
    {
        uint16_t enable;
        uint16_t level;
        uint8_t  reserved[12];
    } __attribute__((packed)) wnr;

    // DRC algorithm.
    struct
    {
        uint16_t enable;
        uint16_t attack_time;
        uint16_t decay_time;
        int16_t  knee_thd;
        int16_t  noise_gate;
        int16_t  slope;
        uint8_t  reserved[4];
    } __attribute__((packed)) drc;

    // AGC algorithm.
    struct
    {
        uint16_t enable;
        uint16_t attack_time;
        uint16_t decay_time;
        uint16_t target_level;
        uint8_t  reserved[8];
    } __attribute__((packed)) agc;

    // Gain algorithm.
    struct
    {
        uint16_t enable;
        int16_t  gain;
        uint8_t  reserved[12];
    } __attribute__((packed)) gain;

    // PEQ algorithm.
    union
    {
        struct
        {
            struct
            {
                uint32_t enable;
                uint16_t band_num;
                uint8_t  reserved[2];
            } __attribute__((packed)) ctrl;
            struct
            {
                uint16_t center_freq;
                int16_t  gain;
                float    qfactor;
            } __attribute__((packed)) band;
        } __attribute__((packed)) u;
        struct
        {
            uint16_t center_freq;
            int16_t  gain;
            float    qfactor;
        } __attribute__((packed)) band[2];
    } __attribute__((packed)) peq;

    // MBDRC algorithm.
    union
    {
        struct
        {
            struct
            {
                uint32_t enable;
                uint16_t band_num;
                uint8_t  reserved[2];
            } __attribute__((packed)) ctrl;
            struct
            {
                uint16_t freq;
                int16_t  compress_thd;
                float    compress_slope;
            } __attribute__((packed)) band;
        } __attribute__((packed)) u;
        struct
        {
            uint16_t freq;
            int16_t  compress_thd;
            float    compress_slope;
        } __attribute__((packed)) band[2];
    } __attribute__((packed)) mbdrc;

    // IIR Filter algorithm.
    struct
    {
        uint16_t enable;
        uint8_t  reserved[14];
    } __attribute__((packed)) iir_filter;

    // NNSE algorithm.
    struct
    {
        uint16_t enable;
        uint8_t  reserved[14];
    } __attribute__((packed)) nnse;
} __attribute__((packed)) sub_block_t;

/**
 * @brief Data base structure of the whole config.
 */
typedef struct config_block
{
    uint32_t    head;                 /**< Head Magic */
    uint32_t    bcd;                  /**< BCD version */
    uint16_t    num;                  /**< Sub block number */
    uint16_t    size;                 /**< Sub block size */
    sub_block_t sub_block[SB_ID_MAX]; /**< Sub block */
    uint32_t    tail;                 /**< Tail Magic */
} __attribute__((packed)) config_block_t;

/**
 * @brief Algorithm parameters check function callback.
 */
typedef struct param_check_cb
{
    /**
     * @brief Check algorithm parameters.
     *
     * @param id Sub-block ID described in sub_block_id_t.
     * @param data Pointer to algorithm parameters.
     * @param size Size of algorithm parameters.
     * @param priv Private pointer of the checking module.
     *
     * @return 0 if check successfully, otherwise error code.
     */
    int32_t (*check)(uint32_t id, const uint8_t *data, uint32_t size,
                     void *priv);

    // Private pointer of the checking module.
    void *priv;
} param_check_cb_t;

/**
 * @brief Algorithm notify function callback while data base is changed.
 */
typedef struct notify_cb
{
    /**
     * @brief Notify that the parameter of sub block ID is changed.
     * @param id Sub-block ID described in sub_block_id_t.
     * @param priv Private pointer of the receiving notify module.
     */
    void (*notify)(uint32_t id, void *priv);

    // Private pointer of the receiving notify module.
    void *priv;
} notify_cb_t;

/**
 * @brief Upgrade callback for users to check algorithm parameters.
 */
typedef struct upgrade_cb
{
    /**
     * @brief Check algorithm parameters while upgrading.
     *
     * @param[in] def Default config pointer.
     * @param[in] def_size Default config size.
     * @param[in] nvm NVM config pointer.
     * @param[in] nvm_size NVM config size.
     * @param[out] cfm Confirmed config pointer.
     * @param[out] cfm_size Confirmed config size.
     * @param[in] priv Private pointer.
     */
    bool (*upgrade)(const uint8_t *def, uint32_t def_size, const uint8_t *nvm,
                    uint32_t nvm_size, uint8_t **cfm, uint32_t *cfm_size,
                    void *priv);

    // Private pointer of the checking module.
    void *priv;
} upgrade_cb_t;

#if defined __cplusplus
extern "C"
{
#endif

    /**
     * @brief Audio configuration initialization.
     *
     * @return true if initializing successfully, otherwise false.
     */
    bool audio_config_init(void);

    /**
     * @brief Save audio configuration to NVM.
     */
    void audio_config_save_config(void);

    /**
     * @brief Read configuration of one sub block
     *
     * @param  Sub-block ID described in sub_block_id_t.
     * @param[out] data Pointer to the sub block data base.
     * @param[out] size Size of the sub block.
     * @return true if reading successfully, otherwise false.
     */
    bool audio_config_read_sb(uint32_t id, const uint8_t **data,
                              uint32_t *size);

    /**
     * @brief Register callback for checking algorithm parameters.
     *
     * @param cb Parameter checking callback.
     * @return true if registering successfully, otherwise false.
     */
    bool audio_config_register_param_check_cb(param_check_cb_t *cb);

    /**
     * @brief Register callback for notifying algorithm parameters is changed.
     *
     * @param cb Notify callback.
     * @return true if registering successfully, otherwise false.
     */
    bool audio_config_register_notify_cb(notify_cb_t *cb);

    /**
     * @brief Register callback for updating config while upgrading.
     *
     * @param cb Upgrade callback.
     * @return true if registering successfully, otherwise false.
     */
    bool audio_config_register_upgrade_cb(upgrade_cb_t *cb);
#if defined __cplusplus
}
#endif

#endif /* _CONFIG_MANAGER_H_ */
