/*********************************************************************
 *
 *   MODULE NAME:
 *       gar_stream.c - Garmin Version of STREAM RAM Benchmark
 *
 *   DESCRIPTION:
 *       This code measures memory transfer rates in MB/s for simple computational
 *       kernels coded in C.
 *
 *       This code was original ported from John D. McCalpin and then heavily
 *       customized for Garmin's use, to further evaluate the impact of cache on
 *       system performance and to increase portability to embedded platforms.
 *
 *       For reference, the License agreement from John D. McCalpin is blow.
 *           -----------------------------------------------------------------------
 *           Copyright 1991-2013: John D. McCalpin
 *           -----------------------------------------------------------------------
 *           License:
 *             1. You are free to use this program and/or to redistribute
 *                this program.
 *             2. You are free to modify this program for your own use,
 *                including commercial use, subject to the publication
 *                restrictions in item 3.
 *             3. You are free to publish results obtained from running this
 *                program, or from works that you derive from this program,
 *                with the following limitations:
 *                3a. In order to be referred to as "STREAM benchmark results",
 *                    published results must be in conformance to the STREAM
 *                    Run Rules, (briefly reviewed below) published at
 *                    http://www.cs.virginia.edu/stream/ref.html
 *                    and incorporated herein by reference.
 *                    As the copyright holder, John McCalpin retains the
 *                    right to determine conformity with the Run Rules.
 *                3b. Results based on modified source code or on runs not in
 *                    accordance with the STREAM Run Rules must be clearly
 *                    labelled whenever they are published.  Examples of
 *                    proper labelling include:
 *                      "tuned STREAM benchmark results"
 *                      "based on a variant of the STREAM benchmark code"
 *                    Other comparable, clear, and reasonable labelling is
 *                    acceptable.
 *                3c. Submission of results to the STREAM benchmark web site
 *                    is encouraged, but not required.
 *             4. Use of this program or creation of derived works based on this
 *                program constitutes acceptance of these licensing restrictions.
 *             5. Absolutely no warranty is expressed or implied.
 *
 *   INSTRUCTIONS:
 *       STREAM requires different amounts of memory to run on different systems,
 *       depending on both the system cache size(s) and the granularity of the
 *       system timer.  Adjust the value of array sizes to meet the following criteria:
 *
 *           - To evaluate the cache, set the size of the arrays to 1/3 the cache size.
 *             This guarantees that each array lives within the cache, thus fully exercising
 *             the cache.
 *
 *           - To thrash the cache, set the size of the arrays to at least 4 times the
 *             size of the cache.
 *
 *           - The size of each array (in the 2 bullets above) should be large enough,
 *             so that each test takes at least 20 clock cycles.  This is to minimize timing
 *             errors, for systems with slow system/tick timers.
 *
 *       Regarding compiler optimizations:
 *
 *           - Per Stream's documentation: compile the code with optimization.  Many
 *             compilers generate unreasonably bad code before the optimizer tightens
 *             things up.  If the results are unreasonably good, on the other hand,
 *             the optimizer might be too smart for me!
 *
 *             For a simple single-core version, try compiling with:
 *               cc -O stream.c -o stream
 *
 *           - For Garmin's use, we selected the following compiler options:
 *               - O2                for faster code,
 *               - align-loops=4     to align all loops on word boundaries, which eliminates
 *                                   extra clock cycles in loops due to aligned fetches
 *               - align-jumps=4     to align branch targets to word boundaries, for same
 *                                   reasons as align-loops.
 *
 *       Regarding the size of the data type selection:
 *
 *           - Per Stream's documentation: run with single-precision variables and
 *             arithmetic, simply define gar_stream_type as a float.
 *
 *           - For Garmin's use, we select a uint32_t.  We believe Stream is geared for
 *             high end servers and PCs.  For embedded platforms, we are less concerned
 *             about throughput of the FPU, and more looking at specific memory and
 *             bus architecture.
 *
 *       Porting Instructions:
 *
 *           - Look at the gar_stream.h examples.  If one already exists, rename it to
 *             gar_stream.h, or modify the include, below, to point to the platform
 *             specific file.  If a gar_stream.h file does not exist for your platform,
 *             use the existing examples to create one for yourself.
 *
 *           - From you platform, declare 3 arrays, sizes specified in comments above.
 *             Then, call gar_stream_main() and pass in the address of the 3 arrays.
 *
 *           - Set the CACHE_LINE_SIZE_BYTES constant in gar_stream.h per your MCU/MPU.
 *
 * Copyright 2018 by Garmin Ltd. or its subsidiaries.
 *********************************************************************/

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/
#include <math.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include "gar_stream.h"
#include "th_lib.h"
#include "th_al.h"

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*  STREAM runs each kernel NTIMES times and reports the best result
 *  for any iteration after the first, therefore the minimum value
 *  for NTIMES is 2.  There are no rules on maximum allowable values
 *  for NTIMES, but values larger than the default are unlikely to
 *  noticeably increase the reported performance.
 */
#define NTIMES (2048)

#define HLINE "------------------------------------------------------------------\n"
#define BYTES_PER_MB (1024 * 1024)
#define USEC_PER_SEC (1.0e6)

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#define GRM_unused(...)

__unused static uint32_t s_unit_test_block[] = {
    0x00000000, 0xAAAA5555, 0x5555AAAA, 0x55AA55AA, 0xAA55AA55, 0xF0F0F0F0, 0x0F0F0F0F, 0xC3C3C3C3,
    0x3C3C3C3C, 0xFF00FF00, 0x00FF00FF, 0xF00FF00F, 0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210,

    0xEFFFFFFF, 0xFFEFFFFF, 0xFFFFEFFF, 0xFFFFFFEF, 0xFEFFFFFF, 0xFFFEFFFF, 0xFFFFFEFF, 0xFFFFFFFE,
    0x10000000, 0x00100000, 0x00001000, 0x00000010, 0x01000000, 0x00010000, 0x00000100, 0x00000001,

    0x7FFFFFFF, 0xFF7FFFFF, 0xFFFF7FFF, 0xFFFFFF7F, 0xF7FFFFFF, 0xFFF7FFFF, 0xFFFFF7FF, 0xFFFFFFF7,
    0x80000000, 0x00800000, 0x00008000, 0x00000080, 0x08000000, 0x00080000, 0x00000800, 0x00000008,

    0x9FFFFFFF, 0xFF9FFFFF, 0xFFFF9FFF, 0xFFFFFF9F, 0xF9FFFFFF, 0xFFF9FFFF, 0xFFFFF9FF, 0xFFFFFFF9,
    0x60000000, 0x00600000, 0x00006000, 0x00000060, 0x06000000, 0x00060000, 0x00000600, 0x00000006,
}; /* s_unit_test_block[] */

#define maxval(x, y) ((x) > (y) ? (x) : (y))
#define print_error(text_)                                        \
  gar_stream_printf("ERROR: LINE: %d - %s\n", __LINE__, (text_)); \
  while (1) {                                                     \
  }
#define HLINE "------------------------------------------------------------------\n"

#define BLOCK_SIZE                              (sizeof(s_unit_test_block))
#define MCU_CACHE_SIZE_KB                       (64U)
#define BENCHMARK_GARSTREAM_CACHE_SIZE          maxval(BLOCK_SIZE, (MCU_CACHE_SIZE_KB * 1024U))
#define BENCHMARK_GARSTREAM_CACHE_EVAL_SIZE     (BENCHMARK_GARSTREAM_CACHE_SIZE / 4U)
#define BENCHMARK_GARSTREAM_CACHE_THRASH_SIZE   (4U * BENCHMARK_GARSTREAM_CACHE_SIZE)

/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/
enum {
  TEST_MEMREAD,
  TEST_MEMREAD_REVERSE,
  TEST_MEMSET,
  TEST_MEMSET_VERTICAL,
  TEST_MEMSET_CLIB_0,
  TEST_MEMSET_CLIB_1,
  TEST_MEMMOVE,
  TEST_MEMMOVE_CLIB,
  TEST_MEMCOPY,
  TEST_MEMCOPY_CLIB,
  TEST_SCALE,
  TEST_ADD,
  TEST_TRIAD,

  TEST_COUNT
};

/*--------------------------------------------------------------------
                           PROJECT INCLUDES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/
static double times[TEST_COUNT][NTIMES];
static double bestrate[TEST_COUNT];
static double avgtime[TEST_COUNT];
static double maxtime[TEST_COUNT];
static double mintime[TEST_COUNT];

/*----------------------------------------------------------
The memory_access_count array contains a value which represents
the number of accesses, to the particular physical memory, per
test.  Tests with a higher number of access will show a higher
MB/sec value, as the number of access will be higher per loop.

The instructions per loop (shown below) were observed using the
compiler options listed in the file header above, and with
gar_stream_type defined as a uint32_t.
----------------------------------------------------------*/
static double memory_access_count[TEST_COUNT];

static __unused char const *const label[TEST_COUNT] = {
    "Mem Read:        ", "Mem Read Rev:    ", "Mem Set:         ", "Mem Set Vertical:",
    "Mem Set Clib 0:  ", "Mem Set Clib 1:  ", "Mem Move:        ", "Mem Move Clib:   ",
    "Mem Copy:        ", "Mem Copy Clib:   ", "Scale:           ", "Add:             ",
    "Triad:           ",
};

/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_memcopy - Sequential Memory Copy Test
 *
 *   DESCRIPTION:
 *       (~4 instructions and 2 accesses per loop)
 *
 *       This test shows the effects and differences between a
 *       write-through and write-back cache.  It should show a similar
 *       execution time as the MEMSET or MEMMOVE test, but since it has
 *       2 memory accesses per loop, it could show a higher MB/sec value.
 *
 *********************************************************************/
static double test_memcopy(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                           gar_stream_type *c) {
  gar_stream_type *ptr_end = &a[array_size];
  gar_stream_type *ptr_a = a;
  gar_stream_type *ptr_c = c;
  GRM_unused(b);

  memory_access_count[TEST_MEMCOPY] = 2;

  if (NULL == a) return (0);
  if (NULL == c) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  while (ptr_a < ptr_end) {
    (*ptr_c++) = (*ptr_a++);
  }
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_memcopy() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_memcopy_clib - Sequential Memory Copy Test
 *
 *   DESCRIPTION:
 *       Like test_memcopy, using standard C library memcpy
 *
 *********************************************************************/
static double test_memcopy_clib(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                                gar_stream_type *c) {
  gar_stream_type *ptr_a = a;
  gar_stream_type *ptr_c = c;
  GRM_unused(b);

  memory_access_count[TEST_MEMCOPY_CLIB] = 2;

  if (NULL == a) return (0);
  if (NULL == c) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  memcpy(ptr_c, ptr_a, array_size * sizeof(gar_stream_type));
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_memcopy_clib() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_memmove - Sequential Memory Move Test
 *
 *   DESCRIPTION:
 *       ~4 instructions and 2 accesses per loop
 *
 *       This test shows the effects and differences between a
 *       write-through and write-back cache.  It should show a similar
 *       execution time as the MEMSET test, but since it has 2 memory
 *       accesses per loop, it will show a higher MB/sec value.
 *
 *********************************************************************/
static double test_memmove(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                           gar_stream_type *c) {
  gar_stream_type *ptr_end = &c[array_size];
  gar_stream_type *ptr = &c[1];
  GRM_unused(a);
  GRM_unused(b);

  memory_access_count[TEST_MEMMOVE] = 2;

  if (NULL == c) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  while (ptr < ptr_end) {
    *ptr = *(ptr - 1);
    ptr++;
  }
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_memmove() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_memmove_clib - Sequential Memory Move Test
 *
 *   DESCRIPTION:
 *       Like test_memmove using Standard C library memmove
 *
 *********************************************************************/
static double test_memmove_clib(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                                gar_stream_type *c) {
  GRM_unused(a);
  GRM_unused(b);

  memory_access_count[TEST_MEMMOVE_CLIB] = 2;

  if (NULL == c) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  memmove(&c[1], &c[0], (array_size - 1) * sizeof(gar_stream_type));
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_memmove_clib() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_memread - Sequential Memory Read Test
 *
 *   DESCRIPTION:
 *       ~4 instructions and 1 access per loop
 *
 *       This test tries to emulate sequential read accesses through memory
 *       with a single load instruction.  To emulate random access, where
 *       the benefits of a cache would be negated, just disable the cache.
 *       With the cache enabled, this test should exercise the cache to
 *       its fullest benefit.
 *
 *********************************************************************/
static double test_memread(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                           gar_stream_type *c) {
  volatile __unused gar_stream_type read;
  gar_stream_type *ptr_end = &a[array_size];
  gar_stream_type *ptr = a;
  GRM_unused(b);
  GRM_unused(c);

  memory_access_count[TEST_MEMREAD] = 1;

  if (NULL == a) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  while (ptr < ptr_end) {
    read = (*ptr++);
  }
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_memread() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_memread_reverse - Sequential Reverse Memory Read Test
 *
 *   DESCRIPTION:
 *       ~4 instructions and 1 access per loop
 *
 *       This test is the same as the TEST_MEMREAD, but reads sequentially
 *       backwards through memory.  With a cache enabled, this should negate
 *       its effects and yield results similar to when the cache is
 *       disabled.
 *
 *********************************************************************/
static double test_memread_reverse(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                                   gar_stream_type *c) {
  volatile __unused gar_stream_type read;
  gar_stream_type *ptr_end = a;
  gar_stream_type *ptr = &a[array_size - 1];
  GRM_unused(b);
  GRM_unused(c);

  memory_access_count[TEST_MEMREAD_REVERSE] = 1;

  if (NULL == a) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  while (ptr >= ptr_end) {
    read = (*ptr--);
  }
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_memread_reverse() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_memset - Sequential Memory Set Test
 *
 *   DESCRIPTION:
 *       ~3 instructions and 1 access per loop
 *
 *       Like the TEST_MEMREAD, this emulates sequential write access
 *       to memory, not necessarily a memset.  This test should show the
 *       effects and differences between a write-through and write-back
 *       cache.
 *
 *********************************************************************/
static double test_memset(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                          gar_stream_type *c) {
  gar_stream_type *ptr_end = &c[array_size];
  gar_stream_type *ptr = c;
  GRM_unused(a);
  GRM_unused(b);

  memory_access_count[TEST_MEMSET] = 1;

  if (NULL == c) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  while (ptr < ptr_end) {
    (*ptr++) = 1;
  }
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_memset() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_memset_clib_0 - C library memset test
 *
 *   DESCRIPTION:
 *       Like test_memset using standard C library memset. Fill with
 *       zero because some architectures have optimized zero fill.
 *
 *********************************************************************/
static double test_memset_clib_0(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                                 gar_stream_type *c) {
  gar_stream_type *ptr = c;
  GRM_unused(a);
  GRM_unused(b);

  memory_access_count[TEST_MEMSET_CLIB_0] = 1;

  if (NULL == c) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  memset(ptr, 0, array_size * sizeof(gar_stream_type));
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_memset_clib_0() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_memset_clib_1 - C library memset test
 *
 *   DESCRIPTION:
 *       Like test_memset using standard C library memset. Fill
 *       with non-zero value.
 *
 *********************************************************************/
static double test_memset_clib_1(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                                 gar_stream_type *c) {
  gar_stream_type *ptr = c;
  GRM_unused(a);
  GRM_unused(b);

  memory_access_count[TEST_MEMSET_CLIB_1] = 1;

  if (NULL == c) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  memset(ptr, 1, array_size * sizeof(gar_stream_type));
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_memset_clib_1() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_memset_vertical - Sequential Memory Set Test
 *
 *   DESCRIPTION:
 *       ~3 instructions and 1 access per loop
 *
 *       This test emulates simplistic graphics operations, where writes
 *       into memory jump around, causing only single store operations per
 *       cache line.  In this test, only a single write per cache line is
 *       allowed.  This test should show the effects or how well a cache
 *       handles dirty bytes in the cache during eviction.
 *
 *       The size of the c buffer should be 4 times as large as the cache,
 *       to ensure data is frequently evicted.
 *
 *********************************************************************/
static double test_memset_vertical(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                                   gar_stream_type *c) {
  size_t word_per_cache_line = CACHE_LINE_SIZE_BYTES / sizeof(gar_stream_type);
  size_t cache_line_offset;
  gar_stream_type *ptr_end = &c[array_size];
  gar_stream_type *ptr;
  GRM_unused(a);
  GRM_unused(b);

  memory_access_count[TEST_MEMSET_VERTICAL] = 1;

  if (NULL == c) return (0);
  if (0 == word_per_cache_line) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  for (cache_line_offset = 0; cache_line_offset < word_per_cache_line; cache_line_offset++) {
    ptr = &c[cache_line_offset];

    while (ptr < ptr_end) {
      *ptr = cache_line_offset;
      ptr += word_per_cache_line;
    }
  }
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_memset_vertical() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_add - Add Test
 *
 *   DESCRIPTION:
 *       ~6 instructions and 3 accesses per loop
 *
 *       This test is more of a system test; where the difference between
 *       reading/writing the memory are compared, with the act of doing some
 *       simple operations with the memory.  If caches and store-buffers are
 *       working well, TEST_SCALE should keep up with TEST_MEMCOPY.  These
 *       tests are designed to defeat systems with a built-in memcpy
 *       instruction that choke while actually doing math.
 *
 *********************************************************************/
static double test_add(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                       gar_stream_type *c) {
  gar_stream_type *ptr_end = &a[array_size];
  gar_stream_type *ptr_a = a;
  gar_stream_type *ptr_b = b;
  gar_stream_type *ptr_c = c;

  memory_access_count[TEST_ADD] = 3;

  if (NULL == a) return (0);
  if (NULL == b) return (0);
  if (NULL == c) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  while (ptr_a < ptr_end) {
    (*ptr_c++) = (*ptr_a++) + (*ptr_b++);
  }
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_add() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_scale - Scalar Test
 *
 *   DESCRIPTION:
 *       ~5 instructions and 2 accesses per loop
 *
 *       This test is more of a system test; where the difference between
 *       reading/writing the memory are compared, with the act of doing some
 *       simple operations with the memory.  If caches and store-buffers are
 *       working well, TEST_SCALE should keep up with TEST_MEMCOPY.  These
 *       tests are designed to defeat systems with a built-in memcpy
 *       instruction that choke while actually doing math.
 *
 *********************************************************************/
static double test_scale(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                         gar_stream_type *c) {
  gar_stream_type *ptr_end = &c[array_size];
  gar_stream_type *ptr = c;
  GRM_unused(a);
  GRM_unused(b);

  memory_access_count[TEST_SCALE] = 2;

  if (NULL == c) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  while (ptr < ptr_end) {
    *ptr = 3 * *ptr;
    ptr++;
  }
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_scale() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       test_triad - Triad Test
 *
 *   DESCRIPTION:
 *       ~7 instructions and 3 accesses per loop
 *
 *       This test is more of a system test; where the difference between
 *       reading/writing the memory are compared, with the act of doing some
 *       simple operations with the memory.  If caches and store-buffers are
 *       working well, TEST_SCALE should keep up with TEST_MEMCOPY.  These
 *       tests are designed to defeat systems with a built-in memcpy
 *       instruction that choke while actually doing math.
 *
 *********************************************************************/
static double test_triad(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                         gar_stream_type *c) {
  gar_stream_type *ptr_end = &a[array_size];
  gar_stream_type *ptr_a = a;
  gar_stream_type *ptr_b = b;
  gar_stream_type *ptr_c = c;

  memory_access_count[TEST_TRIAD] = 3;

  if (NULL == a) return (0);
  if (NULL == b) return (0);
  if (NULL == c) return (0);

  al_signal_start();
  gar_stream_gpio_set();
  while (ptr_a < ptr_end) {
    (*ptr_c++) = (*ptr_b++) + 3 * (*ptr_a++);
  }
  gar_stream_gpio_clear();

  return ((double)al_signal_finished()/(double)al_ticks_per_sec());
} /* test_triad() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       gar_stream_main - Main STREAM Procedure
 *
 *   DESCRIPTION:
 *
 *********************************************************************/
void gar_stream_main(size_t array_size, gar_stream_type *a, gar_stream_type *b,
                     gar_stream_type *c) {
#define runTest(test_id_, test_)                         \
  {                                                      \
    bestrate[(test_id_)] = 0;                            \
    avgtime[(test_id_)] = 0;                             \
    maxtime[(test_id_)] = 0;                             \
    mintime[(test_id_)] = FLT_MAX;                       \
                                                         \
    for (k = 0; k < NTIMES; k++) {                       \
      times[(test_id_)][k] = test_(array_size, a, b, c); \
    }                                                    \
  }

  int j;
  int k;
  double avgt;
  double mint;
  double maxt;

  memset(times, 0, sizeof(times));

  gar_stream_printf(HLINE);
  gar_stream_printf("Read  Array A                = 0x%08x\n", (unsigned int)a);
  gar_stream_printf("Read  Array B                = 0x%08x\n", (unsigned int)b);
  gar_stream_printf("Write Array C                = 0x%08x\n", (unsigned int)c);
  gar_stream_printf("Byte per array element       = %u\n", (unsigned long)sizeof(gar_stream_type));
  gar_stream_printf("Array Count (elements)       = %u\n", (unsigned long)array_size);
  gar_stream_printf("Array Size (bytes)           = %u KB\n",
                    (unsigned long)(sizeof(gar_stream_type) * array_size) / 1024);
  gar_stream_printf("Total Memory (bytes)         = %u KB\n",
                    3 * (unsigned long)(sizeof(gar_stream_type) * array_size) / 1024);
  gar_stream_printf("Test count                   = %u\n", NTIMES);
  gar_stream_printf("Clock Granularity/precision  = %.3f usec\n", (1000000.0 / (double)al_ticks_per_sec()));
  gar_stream_printf(HLINE);

  runTest(TEST_MEMREAD, test_memread);
  runTest(TEST_MEMREAD_REVERSE, test_memread_reverse);
  runTest(TEST_MEMSET, test_memset);
  runTest(TEST_MEMSET_VERTICAL, test_memset_vertical);
  runTest(TEST_MEMSET_CLIB_0, test_memset_clib_0);
  runTest(TEST_MEMSET_CLIB_1, test_memset_clib_1);
  runTest(TEST_MEMMOVE, test_memmove);
  runTest(TEST_MEMMOVE_CLIB, test_memmove_clib);
  runTest(TEST_MEMCOPY, test_memcopy);
  runTest(TEST_MEMCOPY_CLIB, test_memcopy_clib);
  runTest(TEST_SCALE, test_scale);
  runTest(TEST_ADD, test_add);
  runTest(TEST_TRIAD, test_triad);

/*----------------------------------------------------------
Calculate the results.
Note: skip first iteration, so that would most likely result
in cache misses.
----------------------------------------------------------*/
#if (NTIMES > 1)
  for (k = 1; k < NTIMES; k++) {
    for (j = 0; j < TEST_COUNT; j++) {
      avgtime[j] = avgtime[j] + times[j][k];
      mintime[j] = MIN(mintime[j], times[j][k]);
      maxtime[j] = MAX(maxtime[j], times[j][k]);
    }
  }
#else
  for (j = 0; j < TEST_COUNT; j++) {
    avgtime[j] = times[j][0];
    mintime[j] = times[j][0];
    maxtime[j] = times[j][0];
  }
#endif

  gar_stream_printf("Function     Avg Rate MB/s     Avg time     Min time     Max time\n");
  for (j = 0; j < TEST_COUNT; j++) {
#if (NTIMES > 1)
    avgtime[j] = avgtime[j] / (double)(NTIMES - 1);
#endif

    if (0.0 < mintime[j]) {
      bestrate[j] = ((memory_access_count[j] * sizeof(gar_stream_type) * array_size) / avgtime[j]) /
                    BYTES_PER_MB;
    }

    avgt = avgtime[j] * 1000000.0;
    mint = mintime[j] * 1000000.0;
    maxt = maxtime[j] * 1000000.0;
    gar_stream_printf("%s%7u.%1u  %7u.%03u  %7u.%03u  %7u.%03u\n", label[j],
                      (uint32_t)bestrate[j], (uint32_t)((bestrate[j] - (uint32_t)bestrate[j]) * 10),
                      (uint32_t)(avgt), (uint32_t)((avgt - (uint32_t)avgt) * 1000),
                      (uint32_t)(mint), (uint32_t)((mint - (uint32_t)mint) * 1000),
                      (uint32_t)(maxt), (uint32_t)((maxt - (uint32_t)maxt) * 1000));
  }

  gar_stream_printf(HLINE);
} /* gar_stream_main() */

/*********************************************************************
 *
 *   PROCEDURE NAME:
 *       gar_stream_test - Garmin STREAM Test Procedure
 *
 *   DESCRIPTION:
 *
 *********************************************************************/
void gar_stream_test(void) {
  gar_stream_type *buffer1 = NULL;
  gar_stream_type *buffer2 = NULL;
  gar_stream_type *buffer3 = NULL;

  buffer1 = malloc( BENCHMARK_GARSTREAM_CACHE_THRASH_SIZE / sizeof(uint32_t));
  buffer2 = malloc( BENCHMARK_GARSTREAM_CACHE_THRASH_SIZE / sizeof(uint32_t));
  buffer3 = malloc( BENCHMARK_GARSTREAM_CACHE_THRASH_SIZE / sizeof(uint32_t));

  gar_stream_main(BENCHMARK_GARSTREAM_CACHE_EVAL_SIZE / sizeof(gar_stream_type), buffer1,
                  buffer2, buffer3);

  gar_stream_main(BENCHMARK_GARSTREAM_CACHE_THRASH_SIZE / sizeof(gar_stream_type), buffer1,
                  buffer2, buffer3);

  free( buffer1 );
  free( buffer2 );
  free( buffer3 );
} /* gar_stream_test() */
