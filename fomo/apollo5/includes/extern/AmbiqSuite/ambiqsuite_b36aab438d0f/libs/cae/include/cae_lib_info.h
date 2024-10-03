/**
 ****************************************************************************************
 *
 * @file cae_lib_info.h
 *
 * @brief Library information of CAE.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _CAE_LIB_INFO_H_
#define _CAE_LIB_INFO_H_

/**
 * @brief Get CEA lib compiling date.
 *
 * @return Pointer to date string.
 */
const char *cae_get_compiling_date(void);

/**
 * @brief Get CEA lib SHA-1 ID.
 *
 * @return Pointer to SHA-1 ID string.
 */
const char *cae_get_sha1_id(void);

#endif // _CAE_LIB_INFO_H_
