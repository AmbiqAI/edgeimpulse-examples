//*****************************************************************************
//
//! @file am_util_tap_detect.h
//!
//! @brief Tap Gesture Detector
//!
//! These functions implement the tap detector utility
//!
//! @addtogroup tap_detect Tap Detect - Tap Gesture Detector
//! @ingroup utils
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_UTIL_TAP_DETECT_H
#define AM_UTIL_TAP_DETECT_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! Enumerated return constants
//
//*****************************************************************************
typedef enum
{
    NO_TAP_DETECTED = 0,
    TAP_OCCURED,           //! reports every tap
    TAP_DETECTED,          //! only if a single tap is not part of double/triple
    DOUBLE_TAP_DETECTED,
    TRIPLE_TAP_DETECTED
}am_util_tap_detect_enum_t;

//*****************************************************************************
//
//! Tap Detector Data Structure
//
//*****************************************************************************
typedef struct am_util_tap_detector
{
    float accX;
    float accY;
    float accZ;
    float prev_accX;
    float prev_accY;
    float prev_accZ;
    float SlopeThreshold;
    int   previous_peak_location;
    int   current_sample;
    int   sample_count;
    bool  start_flag;
    float sample_rate;
    //! in terms of seconds
    float  peak_min_width_seconds;
    float  group_peak_max_threshold_seconds;
    //! in terms of samples
    int  peak_min_width_samples;
    int  group_peak_max_threshold;
    //! record the max peak found so you can adjust threshold in future
    float max_mag;
    //! magnitude of the partial derivatives
    float mag;
    int   dist;
    //! previous peak was double tap -> flag to sep[arate out double tap pairs
    int previous_tap_was_dbl_tap;

} am_util_tap_detect_t;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize tap detector structure
//!
//! @param tap is a pointer to the tap detector structure
//! @param dp_min_seconds minimum time to detect double (or triple) tap
//! @param dp_max_seconds maximum time to detect double (or triple) tap
//! @param srate is the sample rate at which the accel runs typically 400 or 200
//! @param slope_thresh is the sensitivity setting for tap detection, typ 800
//!
//! This function initializes the tap detector structure and sets various
//! settings, e.g. min/max times for classifying single, double or triple taps.
//! In addition the structures tells the tap detector how long one sample is
//! in time. Finally, it specifies the sensitiviy of tap detection by setting
//! a minimimum slope threshold to signal tap detections.
//
//*****************************************************************************
extern void am_util_tap_detect_init(am_util_tap_detect_t * tap,
                                    float dp_min_seconds,
                                    float dp_max_seconds,
                                    float srate,
                                    float slope_thresh);

//*****************************************************************************
//
//! @brief Print the contents of the Tap Detector Structure
//!
//! @param tap is a pointer to the tap detector structure
//!
//! This function will print the contents of the tap detector structure if
//! needed for debug.
//
//*****************************************************************************
extern void am_util_tap_detect_print(am_util_tap_detect_t * tap);

//*****************************************************************************
//
//! @brief Process One Sample (Triplet) Through the Tap Dector
//!
//! @param tap is a pointer to the tap detector structure
//! @param accX Accelerometer X axis value of a triplet
//! @param accY Accelerometer Y axis value of a triplet
//! @param accZ Accelerometer Z axis value of a triplet
//!
//! This function utilizes the tap detector structure in conjunction with sample
//! counting to establish all necessary timing.
//!
//! @return NO_TAP, TAP_OCCURED, TAP, DOUBLE, TRIPLE
//
//*****************************************************************************
extern am_util_tap_detect_enum_t am_util_tap_detect_process_sample(
                                        am_util_tap_detect_t * tap,
                                        short accX, short accY, short accZ);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_TAP_DETECT_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

