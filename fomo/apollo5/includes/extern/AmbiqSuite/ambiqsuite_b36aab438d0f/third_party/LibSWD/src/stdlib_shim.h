#ifndef STDLIB_SHIM_H
#define STDLIB_SHIM_H

#define malloc am_util_swd_malloc
#define calloc am_util_swd_calloc
#define free   am_util_swd_free

#define USLEEP_FUNCTION am_util_swd_usleep

#define libswd_log(...)
#define fflush(...)

extern void *am_util_swd_malloc(size_t size);
extern void *am_util_swd_calloc(size_t num, size_t size);
extern void am_util_swd_free(void *ptr);
extern void am_util_swd_usleep(int microseconds);

#endif /* end of include guard: STDLIB_SHIM_H */
