/**
 ****************************************************************************************
 *
 * @file aust_lib_info.h
 *
 * @brief Library information of AuStreamer.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _AUST_LIB_INFO_H_
#define _AUST_LIB_INFO_H_

/**
 * @brief Get AuStreamer lib compiling date.
 *
 * @return Pointer to date string.
 */
const char *aust_get_compiling_date(void);

/**
 * @brief Get AuStreamer lib SHA-1 ID.
 *
 * @return Pointer to SHA-1 ID string.
 */
const char *aust_get_sha1_id(void);

#endif // _AUST_LIB_INFO_H_
