#ifndef _CRC_CRC32_H
#define _CRC_CRC32_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus  */

uint32_t crc32(void* data, uint32_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus  */
#endif