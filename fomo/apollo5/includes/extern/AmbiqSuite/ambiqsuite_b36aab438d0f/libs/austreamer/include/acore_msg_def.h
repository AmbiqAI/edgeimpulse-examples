/**
 ******************************************************************************
 *
 * @file acore_msg_def.h
 *
 * @brief Declaration Audio core message definition.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_MSG_DEF_H_
#define _ACORE_MSG_DEF_H_

/**
 * @brief Message IDs
 */
#define HAL_Audio_HW_Control_Request     0x1B
#define HAL_Audio_HW_Control_Response    0x1C
#define HAL_Audio_HW_Read_Request        0x1D
#define HAL_Audio_HW_Read_Response       0x1E
#define HAL_Audio_Buffers_Request        0x30
#define HAL_Audio_Buffers_Response       0x31
#define HAL_Audio_Config_Request         0x32
#define HAL_Audio_Config_Response        0x33
#define HAL_Audio_Playback_Data_Request  0x34
#define HAL_Audio_Playback_Data_Response 0x35
#define HAL_Audio_Record_Data_Request    0x36
#define HAL_Audio_Record_Data_Response   0x37
#define HAL_Audio_Node_Data_Notify       0x38
#define HAL_Audio_Tone_Generate_Request  0x00

/**
 * Audio Sub-Block IDs
 */
#define HAL_Audio_SB_Algorithm_Switches                   0x0009
#define HAL_Audio_SB_Customer_Algorithm_Switches          0x007E
#define HAL_Audio_SB_Reserved1                            0x0008
#define HAL_Audio_SB_Reserved2                            0x0001
#define HAL_Audio_SB_Microphone_Settings                  0x0000
#define HAL_Audio_SB_Primary_Microphone_Settings          0x0000
#define HAL_Audio_SB_Secondary_Microphone_Settings        0x0079
#define HAL_Audio_SB_Tertiary_Microphone_Settings         0x0023
#define HAL_Audio_SB_Mixer_Gains_SW_Settings              0x0005
#define HAL_Audio_SB_Mixer_Gains_Voice_Recording_Settings 0x0007
#define HAL_Audio_SB_Mixer_Gains_Music_Recording_Settings 0x0023
#define HAL_Audio_SB_Mixer_Gains_Tones_Recording_Settings 0x0006
#define HAL_Audio_SB_Mixer_Gains_Smartwatch_Settings      0x0005
#define HAL_Audio_SB_Mute_Settings                        0x0004
#define HAL_Audio_SB_SW_Settings                          0x0002
#define HAL_Audio_SB_Primary_Spkr_Settings                0x0002
#define HAL_Audio_SB_Secondary_Spkr_Settings              0x0003
#define HAL_Audio_SB_Hearable_Settings                    0x0003
#define HAL_Audio_SB_Roundtrip_Latency_Settings           0x003B
#define HAL_Audio_SB_MCPS                                 0x0032
#define HAL_Audio_SB_Logs                                 0x0033
#define HAL_Audio_SB_Sniffer_Enable                       0x0034
#define HAL_Audio_SB_Sniffer_Activate                     0x0035
#define HAL_Audio_SB_Bypass                               0x0036
#define HAL_Audio_SB_Prompt                               0x0037
#define HAL_Audio_SB_UL_AGC_Flag                          0x0009
#define HAL_Audio_SB_UL_AGC_Attack_Time_ms                0x0010
#define HAL_Audio_SB_UL_AGC_Decay_Time_ms                 0x0011
#define HAL_Audio_SB_UL_AGC_Target                        0x0012
#define HAL_Audio_SB_UL_DRC_Attack_Time_ms                0x0013
#define HAL_Audio_SB_UL_DRC_Decay_Time_ms                 0x0014
#define HAL_Audio_SB_UL_DRC_Gain                          0x0015
#define HAL_Audio_SB_UL_DRC_Knee_Threshold                0x0016
#define HAL_Audio_SB_UL_DRC_Noise_Gate                    0x0017
#define HAL_Audio_SB_UL_DRC_Slope                         0x0018
#define HAL_Audio_SB_UL_PEQ_Center_Freq                   0x0019
#define HAL_Audio_SB_UL_PEQ_Qfactor                       0x001a
#define HAL_Audio_SB_UL_PEQ_Gain                          0x001b
#define HAL_Audio_SB_UL_MBDRC_Bound_High                  0x001c
#define HAL_Audio_SB_UL_MBDRC_Compress_Threshold          0x001d
#define HAL_Audio_SB_UL_MBDRC_Compress_Slope              0x001e
#define HAL_Audio_SB_UL_NS_LEVEL                          0x001f
#define HAL_Audio_SB_UL_AEC                               0x0020

#define AUDIO_SB_UL_AEC_CONFIG   0x0000
#define AUDIO_SB_UL_AEC_SNIFFER  0x0001
#define AUDIO_SB_UL_AEC_MCPS     0x0002
#define AUDIO_SB_UL_AEC_BYPASS   0x0003
#define AUDIO_SB_UL_AGC_CONFIG   0x0008
#define AUDIO_SB_UL_AGC_SNIFFER  0x0009
#define AUDIO_SB_UL_AGC_MCPS     0x000a
#define AUDIO_SB_UL_AGC_BYPASS   0x000b
#define AUDIO_SB_UL_NS_CONFIG    0x0010
#define AUDIO_SB_UL_NS_SNIFFER   0x0011
#define AUDIO_SB_UL_NS_MCPS      0x0012
#define AUDIO_SB_UL_NS_BYPASS    0x0013
#define AUDIO_SB_UL_PEQ_CONFIG   0x0018
#define AUDIO_SB_UL_PEQ_SNIFFER  0x0019
#define AUDIO_SB_UL_PEQ_MCPS     0x001a
#define AUDIO_SB_UL_PEQ_BYPASS   0x001b
#define AUDIO_SB_UL_DRC_CONFIG   0x0020
#define AUDIO_SB_UL_DRC_SNIFFER  0x0021
#define AUDIO_SB_UL_DRC_MCPS     0x0022
#define AUDIO_SB_UL_DRC_BYPASS   0x0023
#define AUDIO_SB_UL_NNSE_CONFIG  0x0028
#define AUDIO_SB_UL_NNSE_SNIFFER 0x0029
#define AUDIO_SB_UL_NNSE_MCPS    0x002a
#define AUDIO_SB_UL_NNSE_BYPASS  0x002b
#define AUDIO_SB_UL_WNR_CONFIG   0x0030
#define AUDIO_SB_UL_WNR_SNIFFER  0x0031
#define AUDIO_SB_UL_WNR_MCPS     0x0032
#define AUDIO_SB_UL_WNR_BYPASS   0x0033

#define AUDIO_SB_DL_PEQ_CONFIG         0x0080
#define AUDIO_SB_DL_PEQ_SNIFFER        0x0081
#define AUDIO_SB_DL_PEQ_MCPS           0x0082
#define AUDIO_SB_DL_PEQ_BYPASS         0x0083
#define AUDIO_SB_DL_DRC_CONFIG         0x0088
#define AUDIO_SB_DL_DRC_SNIFFER        0x0089
#define AUDIO_SB_DL_DRC_MCPS           0x008a
#define AUDIO_SB_DL_DRC_BYPASS         0x008b
#define AUDIO_SB_DL_MBDRC_CONFIG       0x0090
#define AUDIO_SB_DL_MBDRC_SNIFFER      0x0091
#define AUDIO_SB_DL_MBDRC_MCPS         0x0092
#define AUDIO_SB_DL_MBDRC_BYPASS       0x0093
#define AUDIO_SB_DL_GAIN_CONFIG        0x0098
#define AUDIO_SB_DL_GAIN_SNIFFER       0x0099
#define AUDIO_SB_DL_GAIN_MCPS          0x009a
#define AUDIO_SB_DL_GAIN_BYPASS        0x009b
#define AUDIO_SB_DL_MIXER_CONFIG       0x00a0
#define AUDIO_SB_DL_MIXER_SNIFFER      0x00a1
#define AUDIO_SB_DL_MIXER_MCPS         0x00a2
#define AUDIO_SB_DL_IIR_FILTER_CONFIG  0x00a8
#define AUDIO_SB_DL_IIR_FILTER_SNIFFER 0x00a9
#define AUDIO_SB_DL_IIR_FILTER_MCPS    0x00aa
#define AUDIO_SB_DL_IIR_FILTER_BYPASS  0x00ab

#define AUDIO_SB_PROMPT 0x00e0

#define AUDIO_SB_ALL_CONFIG 0x00f0
#define AUDIO_SB_ALL_MCPS   0x00f1

/**
 * @brief Endpoint IDs
 */
#define HAL_Audio_Aud_Endpoints_NID        0x00 /**< Unknown audio Endpoints */
#define HAL_Audio_Aud_Endpoint_Hearable    0x01 /**< Hearable mode */
#define HAL_Audio_Aud_Endpoint_Smartwatch  0x0A /**< Smartwatch mode */
#define HAL_Audio_Aud_Endpoint_SW_Ringtone 0x0F /**< Ringtone */
#define HAL_AUDIO_SB_RESERVED1             0x10
#define HAL_AUDIO_SB_RESERVED2             0x11
#define HAL_AUDIO_SB_RESERVED3             0x12
#define HAL_AUDIO_SB_RESERVED4             0x19
#define HAL_AUDIO_SB_RESERVED5             0x1B
#define HAL_AUDIO_SB_RESERVED6             0x21
#define HAL_AUDIO_SB_RESERVED7             0x24
#define HAL_AUDIO_SB_RESERVED8             0x26
#define HAL_AUDIO_SB_RESERVED9             0x29
#define HAL_AUDIO_SB_RESERVED10            0x2C
#define HAL_AUDIO_SB_RESERVED11            0x2D
#define HAL_AUDIO_SB_RESERVED12            0x2E

/**
 * @brief Algorithm mode 1 mask
 */
/** Acoustic Echo Canceller */
#define HAL_Audio_Algorithm_EC 0x0001

/** Downlink Dynamic Range Control */
#define HAL_Audio_Algorithm_DLDRC 0x0040

/** Uplink Dynamic Range Control */
#define HAL_Audio_Algorithm_ULDRC 0x0080

/** VAD Algorithm during VoS */
#define HAL_Audio_Algorithm_VAD 0x0100

/** Microphone Adaptive Gain Control */
#define HAL_Audio_Algorithm_AGC 0x0200

/** VoS Algorithm */
#define HAL_Audio_Algorithm_VoS 0x1000

/** Microphone Noise Suppression */
#define HAL_Audio_Algorithm_NS 0x2000

/** Non Linear Processing */
#define HAL_Audio_Algorithm_NLP 0x8000

/**
 * @brief Algorithm mode 2
 */
/** Speaker Equalizer */
#define HAL_Audio_Algorithm_SEQ 0x0001

/** Microphone Equalizer */
#define HAL_Audio_Algorithm_MEQ 0x0002

/**
 * @brief Algorithm Mode 3
 *
 * @note The VAD specified here is the voice activity detection algorithm in the
 * uplink audio enhancement chain, this will be primarily used in VoS mode.
 */
// #define HAL_Audio_Algorithm_VAD 0x1000 /**< Voice Activity Detection - VAD */

/**
 * @brief Algorithm mode 4
 *
 * @note The VoS® specified here is the Voice-on-Spot® AI Algorithms in the
 * uplink. This will be primarily used in VoS mode. This shall activate LP mode
 * automatically.
 */
// #define HAL_Audio_Algorithm_VoS 0x0004 /**< Key Word Spotting- VoS */

/**
 * @brief Algorithm mode 5
 *
 * @note This mode shall be activated when VoS is enabled, and device is in IDLE
 * mode. This shall call special clocking mode and then configure device for
 * 8kHz and enable blocks needs for supporting ultra-low power mode.
 */
#define HAL_Audio_Algorithm_ULP 0x0008 /**< Ultra-low power mode */

/**
 * @brief Playback selection
 */
/** This is to select external speaker */
#define HAL_Audio_Spkr_Speakerphone 0x0000

/** Wearable */
#define HAL_Audio_Spkr_Wearable 0x0001

/** SoC (for stereo or mono) */
#define HAL_Audio_Spkr_Wearable_mode 0x0002

/** Wearable left channel (for mono) */
#define HAL_Audio_Spkr_Wearable_L 0x0003

/** Wearable right channel (for mono) */
#define HAL_Audio_Spkr_Wearable_R 0x0004

/** Hearable SoC (for stereo or mono) */
#define HAL_Audio_spkr_Hearable 0x0005

/** Hearable speaker, left channel (for mono) */
#define HAL_Audio_Spkr_Hearable_L 0x0006

/** Hearable speaker right channel (for mono) */
#define HAL_Audio_Spkr_Hearable_R 0x0007

/** I2S Wearable (for stereo or mono) */
#define HAL_Audio_Spkr_I2S 0x000E

/** I2S Wearable left channel (for mono) */
#define HAL_Audio_Spkr_I2S_L 0x000F

/** I2S Wearable right channel (for mono) */
#define HAL_Audio_Spkr_I2S_R 0x0010

/**
 * @brief Microphone selection
 */
#define HAL_Audio_MIC_NULL         0x0000 /**< None. Disable mics and power is off */
#define HAL_Audio_Mic_Primary      0x0001 /**< AMIC1 selected */
#define HAL_Audio_Mic_Secondary    0x0002 /**< AMIC2 selected */
#define HAL_Audio_Mic_Tertiary     0x0003 /**< AMIC3 selected */
#define HAL_Audio_MIC_Digital1     0x0004 /**< DMIC1 selected */
#define HAL_Audio_MIC_Digital2     0x0005 /**< DMIC2 selected */
#define HAL_Audio_MIC_Digital3     0x0006 /**< DMIC3 Selected */
#define HAL_Audio_MIC_I2S          0x0007 /**< I2S microphone */
#define HAL_Audio_MIC_Line_in_0dBm 0x0008 /**< Line In, 0dB */

/**
 * @brief Audio path muting and powering options
 */
#define HAL_Audio_Power_Primary       0x0001 /**< Primary mic Power */
#define HAL_Audio_Power_Secondary     0x0002 /**< Secondary Ear Power */
#define HAL_Audio_Power_Tertiary      0x0004 /**< Tertiary Microphone Power */
#define HAL_Audio_Mute_Analog_Primary 0x0008 /**< Primary Analog mic Mute */
#define HAL_Audio_Mute_Analog_Secondary                                        \
    0x0010                                    /**< Secondary Analog mic Mute   \
                                               */
#define HAL_Audio_Mute_Analog_Tertiary 0x0020 /**< Tertiary Analog mic Mute */
#define HAL_Audio_Mute_Digital_Primary 0x0040 /**< Primary Digital mic Mute */
#define HAL_Audio_Mute_Digital_Secondary                                       \
    0x0080 /**< Secondary mic Digital Mute */
#define HAL_Audio_Mute_Digital_Tertiary                                        \
    0x0100 /**< Tertiary Digital mic Mute                                      \
            */
#define HAL_Audio_Mute_Digital_UL                                              \
    0x0200 /**< Voice Call Uplink Digital Mute                                 \
            */
#define HAL_Audio_Mute_Digital_DL                                              \
    0x0400 /**< Voice Call Downlink Digital Mute */
#define HAL_Audio_Mute_HW_Voice_Recording 0x0800 /**< Voice Recording */
#define HAL_Audio_Mute_HQ_Music_Recording                                      \
    0x1000 /**< Hi-Quality Audio Recording */

/**
 * @brief Bits per audio sample
 */
#define HAL_Audio_16_Bits_Sample 0x0000 /**< 16-bit MSB align */
#define HAL_Audio_20_Bits_Sample 0x0001 /**< 20-bit MSB align */
#define HAL_Audio_24_Bits_Sample 0x0002 /**< 24-bit MSB align */
#define HAL_Audio_32_Bits_Sample 0x0003 /**< 32-bit MSB align */

/**
 * @brief General response values
 */
#define HAL_Audio_Response_OK   0x0000
#define HAL_Audio_Response_FAIL 0x0001

/**
 * @brief Audio sample rates
 */
#define HAL_Audio_Sample_Rate_8kHZ    0x0001 /**< 8 kHz */
#define HAL_Audio_Sample_Rate_16kHZ   0x0002 /**< 16 kHz */
#define HAL_Audio_Sample_Rate_24kHZ   0x0004 /**< 24 kHz */
#define HAL_Audio_Sample_Rate_32kHZ   0x0080 /**< 32 kHz */
#define HAL_AUDIO_SAMPLE_RATE_44_1kHZ 0x0100 /**< 44.1 kHz */
#define HAL_AUDIO_SAMPLE_RATE_48kHZ   0x0200 /**< 48 kHz */
#define HAL_AUDIO_SAMPLE_RATE_96kHZ   0x0400 /**< 96 kHz (reserved) */
#define HAL_AUDIO_SAMPLE_RATE_192kHZ  0x1000 /**< 192 kHz (reserved) */

/**
 * @brief Stereo modes
 */
#define HAL_Audio_Mono               0x0000
#define HAL_Audio_Stereo             0x0001
#define HAL_Audio_Stereo_Interleaved 0x0002 /**< (reserved) */

/**
 * @brief Switch
 */
#define HAL_Audio_Switch_OFF 0x0000 /**< Switch off */
#define HAL_Audio_Switch_ON  0x0001 /**< Switch on */

/**
 * @brief MACIF pipe definitions
 */
/** Music playback pipe from audio Framework to HAL. */
#define HAL_Audio_Music_Playback 0x0000

/** Music recording pipe from HAL to audio Framework. */
#define HAL_Audio_Music_Record 0x0001

/** Voice playback pipe from audio Framework to HAL. */
#define HAL_Audio_Voice_Playback 0x0002

/** Voice recording pipe from audio Framework to HAL. */
#define HAL_Audio_Voice_Record 0x0003

/**
 * @brief MACIF pipe activation
 */
/** Pipe is ended, but the remaining data in shared memory buffer is played
 * first. */
#define HAL_Audio_Pipe_Not_Active 0x0000

/** Pipe is ended immediately and the possible data in the shared memory buffer
 * is not played. */
#define HAL_Audio_Pipe_Not_Active_Flush 0x0001

/** Pipe is started. */
#define HAL_Audio_Pipe_Active 0x0002

/**
 * @brief Tone Envelop
 */
#define HAL_Audio_Parms_E_7 0x07 /**< User defined slopes. */

/**
 * @brief Tone Toggle
 */
/** Tone generator enabled. */
#define HAL_Audio_Parms_Tone_Enabled 0x01

/** Tone generator disabled. Tone generation does not always stop immediately
 * after setting this bit to disabled state, but it depends on decay and release
 * time. */
#define HAL_Audio_Parms_Tone_Disabled 0x00

/**
 * @brief Tone TX Output
 */
/** Tone generator output fed to TX speech path. */
#define HAL_Audio_Parms_UL_ON 0x01

/** Tone generator output not fed to TX speech path. */
#define HAL_Audio_Parms_UL_OFF 0x00

/**
 * @brief Tone Waveforms
 */
#define HAL_Audio_Parms_Wave_Sine         0x00 /**< Sine wave output. */
#define HAL_Audio_Parms_Wave_Triangular   0x01 /**< Triangular wave output. */
#define HAL_Audio_Parms_Wave_Square       0x02 /**< Square wave output. */
#define HAL_Audio_Parms_Wave_Pseudo_Noise 0x03 /**< Pseudo noise output. */

#endif /* _ACORE_MSG_DEF_H_ */
