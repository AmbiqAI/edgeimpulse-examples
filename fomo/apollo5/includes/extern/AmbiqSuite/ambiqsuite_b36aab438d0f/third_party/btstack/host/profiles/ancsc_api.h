//*****************************************************************************
//
//  ancsc_api.h
//! @file
//!
//! @brief Ambiq Micro's demonstration of ANCS client.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef ANCSC_API_H
#define ANCSC_API_H

#include "am_util.h"



#ifdef __cplusplus
extern "C"
{
#endif

/*
 * DEFINES
 ****************************************************************************************
 */

#define ANCSC_REV_ATTR_STRING_PRINT_SIZE    (128)


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/**@brief Category IDs for iOS notifications. */
typedef enum
{
    BLE_ANCS_CATEGORY_ID_OTHER, /**< The iOS notification belongs to the "other"
                                   category.  */
    BLE_ANCS_CATEGORY_ID_INCOMING_CALL, /**< The iOS notification belongs to the
                                           "Incoming Call" category. */
    BLE_ANCS_CATEGORY_ID_MISSED_CALL,   /**< The iOS notification belongs to the
                                           "Missed Call" category. */
    BLE_ANCS_CATEGORY_ID_VOICE_MAIL,    /**< The iOS notification belongs to the
                                           "Voice Mail" category. */
    BLE_ANCS_CATEGORY_ID_SOCIAL,        /**< The iOS notification belongs to the
                                           "Social" category. */
    BLE_ANCS_CATEGORY_ID_SCHEDULE,      /**< The iOS notification belongs to the
                                           "Schedule" category. */
    BLE_ANCS_CATEGORY_ID_EMAIL,         /**< The iOS notification belongs to the
                                           "E-mail" category. */
    BLE_ANCS_CATEGORY_ID_NEWS, /**< The iOS notification belongs to the "News"
                                  category. */
    BLE_ANCS_CATEGORY_ID_HEALTH_AND_FITNESS,   /**< The iOS notification belongs
                                                  to the "Health and Fitness"
                                                  category. */
    BLE_ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE, /**< The iOS notification belongs
                                                  to the "Buisness and Finance"
                                                  category. */
    BLE_ANCS_CATEGORY_ID_LOCATION,     /**< The iOS notification belongs to the
                                          "Location" category. */
    BLE_ANCS_CATEGORY_ID_ENTERTAINMENT /**< The iOS notification belongs to the
                                          "Entertainment" category. */
} ancc_category_id_values_t;

/**@brief Event IDs for iOS notifications. */
typedef enum
{
    BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED, /**< The iOS notification was added.
                                           */
    BLE_ANCS_EVENT_ID_NOTIFICATION_MODIFIED, /**< The iOS notification was
                                                modified. */
    BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED   /**< The iOS notification was
                                                removed. */
} ancc_evt_id_values_t;

/**@brief IDs for iOS notification attributes. */
typedef enum
{
    BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER, /**< Identifies that the attribute
                                              data is of an "App Identifier"
                                              type. */
    BLE_ANCS_NOTIF_ATTR_ID_TITLE, /**< Identifies that the attribute data is a
                                     "Title". Needs to be followed by a 2-bytes
                                     max length parameter*/
    BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE, /**< Identifies that the attribute data is
                                        a "Subtitle". Needs to be followed by a
                                        2-bytes max length parameter*/
    BLE_ANCS_NOTIF_ATTR_ID_MESSAGE, /**< Identifies that the attribute data is a
                                       "Message". Needs to be followed by a
                                       2-bytes max length parameter*/
    BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE, /**< Identifies that the attribute data
                                            is a "Message Size". */
    BLE_ANCS_NOTIF_ATTR_ID_DATE, /**< Identifies that the attribute data is a
                                    "Date". */
    BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL, /**< The notification has a
                                                     "Positive action" that can
                                                     be executed associated with
                                                     it. */
    BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL, /**< The notification has a
                                                     "Negative action" that can
                                                     be executed associated with
                                                     it. */
} ancc_notif_attr_id_values_t;


/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
*
*/
void app_ancs_data_process(uint8_t *ancs_data);


#ifdef __cplusplus
}
#endif

#endif // ANCSC_API_H
