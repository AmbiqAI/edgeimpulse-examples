/**
 ******************************************************************************
 *
 * @file acore_msg_struct.h
 *
 * @brief Declaration Audio core message struct.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_MSG_STRUCT_H_
#define _ACORE_MSG_STRUCT_H_

#include <stdint.h>

/**
 * Phonet protocol header.
 */
struct phonethdr
{
    uint8_t  pn_media;  /**< Media type (link-layer identifier) */
    uint8_t  pn_rdev;   /**< Receiver device ID */
    uint8_t  pn_sdev;   /**< Sender device ID */
    uint8_t  pn_res;    /**< Resource ID or function */
    uint16_t pn_length; /**< Big-endian number of bytes following this field.
                        Value also euqals total message length minus 6. Max of
                        total message length is 512 according to USB packet
                        size, so max of pn_length is (512 - 6)*/
    uint8_t pn_robj;    /**< Receiver object ID */
    uint8_t pn_sobj;    /**< Sender object ID */
} __attribute__((packed));

/**
 * Audio core message format
 */
typedef struct acore_msg
{
    struct phonethdr pn_hdr;       /**< Phonet header */
    uint8_t          trans_id;     /**< Transaction id */
    uint8_t          msg_id;       /**< Message id */
    uint8_t          user_data[0]; /**< User data */
} __attribute__((packed)) acore_msg_t;

/**
 * Audio core message sub block header
 */
typedef struct acore_msg_sb_header
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
} __attribute__((packed)) acore_msg_sb_header_t;

/**
 * Audio HW control request
 */
typedef struct hw_ctrl_req
{
    uint8_t  trans_id;         /**< Transaction id */
    uint8_t  msg_id;           /**< Message id */
    uint16_t num_of_subblocks; /**< Number of sub blocks to be request */
    uint8_t  subblock[0]; /**< Sub blocks if num_of_subblocks is non-zero */
} __attribute__((packed)) hw_ctrl_req_t;

/**
 * Audio HW control response
 */
typedef struct hw_ctrl_resp
{
    uint8_t  trans_id;         /**< Transaction id */
    uint8_t  msg_id;           /**< Message id */
    uint16_t num_of_subblocks; /**< Number of sub blocks that failed */
    uint16_t subblock_id[0];   /**< Sub block id that failed */
} __attribute__((packed)) hw_ctrl_resp_t;

/**
 * Audio HW read request
 */
typedef struct hw_read_req
{
    uint8_t  trans_id;         /**< Transaction id */
    uint8_t  msg_id;           /**< Message id */
    uint16_t num_of_subblocks; /**< Number of sub blocks to be request */
    uint16_t subblock_id[0];   /**< Sub block id that to be request */
} __attribute__((packed)) hw_read_req_t;

/**
 * Audio HW read response
 */
typedef struct hw_read_resp
{
    uint8_t  trans_id;         /**< Transaction id */
    uint8_t  msg_id;           /**< Message id */
    uint16_t num_of_subblocks; /**< Number of sub blocks that responded */
    uint8_t  subblock[0];      /**< Sub blocks that responded */
} __attribute__((packed)) hw_read_resp_t;

/**
 * Audio tone generate request
 */
typedef struct tone_gen_req
{
    uint8_t  trans_id; /**< Transaction id */
    uint8_t  msg_id;   /**< Message id */
    uint8_t  filter;
    uint8_t  slopes   : 3;
    uint8_t  waveform : 3;
    uint8_t  output   : 1;
    uint8_t  switches : 1;
    uint16_t gain;
    uint16_t freq1;
    uint16_t freq2;
    uint16_t freq3;
    uint16_t freq4;
    uint8_t  init_level;
    uint8_t  init_dur;
    uint8_t  attack_level;
    uint8_t  attack_dur;
    uint8_t  sustain_level;
    uint8_t  decay_dur;
    uint8_t  release_level;
    uint8_t  release_dur;
} __attribute__((packed)) tone_gen_req_t;

/**
 * Audio buffers request
 */
typedef struct buf_req
{
    uint8_t  trans_id; /**< Transaction id */
    uint8_t  msg_id;   /**< Message id */
    uint16_t filter;
    uint8_t *music_play_buf_addr; /**< Music playback buffer address*/
    uint8_t *music_rec_buf_addr;  /**< Music record buffer address*/
    uint8_t *voice_play_buf_addr; /**< Voice playback buffer address*/
    uint8_t *voice_rec_buf_addr;  /**< Voice record buffer address*/
    uint32_t music_play_buf_len;  /**< Length of music playback buffer */
    uint32_t music_rec_buf_len;   /**< Length of music record buffer */
    uint32_t voice_play_buf_len;  /**< Length of voice playback buffer */
    uint32_t voice_rec_buf_len;   /**< Length of voice record buffer */
} __attribute__((packed)) buf_req_t;

/**
 * Audio buffers response
 */
typedef struct buf_resp
{
    uint8_t  trans_id; /**< Transaction id */
    uint8_t  msg_id;   /**< Message id */
    uint16_t filter;
    uint8_t *music_play_buf_addr; /**< Music playback buffer address*/
    uint8_t *music_rec_buf_addr;  /**< Music record buffer address*/
    uint8_t *voice_play_buf_addr; /**< Voice playback buffer address*/
    uint8_t *voice_rec_buf_addr;  /**< Voice record buffer address*/
    uint32_t music_play_buf_len;  /**< Length of music playback buffer */
    uint32_t music_rec_buf_len;   /**< Length of music record buffer */
    uint32_t voice_play_buf_len;  /**< Length of voice playback buffer */
    uint32_t voice_rec_buf_len;   /**< Length of voice record buffer */
} __attribute__((packed)) buf_resp_t;

/**
 * Audio config request
 */
typedef struct conf_req
{
    uint8_t  trans_id;     /**< Transaction id */
    uint8_t  msg_id;       /**< Message id */
    uint16_t pipe;         /**< Audio pipe */
    uint16_t active;       /**< Pipe activation */
    uint16_t mode;         /**< Audio mode: stereo or mono */
    uint16_t bit_depth;    /**< Bits per sample */
    uint16_t sample_rate;  /**< Specific sample rate, bit field */
    uint16_t circular_len; /**< Number of frames to be copied into the buffer */
} __attribute__((packed)) conf_req_t;

/**
 * Audio config response
 */
typedef struct conf_resp
{
    uint8_t  trans_id;     /**< Transaction id */
    uint8_t  msg_id;       /**< Message id */
    uint16_t pipe;         /**< Audio pipe */
    uint16_t response;     /**< Common response */
    uint16_t sample_rates; /**< Sample rates that HAL supports, bit field */
} __attribute__((packed)) conf_resp_t;

/**
 * Audio music playback data request
 */
typedef struct music_play_data_req
{
    uint8_t  trans_id; /**< Transaction id */
    uint8_t  msg_id;   /**< Message id */
    uint16_t pipe;     /**< Audio pipe */
    uint8_t *pwrite;
    uint16_t threshold;
    uint16_t mode;        /**< Audio mode: stereo or mono */
    uint16_t sample_rate; /**< Specific sample rate, bit field */
} __attribute__((packed)) music_play_data_req_t;

/**
 * Audio music playback data response
 */
typedef struct music_play_data_resp
{
    uint8_t  trans_id; /**< Transaction id */
    uint8_t  msg_id;   /**< Message id */
    uint16_t pipe;     /**< Audio pipe */
    uint8_t *pread;    /**< Start address*/
} __attribute__((packed)) music_play_data_resp_t;

/**
 * Audio record data request
 */
typedef struct record_data_req
{
    uint8_t  trans_id;  /**< Transaction id */
    uint8_t  msg_id;    /**< Message id */
    uint16_t pipe;      /**< Audio pipe */
    uint8_t *pread;     /**< Start address to be written by HAL */
    uint16_t threshold; /**< Number of empty frame slots */
} __attribute__((packed)) record_data_req_t;

/**
 * Audio record data response
 */
typedef struct record_data_resp
{
    uint8_t  trans_id; /**< Transaction id */
    uint8_t  msg_id;   /**< Message id */
    uint16_t pipe;     /**< Audio pipe */
    uint8_t *pwrite;
    uint16_t mode;        /**< Audio mode: stereo or mono */
    uint16_t sample_rate; /**< Specific sample rate, bit field */
} __attribute__((packed)) record_data_resp_t;

/**
 * Audio save node data notify
 */
typedef struct node_data_notify
{
    uint8_t  trans_id;    /**< Transaction id */
    uint8_t  msg_id;      /**< Message id */
    uint8_t  direction;   /**< Input data or output data */
    uint8_t  index;       /**< Node index */
    uint8_t  mode;        /**< Audio mode: stereo or mono */
    uint16_t sample_rate; /**< Specific sample rate, bit field */
    uint16_t length;      /**< Pcm data length */
    int16_t  pcm[0];      /**< Pcm data */
} __attribute__((packed)) node_data_notify_t;

/**
 * Audio sub block algorithm switch
 */
typedef struct sb_algo_switch
{
    uint16_t block_id;     /**< Block id */
    uint16_t block_len;    /**< Block length */
    uint16_t switch1  : 1; /**< HAL_Audio_Control_Echo_Canceller_Parameters */
    uint16_t switch2  : 1; /**< HAL_Audio_Control_Noise_Suppressor_Parameters */
    uint16_t switch3  : 1; /**< HAL_Audio_Control_SB_NLP_Parameters */
    uint16_t switch4  : 1; /**< HAL_Audio_Control_DL_MBComp_Parameters */
    uint16_t switch5  : 1; /**< HAL_Audio_Control_UL_DRC_Parameters */
    uint16_t switch6  : 1; /**< HAL_Audio_Control_UL_AGC_Parameters */
    uint16_t switch7  : 1; /**< HAL_Audio_Control_DL_AGC_Parameters */
    uint16_t switch8  : 1; /**< HAL_Audio_Control_UL_Exp_Parameters */
    uint16_t switch9  : 1; /**< HAL_Audio_Control_DL_Exp_Parameters */
    uint16_t switch10 : 7; /**< Switch bit field reserved */
} __attribute__((packed)) sb_algo_switch_t;

/**
 * Audio sub block algorithm switch
 */
typedef struct sb_sniffer_enable
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length. Total message length can't exceed the
                           value limitation of phonethdr.pn_length */
    uint8_t enable    : 1; /**< Sniffer enable flag */
    uint8_t direction : 1; /**< Sniffer input/output flag. 0 is input, 1 is
                              output */
    uint8_t index    : 4;  /**< Sniffer node index */
    uint8_t reserved : 2;  /**< Reserved */
} __attribute__((packed)) sb_sniffer_enable_t;

/**
 * Audio sub block algorithm switch
 */
typedef struct sb_sniffer_active
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t active;    /**< Sniffer active flag */
} __attribute__((packed)) sb_sniffer_active_t;

/**
 * Audio sub block bypass settings
 */
typedef struct sb_bypass
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint8_t  bypass;    /**< 1: enable bypass, 0: disable bypass */
    uint8_t  reserved;  /**< Reserved for alignment */
} __attribute__((packed)) sb_bypass_t;

/**
 * Audio sub block prompt
 */
typedef struct sb_prompt
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint8_t  type;      /**< Prompt type */
    uint8_t  index;     /**< Prompt index of current type */
    uint16_t frequency; /**< Sine wave frequency(when type is sine wave) */
    uint16_t duration;  /**< Playing time in milliseconds */
} __attribute__((packed)) sb_prompt_t;

/**
 * Audio sub block mixer gains settings
 */
typedef struct sb_mixer_gains_settings
{
    uint16_t block_id;    /**< Block id */
    uint16_t block_len;   /**< Block length */
    uint16_t tone_gain;   /**< Tone generator gain, Q1.14, linear 0~2.0 */
    uint16_t speech_gain; /**< Speech playback gain, Q1.14, linear 0~2.0 */
    uint16_t music_gain;  /**< Music playback gain, Q1.14, linear 0~2.0 */
    uint16_t record_gain; /**< HQ audio recording gain, Q1.14, linear 0~2.0 */
} __attribute__((packed)) sb_mixer_gains_settings_t;

/**
 * Audio sub block mute settings
 */
typedef struct sb_mute_settings
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t mute;      /**< Bit field */
    uint16_t filter;
} __attribute__((packed)) sb_mute_settings_t;

/**
 * Audio sub block primary speaker settings
 */
typedef struct sb_prim_spk_settings
{
    uint16_t block_id;   /**< Block id */
    uint16_t block_len;  /**< Block length */
    uint8_t  ep_mode;    /**< Endpoint mode */
    uint8_t  ep_id;      /**< Endpoint id */
    uint16_t logic_vol;  /**< Logic volume, range 0 to 9 */
    uint16_t ctl_obj;    /**< Controlled object */
    int16_t  gain_left;  /**< Left channel gain, Q14.1 value in dB */
    int16_t  gain_right; /**< Right channel gain, Q14.1 value in dB */
    uint16_t filter1;
    uint16_t sidetone; /**< Linear factor, 0~2.0, Q1.14 */
    uint16_t filter2;
} __attribute__((packed)) sb_prim_spk_settings_t;

/**
 * Audio sub block hearable settings
 */
typedef struct sb_hearable_settings
{
    uint16_t block_id;   /**< Block id */
    uint16_t block_len;  /**< Block length */
    uint8_t  ep_mode;    /**< Endpoint mode */
    uint8_t  ep_id;      /**< Endpoint id */
    uint16_t hearable;   /**< Controlled object */
    int16_t  gain_left;  /**< Left channel gain, Q14.1 value in dB */
    int16_t  gain_right; /**< Right channel gain, Q14.1 value in dB */
    uint16_t filter;
    uint16_t sidetone; /**< Linear factor, 0~2.0, Q1.14 */
} __attribute__((packed)) sb_hearable_settings_t;

/**
 * Audio sub block reserved 1
 */
typedef struct sb_reserved1
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t filter1;
    uint16_t filter2;
} __attribute__((packed)) sb_reserved1_t;

/**
 * Audio sub block reserved 2
 */
typedef struct sb_reserved2
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t filter1;
    uint16_t filter2;
} __attribute__((packed)) sb_reserved2_t;

/**
 * Audio sub block roundtrip latency settings
 */
typedef struct sb_roundtrip_latency
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t latency; /**< AEC echo path roundtrip delay in sample. Max 35ms */
    uint16_t filter;
} __attribute__((packed)) sb_roundtrip_latency_t;

/**
 * Audio sub block AGC attack time
 */
typedef struct sb_agc_attack_time
{
    uint16_t block_id;    /**< Block id */
    uint16_t block_len;   /**< Block length */
    int16_t  attack_time; /**< Attack time in millisecond */
} __attribute__((packed)) sb_agc_attack_time_t;

/**
 * Audio sub block AGC decay time
 */
typedef struct sb_agc_decay_time
{
    uint16_t block_id;   /**< Block id */
    uint16_t block_len;  /**< Block length */
    int16_t  decay_time; /**< Decay time in millisecond */
} __attribute__((packed)) sb_agc_decay_time_t;

/**
 * Audio sub block AGC target
 */
typedef struct sb_agc_target
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    int16_t  level;     /**< Target level */
} __attribute__((packed)) sb_agc_target_t;

/**
 * Audio sub block DRC attack time
 */
typedef struct sb_drc_attack_time
{
    uint16_t block_id;    /**< Block id */
    uint16_t block_len;   /**< Block length */
    int16_t  attack_time; /**< Attack time in millisecond */
} __attribute__((packed)) sb_drc_attack_time_t;

/**
 * Audio sub block DRC decay time
 */
typedef struct sb_drc_decay_time
{
    uint16_t block_id;   /**< Block id */
    uint16_t block_len;  /**< Block length */
    int16_t  decay_time; /**< Decay time in millisecond */
} __attribute__((packed)) sb_drc_decay_time_t;

/**
 * Audio sub block DRC knee threshold
 */
typedef struct sb_drc_knee_threshold
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    int16_t  knee_thr;  /**< Knee threshold */
} __attribute__((packed)) sb_drc_knee_threshold_t;

/**
 * Audio sub block DRC noise gate
 */
typedef struct sb_drc_noise_gate
{
    uint16_t block_id;   /**< Block id */
    uint16_t block_len;  /**< Block length */
    int16_t  noise_gate; /**< Noise gate */
} __attribute__((packed)) sb_drc_noise_gate_t;

/**
 * Audio sub block DRC slope
 */
typedef struct sb_drc_slope
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    int16_t  slope;     /**< Slope */
} __attribute__((packed)) sb_drc_slope_t;

/**
 * Audio sub block speaker left EQ center frequency settings
 */
typedef struct sb_spk_left_eq_freq
{
    uint16_t block_id;       /**< Block id */
    uint16_t block_len;      /**< Block length */
    uint16_t band_num;       /**< Number of band */
    int32_t  center_freq[0]; /**< Center frequency */
} __attribute__((packed)) sb_spk_left_eq_freq_t;

/**
 * Audio sub block speaker left EQ Q factor settings
 */
typedef struct sb_spk_left_eq_qfactor
{
    uint16_t block_id;   /**< Block id */
    uint16_t block_len;  /**< Block length */
    uint16_t band_num;   /**< Number of band */
    int32_t  qfactor[0]; /**< Q factor */
} __attribute__((packed)) sb_spk_left_eq_qfactor_t;

/**
 * Audio sub block speaker left EQ gain settings
 */
typedef struct sb_spk_left_eq_gain
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t band_num;  /**< Number of band */
    int32_t  gain[0];   /**< Gain list of every band */
} __attribute__((packed)) sb_spk_left_eq_gain_t;

/**
 * Audio sub block MBDRC frequency high bound settings
 */
typedef struct sb_mbdrc_bound_high
{
    uint16_t block_id;      /**< Block id */
    uint16_t block_len;     /**< Block length */
    uint16_t band_num;      /**< Number of band */
    int32_t  bound_high[0]; /**< Frequency high bound */
} __attribute__((packed)) sb_mbdrc_bound_high_t;

/**
 * Audio sub block MBDRC compress threshold settings
 */
typedef struct sb_mbdrc_comp_threshold
{
    uint16_t block_id;     /**< Block id */
    uint16_t block_len;    /**< Block length */
    uint16_t band_num;     /**< Number of band */
    int16_t  threshold[0]; /**< Compress threshold */
} __attribute__((packed)) sb_mbdrc_comp_threshold_t;

/**
 * Audio sub block MBDRC compress slope settings
 */
typedef struct sb_mbdrc_comp_slope
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t band_num;  /**< Number of band */
    int32_t  slope[0];  /**< Compress slope */
} __attribute__((packed)) sb_mbdrc_comp_slope_t;

/**
 * Audio sub block NS level
 */
typedef struct sb_ns_level
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    int16_t  level;     /**< ns level */
} __attribute__((packed)) sb_ns_level_t;

/**
 * Audio sub block AEC
 */
typedef struct sb_aec
{
    uint16_t block_id;    /**< Block id */
    uint16_t block_len;   /**< Block length */
    int16_t  enable;      /**< AEC enable */
    int16_t  filter_len;  /**< Filter length */
    int16_t  fixed_delay; /**< Fixed_delay */
    int16_t  nlp_enable;  /**< NLP enable */
    int16_t  nlp_level;   /**< NLP level */
} __attribute__((packed)) sb_aec_t;

/**
 * Audio sub block algorithm MCPS
 */
typedef struct sb_mcps
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t obj_num;   /**< Object number */
    struct
    {
        uint16_t id;   /**< Object ID */
        uint32_t mcps; /**< Million Cycles Per Second */
    } __attribute__((packed)) obj[0];
} __attribute__((packed)) sb_mcps_t;

typedef struct sb_aec_cfg
{
    uint16_t block_id;    /**< Block id */
    uint16_t block_len;   /**< Block length */
    uint16_t enable;      /**< AEC enable */
    uint16_t filter_len;  /**< Filter length */
    uint16_t fixed_delay; /**< Fixed_delay */
    uint16_t nlp_enable;  /**< NLP enable */
    uint16_t nlp_level;   /**< NLP level */
} __attribute__((packed)) sb_aec_cfg_t;

typedef struct sb_agc_cfg
{
    uint16_t block_id;     /**< Block id */
    uint16_t block_len;    /**< Block length */
    uint16_t enable;       /**< AGC enable */
    uint16_t attack_time;  /**< Attack time in millisecond */
    uint16_t decay_time;   /**< Decay time in millisecond */
    uint16_t target_level; /**< Target level */
} __attribute__((packed)) sb_agc_cfg_t;

typedef struct sb_ns_cfg
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t enable;    /**< NS enable */
    uint16_t level;     /**< NS level */
} __attribute__((packed)) sb_ns_cfg_t;

typedef struct sb_gain_cfg
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t enable;    /**< Gain enable */
    int16_t  gain;      /**< Gain in dB */
} __attribute__((packed)) sb_gain_cfg_t;

typedef struct sb_drc_cfg
{
    uint16_t block_id;    /**< Block id */
    uint16_t block_len;   /**< Block length */
    uint16_t enable;      /**< DRC enable */
    uint16_t attack_time; /**< Attack time in millisecond */
    uint16_t decay_time;  /**< Decay time in millisecond */
    int16_t  knee_thd;    /**< Knee threshold */
    int16_t  noise_gate;  /**< Noise gate */
    int16_t  slope;       /**< Slope */
} __attribute__((packed)) sb_drc_cfg_t;

typedef struct peq_band
{
    uint16_t enable;      /**< PEQ enable */
    uint16_t center_freq; /**< Center frequency */
    int16_t  gain;        /**< Gain */
    int32_t  qfactor;     /**< Q factor */
} __attribute__((packed)) peq_band_t;

typedef struct sb_peq_cfg
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t band_num;  /**< Number of band */
    struct
    {
        uint16_t enable;      /**< PEQ enable */
        uint16_t center_freq; /**< Center frequency */
        int16_t  gain;        /**< Gain */
        int32_t  qfactor;     /**< Q factor */
    } __attribute__((packed)) band[0];
} __attribute__((packed)) sb_peq_cfg_t;

typedef struct mbdrc_band
{
    uint16_t enable;     /**< MBDRC enable */
    uint16_t freq_bound; /**< Frequency high bound */
    int16_t  threshold;  /**< Compress threshold */
    int32_t  slope;      /**< Compress slope */
} __attribute__((packed)) mbdrc_band_t;

typedef struct sb_mbdrc_cfg
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t band_num;  /**< Number of band */
    struct
    {
        uint16_t enable;     /**< MBDRC enable */
        uint16_t freq_bound; /**< Frequency high bound */
        int16_t  threshold;  /**< Compress threshold */
        int32_t  slope;      /**< Compress slope */
    } __attribute__((packed)) band[0];
} __attribute__((packed)) sb_mbdrc_cfg_t;

typedef struct sb_iir_filter_cfg
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t enable;    /**< IIR filter enable */
} __attribute__((packed)) sb_iir_filter_cfg_t;

typedef struct sb_nnse_cfg
{
    uint16_t block_id;  /**< Block id */
    uint16_t block_len; /**< Block length */
    uint16_t enable;    /**< NNSE enable */
} __attribute__((packed)) sb_nnse_cfg_t;

/** struct phonethdr length 8 */
#define PHONET_HEADER_LEN (sizeof(struct phonethdr))

/** phonethdr length offset 6 */
#define PHONET_LEN_OFFSET (6)

/** Including transaction id(1 byte) and message id(1 byte) */
#define PAYLOAD_HEADER_LEN (2)

#define NUM_OF_SUBBLOCKS_LEN (2)

/** Including sub block id(2 bytes) and sub block length(2 bytes) */
#define SUB_BLOCK_HEADER_LEN (4)

#endif /* _ACORE_MSG_STRUCT_H_ */
