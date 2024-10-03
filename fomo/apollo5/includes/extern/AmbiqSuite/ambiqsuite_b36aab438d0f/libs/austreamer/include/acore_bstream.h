/**
 ******************************************************************************
 *
 * @file acore_bstream.h
 *
 * @brief Byte stream to integer conversion macros.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_BSTREAM_H_
#define _ACORE_BSTREAM_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * Macros for converting a bit endian byte buffer to integers.
 */
#define BYTES_TO_UINT16(n, p)                                                  \
    {                                                                          \
        n = (((uint16_t)(p)[0] << 8) + (uint16_t)(p)[1]);                      \
    }

#define BYTES_TO_UINT24(n, p)                                                  \
    {                                                                          \
        n = (((uint16_t)(p)[0] << 16) + ((uint16_t)(p)[1] << 8) +              \
             (uint16_t)(p)[2]);                                                \
    }

#define BYTES_TO_UINT32(n, p)                                                  \
    {                                                                          \
        n = (((uint32_t)(p)[0] << 24) + ((uint32_t)(p)[1] << 16) +             \
             ((uint32_t)(p)[2] << 8) + (uint32_t)(p)[3]);                      \
    }

#define BYTES_TO_UINT40(n, p)                                                  \
    {                                                                          \
        n = (((uint64_t)(p)[0] << 32) + ((uint64_t)(p)[1] << 24) +             \
             ((uint64_t)(p)[2] << 16) + ((uint64_t)(p)[3] << 8) +              \
             (uint64_t)(p)[4]);                                                \
    }

#define BYTES_TO_UINT64(n, p)                                                  \
    {                                                                          \
        n = (((uint64_t)(p)[0] << 56) + ((uint64_t)(p)[1] << 48) +             \
             ((uint64_t)(p)[2] << 40) + ((uint64_t)(p)[3] << 32) +             \
             ((uint64_t)(p)[4] << 24) + ((uint64_t)(p)[5] << 16) +             \
             ((uint64_t)(p)[6] << 8) + (uint64_t)(p)[7]);                      \
    }

/*!
 * Macros for converting big endian integers to array of bytes
 */
#define UINT16_TO_BYTES(n) ((uint8_t)((n) >> 8)), ((uint8_t)(n))
#define UINT32_TO_BYTES(n)                                                     \
    ((uint8_t)((n) >> 24)), ((uint8_t)((n) >> 16)), ((uint8_t)((n) >> 8)),     \
        ((uint8_t)(n))

/*!
 * Macros for converting big endian integers to single bytes
 */
#define UINT16_TO_BYTE0(n) ((uint8_t)((n) >> 8))
#define UINT16_TO_BYTE1(n) ((uint8_t)(n))

#define UINT32_TO_BYTE0(n) ((uint8_t)((n) >> 24))
#define UINT32_TO_BYTE1(n) ((uint8_t)((n) >> 16))
#define UINT32_TO_BYTE2(n) ((uint8_t)((n) >> 8))
#define UINT32_TO_BYTE3(n) ((uint8_t)(n))

/*!
 * Macros for converting a big endian byte stream to integers, with
 * increment.
 */
#define BSTREAM_TO_INT8(n, p)                                                  \
    {                                                                          \
        n = (int8_t)(*(p)++);                                                  \
    }
#define BSTREAM_TO_UINT8(n, p)                                                 \
    {                                                                          \
        n = (uint8_t)(*(p)++);                                                 \
    }
#define BSTREAM_TO_UINT16(n, p)                                                \
    {                                                                          \
        BYTES_TO_UINT16(n, p);                                                 \
        p += 2;                                                                \
    }
#define BSTREAM_TO_UINT24(n, p)                                                \
    {                                                                          \
        BYTES_TO_UINT24(n, p);                                                 \
        p += 3;                                                                \
    }
#define BSTREAM_TO_UINT32(n, p)                                                \
    {                                                                          \
        BYTES_TO_UINT32(n, p);                                                 \
        p += 4;                                                                \
    }
#define BSTREAM_TO_UINT40(n, p)                                                \
    {                                                                          \
        BYTES_TO_UINT40(n, p);                                                 \
        p += 5;                                                                \
    }
#define BSTREAM_TO_UINT64(n, p)                                                \
    {                                                                          \
        BYTES_TO_UINT64(n, p);                                                 \
        p += 8;                                                                \
    }

/*!
 * Macros for converting integers to a big endian byte stream, with increment.
 */
#define UINT8_TO_BSTREAM(p, n)                                                 \
    {                                                                          \
        *(p)++ = (uint8_t)(n);                                                 \
    }
#define UINT16_TO_BSTREAM(p, n)                                                \
    {                                                                          \
        *(p)++ = (uint8_t)((n) >> 8);                                          \
        *(p)++ = (uint8_t)(n);                                                 \
    }
#define UINT24_TO_BSTREAM(p, n)                                                \
    {                                                                          \
        *(p)++ = (uint8_t)((n) >> 16);                                         \
        *(p)++ = (uint8_t)((n) >> 8);                                          \
        *(p)++ = (uint8_t)(n);                                                 \
    }
#define UINT32_TO_BSTREAM(p, n)                                                \
    {                                                                          \
        *(p)++ = (uint8_t)((n) >> 24);                                         \
        *(p)++ = (uint8_t)((n) >> 16);                                         \
        *(p)++ = (uint8_t)((n) >> 8);                                          \
        *(p)++ = (uint8_t)(n);                                                 \
    }
#define UINT40_TO_BSTREAM(p, n)                                                \
    {                                                                          \
        *(p)++ = (uint8_t)((n) >> 32);                                         \
        *(p)++ = (uint8_t)((n) >> 24);                                         \
        *(p)++ = (uint8_t)((n) >> 16);                                         \
        *(p)++ = (uint8_t)((n) >> 8);                                          \
        *(p)++ = (uint8_t)(n);                                                 \
    }
#define UINT64_TO_BSTREAM(p, n)                                                \
    {                                                                          \
        *(p)++ = (uint8_t)((n) >> 56);                                         \
        *(p)++ = (uint8_t)((n) >> 48);                                         \
        *(p)++ = (uint8_t)((n) >> 40);                                         \
        *(p)++ = (uint8_t)((n) >> 32);                                         \
        *(p)++ = (uint8_t)((n) >> 24);                                         \
        *(p)++ = (uint8_t)((n) >> 16);                                         \
        *(p)++ = (uint8_t)((n) >> 8);                                          \
        *(p)++ = (uint8_t)(n);                                                 \
    }

/*!
 * Macros for converting integers to a big endian byte stream, without
 * increment.
 */
#define UINT16_TO_BUF(p, n)                                                    \
    {                                                                          \
        (p)[0] = (uint8_t)((n) >> 8);                                          \
        (p)[1] = (uint8_t)(n);                                                 \
    }
#define UINT24_TO_BUF(p, n)                                                    \
    {                                                                          \
        (p)[0] = (uint8_t)((n) >> 16);                                         \
        (p)[1] = (uint8_t)((n) >> 8);                                          \
        (p)[2] = (uint8_t)(n);                                                 \
    }
#define UINT32_TO_BUF(p, n)                                                    \
    {                                                                          \
        (p)[0] = (uint8_t)((n) >> 24);                                         \
        (p)[1] = (uint8_t)((n) >> 16);                                         \
        (p)[2] = (uint8_t)((n) >> 8);                                          \
        (p)[3] = (uint8_t)(n);                                                 \
    }
#define UINT40_TO_BUF(p, n)                                                    \
    {                                                                          \
        (p)[0] = (uint8_t)((n) >> 32);                                         \
        (p)[1] = (uint8_t)((n) >> 24);                                         \
        (p)[2] = (uint8_t)((n) >> 16);                                         \
        (p)[3] = (uint8_t)((n) >> 8);                                          \
        (p)[4] = (uint8_t)(n);                                                 \
    }

/*!
 * Macros for comparing a big endian byte buffer to integers.
 */
#define BYTES_UINT16_CMP(p, n)                                                 \
    ((p)[1] == UINT16_TO_BYTE1(n) && (p)[0] == UINT16_TO_BYTE0(n))

/*!
 * Macros for IEEE FLOAT type:  exponent = byte 3, mantissa = bytes 2-0
 */
#define FLT_TO_UINT32(m, e) ((m) | ((int32_t)(e) << 24))
#define UINT32_TO_FLT(m, e, n)                                                 \
    {                                                                          \
        m = UINT32_TO_FLT_M(n);                                                \
        e = UINT32_TO_FLT_E(n);                                                \
    }
#define UINT32_TO_FLT_M(n)                                                     \
    ((((n)&0x00FFFFFF) >= 0x00800000) ? ((int32_t)(((n) | 0xFF000000)))        \
                                      : ((int32_t)((n)&0x00FFFFFF)))
#define UINT32_TO_FLT_E(n) ((int8_t)(n >> 24))

/*!
 * Macros for IEEE SFLOAT type:  exponent = bits 15-12, mantissa = bits 11-0
 */
#define SFLT_TO_UINT16(m, e) ((m) | ((int16_t)(e) << 12))
#define UINT16_TO_SFLT(m, e, n)                                                \
    {                                                                          \
        m = UINT16_TO_SFLT_M(n);                                               \
        e = UINT16_TO_SFLT_E(n);                                               \
    }
#define UINT16_TO_SFLT_M(n)                                                    \
    ((((n)&0x0FFF) >= 0x0800) ? ((int16_t)(((n) | 0xF000)))                    \
                              : ((int16_t)((n)&0x0FFF)))
#define UINT16_TO_SFLT_E(n)                                                    \
    (((n >> 12) >= 0x0008) ? ((int8_t)(((n >> 12) | 0xF0)))                    \
                           : ((int8_t)(n >> 12)))

#ifdef __cplusplus
};
#endif

#endif /* _ACORE_BSTREAM_H_ */
