//*****************************************************************************
//
// Copyright (c) 2022, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

#ifndef __AUST_SEM_H__
#define __AUST_SEM_H__

#include <FreeRTOS.h>
#include "semphr.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /// @brief Metal semaphore structure.
    typedef struct
    {
        SemaphoreHandle_t sem_; ///< Semaphore handle.
    } metal_sem_t;

    /* Initialize semaphore object SEM to VALUE.  If PSHARED then share it
       with other processes.  */
    extern int metal_sem_init(metal_sem_t *__sem, int __pshared,
                              unsigned int __value);

    /* Free resources associated with semaphore object SEM.  */
    extern int metal_sem_destroy(metal_sem_t *__sem);

    /* Open a named semaphore NAME with open flags OFLAG.  */
    extern metal_sem_t *metal_sem_open(const char *__name, int __oflag, ...);

    /* Close descriptor for named semaphore SEM.  */
    extern int metal_sem_close(metal_sem_t *__sem);

    /* Remove named semaphore NAME.  */
    extern int metal_sem_unlink(const char *__name);

    /* Wait for SEM being posted.

       This function is a cancellation point and therefore not marked with
       __THROW.  */
    extern int metal_sem_wait(metal_sem_t *__sem);

#ifdef __USE_XOPEN2K
    /* Similar to `sem_wait' but wait only until ABSTIME.

       This function is a cancellation point and therefore not marked with
       __THROW.  */
    extern int metal_sem_timedwait(metal_sem_t *__restrict __sem,
                                   const struct timespec *__restrict __abstime);
#endif

    /* Test whether SEM is posted.  */
    extern int metal_sem_trywait(metal_sem_t *__sem);

    /* Post SEM.  */
    extern int metal_sem_post(metal_sem_t *__sem);

    /* Get current value of SEM and store it in *SVAL.  */
    extern int metal_sem_getvalue(metal_sem_t *__restrict __sem,
                                  int *__restrict __sval);

#ifdef __cplusplus
}
#endif

#endif /* __AUST_SEM_H__ */
