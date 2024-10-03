#include <stdint.h>
#include "test_control_array.h"

// define the test data array at fixed ram address
#if defined (__ARMCC_VERSION)
test_control_array_t sControlArray              __attribute__((section(".ARM.__at_0x10060000"))) = {0};
#elif defined (__IAR_SYSTEMS_ICC__)
__root test_control_array_t sControlArray       @ 0x10060000 = {0};
#elif defined (__GNUC__)
test_control_array_t sControlArray              __attribute__((section(".shared"))) = {0};
#else
#error other compilers not supported.
#endif
