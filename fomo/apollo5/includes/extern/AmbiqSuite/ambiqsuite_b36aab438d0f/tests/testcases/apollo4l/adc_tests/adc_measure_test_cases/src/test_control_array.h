#ifndef __TEST_CONTROL_ARRAY__
#define __TEST_CONTROL_ARRAY__

typedef struct test_control_array {
    // control word:
    // 0x00000000: initilized but not ready
    // 0x00000001: ready by local device
    // 0x00000002: test start by remote host
    // 0x00000003: test end by local device
    uint32_t g_ui32ControlWord;
    int32_t g_ui32InputData[64];
    int32_t g_ui32OutputData[64];

} test_control_array_t;

extern test_control_array_t sControlArray;

#endif // __TEST_CONTROL_ARRAY__