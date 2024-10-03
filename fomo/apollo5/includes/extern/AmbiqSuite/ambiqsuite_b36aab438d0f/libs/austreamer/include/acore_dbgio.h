/**
 ****************************************************************************************
 *
 * @file acore_dbgio.h
 *
 * @brief Declaration Audio core debug io definition.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ACORE_DBGIO_H_
#define _ACORE_DBGIO_H_

#if defined __cplusplus
extern "C"
{
#endif

    /**
     * @brief DBGIO switch.
     */
    typedef enum
    {
        DBGIO_OFF,
        DBGIO_ON
    } dbgio_sw_e;

    /**
     * @brief dbgio_e defines the debug signal index of debug IO lookup
     * table uint32_t dbgio_table[DBGIO_MAX] When adding a new debug signal,
     * update both dbgio_e and dbgio_table[] accordingly.
     */
    typedef enum
    {
        /* aust bus thread*/
        DBGIO_AUST_BUS,
        /* aust queue element thread*/
        DBGIO_AUST_QUEUE,
        /* aust data flow*/
        DBGIO_AUST_DATA_FLOW,
        /* aust Decoder processing */
        DBGIO_AUST_DECODE,
        /* aust Encoder processing */
        DBGIO_AUST_ENCODE,
        /* means aust Resample processing */
        DBGIO_AUST_RESAMPLE,
        /* aust Convert processing */
        DBGIO_AUST_CONVERT,
        /* aust Gain processing */
        DBGIO_AUST_GAIN,
        /* aust AEC processing */
        DBGIO_AUST_AEC,
        /* aust MIX processing */
        DBGIO_AUST_MIX,
        /* aust sniffer processing */
        DBGIO_AUST_SNIFFER,
        /* aust scheduler */
        DBGIO_AUST_SCHEDULER,
        /* acore message agent processing */
        DBGIO_ACORE_MSG_AGENT,
        /* acore src processing */
        DBGIO_ACORE_SRC,
        /* acore sink processing */
        DBGIO_ACORE_SINK,
        /* adev I2S-0 interrupt */
        DBGIO_ADEV_I2S0_INT,
        /* adev I2S-1 interrupt */
        DBGIO_ADEV_I2S1_INT,
        /* adev PDM-0 interrupt */
        DBGIO_ADEV_PDM0_INT,
        /* adev I2S-0 read */
        DBGIO_ADEV_I2S0_READ,
        /* adev I2S-0 write */
        DBGIO_ADEV_I2S0_WRITE,
        /* adev I2S-1 read */
        DBGIO_ADEV_I2S1_READ,
        /* adev I2S-1 write */
        DBGIO_ADEV_I2S1_WRITE,
        /* adev PDM read */
        DBGIO_ADEV_PDM_READ,

        DBGIO_MAX
    } dbgio_e;

    /**
     * @brief Enable or disable timing debug io toggle
     *
     * @param[in] dbgio_on DBGIO_ON: enable timing debug io toggle,
     * DBGIO_OFF:disable timing debug io toggle
     */
    void acore_dbgio_init(dbgio_sw_e dbgio_on);

    /**
     * @brief Set the dbgio to high.
     *
     * @param dbgio[in] Debug IO enumerated in dbgio_e.
     */
    void acore_dbgio_set(dbgio_e dbgio);

    /**
     * @brief Set the dbgio to low.
     *
     * @param dbgio[in] Debug IO enumerated in dbgio_e.
     */
    void acore_dbgio_clear(dbgio_e dbgio);

#if defined __cplusplus
}
#endif

#endif /* _ACORE_DBGIO_H_ */
