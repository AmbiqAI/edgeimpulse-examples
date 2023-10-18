/**
 * @file main.cc
 * @author Ambiq
 * @brief Basic EdgeImpulse Example (runs on test sample data)
 * @version 0.1
 * @date 2022-12-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>

#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "ns_ambiqsuite_harness.h"
#include "ns_malloc.h"
#include "ns_timer.h"
#include "ns_core.h"
#include "ns_peripherals_button.h"
#include "ns_peripherals_power.h"

// Callback function declaration
static int get_signal_data(size_t offset, size_t length, float *out_ptr);

// Raw features copied from test sample (Edge Impulse > Model testing)
static float input_buf[] = {
    /* Paste your raw features here! */
    2.5139, 0.7159, 1.7334, 2.6624, 0.6883, 0.6560, 2.6588, 0.6225, -0.1796, 2.6456, 0.5064, -1.1839, 2.6193, 0.5339, -1.7370, 2.5055, 0.5064, -1.8902, 2.3750, 0.6093, -1.7585, 2.2362, 0.7075, -1.3180, 1.8304, 0.4812, -0.3148, 1.7334, 0.6381, 0.5602, 1.7011, 0.8164, 1.6173, 1.6472, 1.1588, 2.2482, 1.5598, 1.5024, 2.6001, 1.4234, 1.7693, 2.8874, 1.2857, 1.7549, 3.2166, 1.0427, 1.0858, 4.2210, 1.0223, 0.3998, 4.8411, 0.9840, -0.2598, 6.4009, 1.2186, -0.8033, 8.0098, 1.4377, -0.8440, 9.6223, 1.4497, -0.3831, 10.5261, 1.5107, -0.5459, 10.7548, 1.8364, 0.2765, 11.7759, 1.9632, 1.0726, 11.6717, 1.9692, 1.4174, 11.4120, 2.0985, 1.1935, 11.4419, 2.4062, 0.6907, 12.0045, 2.6444, -0.2430, 12.5696, 2.7833, -1.4281, 12.9454, 3.2920, -2.5582, 15.7718, 3.4716, -2.5103, 16.4254, 3.5458, -2.1620, 16.8348, 3.6619, -1.4018, 16.9222, 3.8008, -0.9732, 17.7542, 4.0306, -0.7015, 18.8795, 4.2078, -1.0918, 20.0347, 4.3443, -0.6548, 20.0838, 4.3191, -0.6393, 19.5343, 4.3060, -0.9433, 18.7143, 4.2653, -0.8080, 17.9745, 4.2976, -1.1229, 17.9900, 4.2461, -1.5598, 17.9421, 4.1635, -1.3934, 17.3747, 3.7362, -1.2701, 16.8875, 3.4524, -1.1085, 16.3967, 3.1328, -0.7769, 15.9322, 2.8539, 0.0096, 15.6868, 2.6815, 0.6524, 15.6162, 2.5666, 0.9541, 15.8029, 1.7262, 1.6197, 13.0879, 1.4078, 1.8016, 11.1199, 1.4234, 1.9321, 10.6087, 1.3970, 1.9704, 9.9323, 1.3144, 1.7777, 8.9447, 1.2450, 1.4856, 7.8362, 1.3180, 1.1432, 6.9157, 1.4736, 0.0730, 5.8454, 1.5095, -0.1149, 4.7309, 1.4712, -0.0838, 3.5733, 1.5646, -0.0048, 2.6659, 1.7322, 0.2693, 2.3403, 1.8783, 0.7254, 2.1847, 1.9992, 0.9613, 1.7885, 2.2134, 1.0774, 0.8918, 2.3128, 1.0523, 0.3843, 2.3523, 0.9469, -0.2538, 2.4337, 0.7242, -0.5962, 2.6061, 0.3974, -0.8236, 2.7234, 0.2107, -1.0403, 2.8287, 0.3220, -1.3719, 2.8766, 0.1101, -2.0889, 2.8599, -0.1053, -2.3200, 2.8802, -0.3711, -2.1440, 2.8228, -0.4058, -1.8124, 2.7066, -0.2682, -1.4293, 2.5917, -0.3783, -0.7865, 2.5833, -0.5698, 0.3771, 2.6396, -0.4429, 2.5343, 2.6612, -0.3424, 3.2465, 2.4852, -0.2550, 3.6440, 2.4708, -0.1748, 4.2186, 2.4816, 0.2753, 4.7621, 2.4900, 0.7925, 5.1870, 2.4684, 1.0044, 5.6503, 2.5175, 0.5004, 5.6491, 2.2637, -0.2394, 5.6324, 2.1093, -0.9325, 5.7616, 1.9321, -1.2905, 6.0478, 1.7322, -1.1540, 6.6690, 1.7526, -1.0570, 7.9476, 1.6891, -0.9649, 10.5201, 2.0279, 0.3531, 11.8082, 1.9178, 0.4657, 12.2404, 2.0578, 0.5842, 12.6522, 2.3774, 1.1660, 13.7415, 2.4588, 1.3946, 14.5172, 2.6588, 0.6668, 14.8943, 3.0299, -0.3065, 17.7051, 3.6452, -1.2965, 18.4701, 3.8152, -1.9609, 18.9729, 4.0211, -1.5718, 19.5941, 4.2760, -1.1349, 20.0299, 4.5789, -0.8344, 20.6368, 4.7561, -0.7326, 21.0606, 4.9261, 0.1245, 20.6931, 4.8985, 0.5578, 19.9832, 4.8542, 0.7099, 19.4289, 4.7297, 0.9505, 18.6041, 4.5466, 0.9014, 17.7147, 4.2485, 0.7422, 16.9701, 3.9133, 0.3268, 15.8041, 3.5267, -0.1125, 15.0260, 3.3267, -0.5698, 14.9111, 3.0514, -0.7374, 14.5125, 2.6169, -0.4549, 13.1837, 2.3272, -0.2885, 11.9542, 2.1835, -0.1472, 11.4179, 2.1452, 0.1700, 11.3976, 1.8759, 0.4932, 10.6961, 1.7190, 0.8978, 9.9359, 1.6089, 1.1839, 9.1040, 1.6364, 0.8930, 8.6730, 1.6867, 0.6943, 8.2013, 1.7011, 0.6752, 7.2245, 1.7190, 0.3543, 5.9735, 1.9106, -0.6440, 4.2904, 1.9477, -0.5004, 3.5961, 1.9752, -0.2023, 2.8204, 2.0806, -0.1425, 2.3439, 2.1691, 0.1915, 2.1979, 2.2218, 0.7602, 2.0123, 2.2039, 1.1995, 1.5215, 2.2338, 1.5754, 0.5662, 2.3559, 1.0798, 0.2131, 2.4588, 0.7649, -0.0168, 2.5618, 0.7015, -0.4908, 2.6851, 0.2538, -1.1073, 2.6995, -0.0539, -1.6927, 2.7150, -0.2394, -2.3236, 2.7665, 0.1844, -2.6133, 2.7294, 0.7913, -1.9453, 2.6372, 1.0846, -1.4377, 2.5103, 0.7937, -0.8942, 2.3008, 0.3196, -0.3268, 2.1739, 0.0503, 0.7183, 2.1811, 0.1712, 2.9197, 2.1308, 0.8128, 3.8307, 2.1572, 0.8559, 4.6543, 2.2529, 0.6895, 5.5007, 2.3355, 0.6165, 5.8909, 2.3248, 0.5231, 5.4911, 2.2661, 0.1532, 4.5801, 1.9213, -0.9661, 2.6085, 1.1995, -1.8687, 2.4445, 1.0774, -1.8938, 3.9923, 0.9146, -1.8483, 6.3267, 1.1576, -1.7083, 9.3218, 1.5467, -0.8547, 12.4295, 2.0111, 0.0251, 15.2104, 2.8730, 1.0666, 18.3192, 3.2872, 0.8595, 19.0566, 3.5027, 0.6512, 18.8627, 3.8092, 0.0251, 18.5203, 4.2102, -0.5626, 18.9142, 4.6112, -0.5195, 19.4026, 4.9536, -0.8392, 19.8372, 5.6264, -1.9932, 21.1935, 5.8742, -2.9856, 22.2445, 6.0406, -3.2549, 22.7090, 6.0597, -2.8623, 22.6120, 5.8969, -2.1021, 21.9824, 5.5701, -1.0738, 21.0079, 5.1691, -0.3412, 20.0610, 4.1899, 0.4884, 17.5471, 3.8235, 0.8799, 16.5918, 3.4213, 1.2965, 15.4426, 3.0694, 1.4700, 14.2096, 2.8790, 1.4772, 13.5536, 2.6480, 1.3479, 13.0855, 2.4253, 1.0511, 12.5372, 1.9058, 1.3659, 11.3653, 1.6580, 1.0810, 10.8589, 1.5634, 1.0295, 10.6793, 1.5311, 1.3539, 10.8948, 1.2749, 1.9764, 10.5656, 1.1995, 2.2877, 9.9144, 1.1121, 2.3152, 6.7145, 1.1372, 1.0750, 5.5330, 1.2330, 0.3723, 4.6615, 1.4880, 0.0204, 3.9863, 1.6783, 0.0898, 3.5602, 1.8878, 0.2131, 2.9173, 2.1081, -0.0718, 2.0411, 2.2170, -0.7254, 0.1101, 2.4409, -0.8248, -0.7111, 2.5929, -0.6728, -1.5814, 2.7892, -0.5351, -1.9740, 2.8970, -0.4513, -2.0985, 3.1005, -0.5782, -1.8830, 3.2262, -0.4345, -1.6316
};

ns_timer_config_t ei_tickTimer = {
    .api = &ns_timer_V1_0_0,
    .timer = NS_TIMER_COUNTER,
    .enableInterrupt = false,
};

int main(int argc, char **argv) {
    ns_core_config_t ns_core_cfg = {.api = &ns_core_V1_0_0};
    
    signal_t signal;            // Wrapper for raw input buffer
    ei_impulse_result_t result; // Used to store inference output
    EI_IMPULSE_ERROR res;       // Return code from inference

    NS_TRY(ns_core_init(&ns_core_cfg), "Core init failed.\b");
    NS_TRY(ns_power_config(&ns_development_default), "Power Init Failed\n");
    ns_interrupt_master_enable();
    ns_itm_printf_enable();
	NS_TRY(ns_timer_init(&ei_tickTimer), "Timer init failed.\n");
    ns_malloc_init();

    // Calculate the length of the buffer
    size_t buf_len = sizeof(input_buf) / sizeof(input_buf[0]);

    // Make sure that the length of the buffer matches expected input length
    if (buf_len != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ns_lp_printf("ERROR: The size of the input buffer is not correct.\r\n");
        ns_lp_printf("Expected %d items, but got %d\r\n", 
                EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, 
                (int)buf_len);
        return 1;
    }

    // Assign callback function to fill buffer used for preprocessing/inference
    signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    signal.get_data = &get_signal_data;

    // Perform DSP pre-processing and inference
    res = run_classifier(&signal, &result, false);

    // Print return code and how long it took to perform inference
    ns_lp_printf("run_classifier returned: %d\r\n", res);
    ns_lp_printf("Timing: DSP %d ms, inference %d ms, anomaly %d ms\r\n", 
            result.timing.dsp, 
            result.timing.classification, 
            result.timing.anomaly);

    // Print the prediction results (object detection)
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    ns_lp_printf("Object detection bounding boxes:\r\n");
    for (uint32_t i = 0; i < EI_CLASSIFIER_OBJECT_DETECTION_COUNT; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) {
            continue;
        }
        ns_lp_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n", 
                bb.label, 
                bb.value, 
                bb.x, 
                bb.y, 
                bb.width, 
                bb.height);
    }

    // Print the prediction results (classification)
#else
    ns_lp_printf("Predictions:\r\n");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ns_lp_printf("  %s: ", ei_classifier_inferencing_categories[i]);
        ns_lp_printf("%.5f\r\n", result.classification[i].value);
    }
#endif

    // Print anomaly result (if it exists)
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ns_lp_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
#endif

    return 0;
}

// Callback: fill a section of the out_ptr buffer when requested
static int get_signal_data(size_t offset, size_t length, float *out_ptr) {
    for (size_t i = 0; i < length; i++) {
        out_ptr[i] = (input_buf + offset)[i];
    }

    return EIDSP_OK;
}