/**
 *  \file gatt_db.h
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_GATT_DB_
#define _H_GATT_DB_

/**
 * addgroup gatt_db_module
 */

/**
 * defgroup gatt_db_tuneable_param Tuneable Parameters
 * {
 * This section defines the Tuneable Constants of Data Base Module.
 */

/** Number of Characteristics in the data base */
#define GATT_CHARACTERISTIC_COUNT            42

/** Number of Services in the data base */
#define GATT_SERVICE_COUNT                   8

/** Number of Characteristics that are configurable by the client */
#define GATT_DB_MAX_CONFIGUREABLE_CHAR       7

/** Maximum Length of any Characteristic Value/Descriptor */
#define GATT_DB_MAX_VAL_LENGTH                68

#define GATT_VALUE_ARRAY_SIZE                 269

#define GATT_CONST_VALUE_ARRAY_SIZE           252

#define GATT_DB_PEER_VALUE_ARRAY_SIZE         14

#define GATT_DB_MAX_ATTRIBUTES                101

#define GATT_UUID_ARRAY_SIZE                  120

#define GATT_DB_MAX_TYPE_COUNT                55

#define GATT_DB_MAX_PEER_CONFIGURATION        \
        (GATT_DB_PEER_VALUE_ARRAY_SIZE * BT_MAX_DEVICE_QUEUE_SIZE)

/** \} */

/** Service Instance Reference */

/** GAP Service */
#define GATT_SER_GAP_INST                  0

/** GATT Service */
#define GATT_SER_GATT_INST                 1

/** DeviceInformation Service */
#define GATT_SER_DEV_INFO_INST             2

/** Battery Service */
#define GATT_SER_BATTERY_INST              3

/** Weight Scale Service */
#define GATT_SER_WGT_SCALE_INST            4

/** BodyComposition Service */
#define GATT_SER_BDY_CMP_INST              5

/** UserData Service */
#define GATT_SER_USER_DATA_INST            6

/** CurrentTimeService Service */
#define GATT_SER_CUR_TIME_INST             7

/** Characteristic Instance Reference */

/** DeviceName */
#define GATT_CHAR_DEV_NAME_INST            0

/** Appearance */
#define GATT_CHAR_APPEARANCE_INST          1

/** ServiceChanged */
#define GATT_CHAR_SER_CHNGD_INST           2

/** ManufacturerName */
#define GATT_CHAR_MAN_NAME_INST            3

/** ModelNumber */
#define GATT_CHAR_MODEL_NO_INST            4

/** BatteryLevel */
#define GATT_CHAR_BATTERY_LVL_INST         5

/** WeightMeasurment */
#define GATT_CHAR_WGT_MSRMNT_INST          6

/** WeightScaleFeature */
#define GATT_CHAR_WGT_FEATURE_INST         7

/** BodyCompositionMeasurement */
#define GATT_CHAR_BC_MSRMNT_INST           8

/** BodyCompositionFeature */
#define GATT_CHAR_BC_FEATURE_INST          9

/** FirstName */
#define GATT_CHAR_FIRST_NAME_INST          10

/** LastName */
#define GATT_CHAR_LAST_NAME_INST           11

/** Email */
#define GATT_CHAR_EMAIL_INST               12

/** Age */
#define GATT_CHAR_AGE_INST                 13

/** Gender */
#define GATT_CHAR_GENDER_INST              14

/** DateofBirth */
#define GATT_CHAR_DATE_OF_BIRTH_INST       15

/** Weight */
#define GATT_CHAR_WEGT_INST                16

/** Height */
#define GATT_CHAR_HEGT_INST                17

/** VO2Max */
#define GATT_CHAR_VO2MAX_INST              18

/** HeartrateMax */
#define GATT_CHAR_HRT_RATE_MAX_INST        19

/** RestingHeartRate */
#define GATT_CHAR_REST_HRT_RATE_INST       20

/** MaxRecommHeartRate */
#define GATT_CHAR_MAX_RECOM_HRT_RATE_INST  21

/** AerobicThreshold */
#define GATT_CHAR_AEROB_THSLD_INST         22

/** AnaerobicThreshold */
#define GATT_CHAR_ANROB_THSLD_INST         23

/** SportTypeAerobicAnaerobicThresholds */
#define GATT_CHAR_SPT_AEROB_ANROB_THSLD_INST24

/** DateofThresholdAssessment */
#define GATT_CHAR_DATE_OF_THSLD_ASSEMT_INST25

/** WaistCircumference */
#define GATT_CHAR_WIST_CIRMRCE_INST        26

/** HipCircumference */
#define GATT_CHAR_HIP_CIRMRCE_INST         27

/** FatBurnHeartRateLowerLimit */
#define GATT_CHAR_FAT_BRN_HRT_LOW_LMT_INST 28

/** FatBurnHeartRateUpperLimit */
#define GATT_CHAR_FAT_BRN_HRT_UPR_LMT_INST 29

/** AerobicHeartRateLowerLimit */
#define GATT_CHAR_AEROB_HRT_LOW_LMT_INST   30

/** AerobicHeartRateUpperrLimit */
#define GATT_CHAR_AEROB_HRT_UPR_LMT_INST   31

/** AnaerobicHeartRateLowerLimit */
#define GATT_CHAR_ANROB_HRT_LOW_LMT_INST   32

/** AnaerobicHeartRateUpperLimit */
#define GATT_CHAR_ANROB_HRT_UPR_LMT_INST   33

/** FiveZoneHeartRateLimits */
#define GATT_CHAR_FIVE_ZON_HRT_LMT_INST    34

/** ThreeZoneHeartRat Limits */
#define GATT_CHAR_THREE_ZON_HRT_LMT_INST   35

/** TwoZoneHeartRateLimit */
#define GATT_CHAR_TWO_ZON_HRT_LMT_INST     36

/** DatabaseChangeIncrement */
#define GATT_CHAR_DB_CHNG_INC_INST         37

/** UserIndex */
#define GATT_CHAR_USER_INDEX_INST          38

/** UserControlPoint */
#define GATT_CHAR_USER_CNTRL_PNT_INST      39

/** Language */
#define GATT_CHAR_LANGAGE_INST             40

/** CurrentTime */
#define GATT_CHAR_CUR_TIME_INST            41

#endif /* _H_GATT_DB_ */

