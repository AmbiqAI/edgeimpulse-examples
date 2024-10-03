
/**
 *  \file appl_uri.c
 *
 *
 */

/*
 *  Copyright (C) 2015. Mindtree Limited.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl_uri.h"

#ifdef APPL_URI_UTIL
/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- MACROS */

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
DECL_STATIC DECL_CONST APPL_UTF_CODE_NAME utf_code_name_table[] =
{
    { 0x01,  "UNKNOWN"                              },
    { 0x02,  "aaa:"                                 },
    { 0x03,  "aaas:"                                },
    { 0x04,  "about:"                               },
    { 0x05,  "acap:"                                },
    { 0x06,  "acct:"                                },
    { 0x07,  "cap:"                                 },
    { 0x08,  "cid:"                                 },
    { 0x09,  "coap:"                                },
    { 0x0A,  "coaps:"                               },
    { 0x0B,  "crid:"                                },
    { 0x0C,  "data:"                                },
    { 0x0D,  "dav:"                                 },
    { 0x0E,  "dict:"                                },
    { 0x0F,  "dns:"                                 },
    { 0x10,  "file:"                                },
    { 0x11,  "ftp:"                                 },
    { 0x12,  "geo:"                                 },
    { 0x13,  "go:"                                  },
    { 0x14,  "gopher:"                              },
    { 0x15,  "h323:"                                },
    { 0x16,  "http:"                                },
    { 0x17,  "https:"                               },
    { 0x18,  "iax:"                                 },
    { 0x19,  "icap:"                                },
    { 0x1A,  "im:"                                  },
    { 0x1B,  "imap:"                                },
    { 0x1C,  "info:"                                },
    { 0x1D,  "ipp:"                                 },
    { 0x1E,  "ipps:"                                },
    { 0x1F,  "iris:"                                },
    { 0x20,  "iris.beep:"                           },
    { 0x21,  "iris.xpc:"                            },
    { 0x22,  "iris.xpcs:"                           },
    { 0x23,  "iris.lwz:"                            },
    { 0x24,  "jabber:"                              },
    { 0x25,  "ldap:"                                },
    { 0x26,  "mailto:"                              },
    { 0x27,  "mid:"                                 },
    { 0x28,  "msrp:"                                },
    { 0x29,  "msrps:"                               },
    { 0x2A,  "mtqp:"                                },
    { 0x2B,  "mupdate:"                             },
    { 0x2C,  "news:"                                },
    { 0x2D,  "nfs:"                                 },
    { 0x2E,  "ni:"                                  },
    { 0x2F,  "nih:"                                 },
    { 0x30,  "nntp:"                                },
    { 0x31,  "opaquelocktoken:"                     },
    { 0x32,  "pop:"                                 },
    { 0x33,  "pres:"                                },
    { 0x34,  "reload:"                              },
    { 0x35,  "rtsp:"                                },
    { 0x36,  "rtsps:"                               },
    { 0x37,  "rtspu:"                               },
    { 0x38,  "service:"                             },
    { 0x39,  "session:"                             },
    { 0x3A,  "shttp:"                               },
    { 0x3B,  "sieve:"                               },
    { 0x3C,  "sip:"                                 },
    { 0x3D,  "sips:"                                },
    { 0x3E,  "sms:"                                 },
    { 0x3F,  "snmp:"                                },
    { 0x40,  "soap.beep:"                           },
    { 0x41,  "soap.beeps:"                          },
    { 0x42,  "stun:"                                },
    { 0x43,  "stuns:"                               },
    { 0x44,  "tag:"                                 },
    { 0x45,  "tel:"                                 },
    { 0x46,  "telnet:"                              },
    { 0x47,  "tftp:"                                },
    { 0x48,  "thismessage:"                         },
    { 0x49,  "tn3270:"                              },
    { 0x4A,  "tip:"                                 },
    { 0x4B,  "turn:"                                },
    { 0x4C,  "turns:"                               },
    { 0x4D,  "tv:"                                  },
    { 0x4E,  "urn:"                                 },
    { 0x4F,  "vemmi:"                               },
    { 0x50,  "ws:"                                  },
    { 0x51,  "wss:"                                 },
    { 0x52,  "xcon:"                                },
    { 0x53,  "xcon-userid:"                         },
    { 0x54,  "xmlrpc.beep:"                         },
    { 0x55,  "xmlrpc.beeps:"                        },
    { 0x56,  "xmpp:"                                },
    { 0x57,  "z39.50r:"                             },
    { 0x58,  "z39.50s:"                             },
    { 0x59,  "acr:"                                 },
    { 0x5A,  "adiumxtra:"                           },
    { 0x5B,  "afp:"                                 },
    { 0x5C,  "afs:"                                 },
    { 0x5D,  "aim:"                                 },
    { 0x5E,  "apt:"                                 },
    { 0x5F,  "attachment:"                          },
    { 0x60,  "aw:"                                  },
    { 0x61,  "barion:"                              },
    { 0x62,  "beshare:"                             },
    { 0x63,  "bitcoin:"                             },
    { 0x64,  "bolo:"                                },
    { 0x65,  "callto:"                              },
    { 0x66,  "chrome:"                              },
    { 0x67,  "chrome-extension:"                    },
    { 0x68,  "com-eventbrite-attendee:"             },
    { 0x69,  "content:"                             },
    { 0x6A,  "cvs:"                                 },
    { 0x6B,  "dlna-playsingle:"                     },
    { 0x6C,  "dlna-playcontainer:"                  },
    { 0x6D,  "dtn:"                                 },
    { 0x6E,  "dvb:"                                 },
    { 0x6F,  "ed2k:"                                },
    { 0x70,  "facetime:"                            },
    { 0x71,  "feed:"                                },
    { 0x72,  "feedready:"                           },
    { 0x73,  "finger:"                              },
    { 0x74,  "fish:"                                },
    { 0x75,  "gg:"                                  },
    { 0x76,  "git:"                                 },
    { 0x77,  "gizmoproject:"                        },
    { 0x78,  "gtalk:"                               },
    { 0x79,  "ham:"                                 },
    { 0x7A,  "hcp:"                                 },
    { 0x7B,  "icon:"                                },
    { 0x7C,  "ipn:"                                 },
    { 0x7D,  "irc:"                                 },
    { 0x7E,  "irc6:"                                },
    { 0x7F,  "ircs:"                                },
    { 0x80,  "itms:"                                },
    { 0x81,  "jar:"                                 },
    { 0x82,  "jms:"                                 },
    { 0x83,  "keyparc:"                             },
    { 0x84,  "lastfm:"                              },
    { 0x85,  "ldaps:"                               },
    { 0x86,  "magnet:"                              },
    { 0x87,  "maps:"                                },
    { 0x88,  "market:"                              },
    { 0x89,  "message:"                             },
    { 0x8A,  "mms:"                                 },
    { 0x8B,  "ms-help:"                             },
    { 0x8C,  "ms-settings-power:"                   },
    { 0x8D,  "msnim:"                               },
    { 0x8E,  "mumble:"                              },
    { 0x8F,  "mvn:"                                 },
    { 0x90,  "notes:"                               },
    { 0x91,  "oid:"                                 },
    { 0x92,  "palm:"                                },
    { 0x93,  "paparazzi:"                           },
    { 0x94,  "pkcs11:"                              },
    { 0x95,  "platform:"                            },
    { 0x96,  "proxy:"                               },
    { 0x97,  "psyc:"                                },
    { 0x98,  "query:"                               },
    { 0x99,  "res:"                                 },
    { 0x9A,  "resource:"                            },
    { 0x9B,  "rmi:"                                 },
    { 0x9C,  "rsync:"                               },
    { 0x9D,  "rtmfp:"                               },
    { 0x9E,  "rtmp:"                                },
    { 0x9F,  "secondlife:"                          },
    { 0xA0,  "sftp:"                                },
    { 0xA1,  "sgn:"                                 },
    { 0xA2,  "skype:"                               },
    { 0xA3,  "smb:"                                 },
    { 0xA4,  "smtp:"                                },
    { 0xA5,  "soldat:"                              },
    { 0xA6,  "spotify:"                             },
    { 0xA7,  "ssh:"                                 },
    { 0xA8,  "steam:"                               },
    { 0xA9,  "submit:"                              },
    { 0xAA,  "svn:"                                 },
    { 0xAB,  "teamspeak:"                           },
    { 0xAC,  "teliaeid:"                            },
    { 0xAD,  "things:"                              },
    { 0xAE,  "udp:"                                 },
    { 0xAF,  "unreal:"                              },
    { 0xB0,  "ut2004:"                              },
    { 0xB1,  "ventrilo:"                            },
    { 0xB2,  "view-source:"                         },
    { 0xB3,  "webcal:"                              },
    { 0xB4,  "wtai:"                                },
    { 0xB5,  "wyciwyg:"                             },
    { 0xB6,  "xfire:"                               },
    { 0xB7,  "xri:"                                 },
    { 0xB8,  "ymsgr:"                               },
    { 0xB9,  "example:"                             },
    { 0xBA,  "ms-settings-cloudstorage:"            },
};

/* --------------------------------------------- Functions */

/**
 *  \fn *appl_get_utf_str_name
 *
 *  \brief
 *
 *  \Description
 *  This function is used to get the UTF string name from the UTF code.
 *
 *  \param code
 *
 *  \return char *
 *          Returns the UTF String name or "UNKNOWN"
 */
char *appl_get_utf_str_name (UCHAR code)
{
    int i, size;
    char *utf_name;

    utf_name = "UNKNOWN";
    size = sizeof(utf_code_name_table)/sizeof(APPL_UTF_CODE_NAME);

    for (i = 0; i < size; i++)
    {
        if (utf_code_name_table[i].utf_code == code)
        {
            utf_name = utf_code_name_table[i].name_str;
            break;
        }
    }

    return utf_name;
}

/**
 *  \fn appl_encode_octet_to_utf
 *
 *  \brief
 *
 *  \Description This function encodes the incoming Octet to UTF format.
 *
 *
 *  \param [in]  UCHAR ip_uri_code : UTF URI Code to be UTF encoded
 *               The desired UTF string has to be looked up from
 *               utf_code_name_table and corresponding urf_code has to be
 *               passed as input here.
 *               NOTE: currently only BT SIG assigned values are taken into
 *               consideration.
 *  \param [out] UCHAR * op_utf: This is the buffer into which the encoded UTF
 *               data is populated. The buffer has to be allocated by the
 *               calling function.
 *  \param [out] UCHAR * op_len: This is the length of the encoded output.
 *
 *  \return API_RESULT
 *          API_SUCCESS on successful encoding, API_FAILURE otherwise.
 */
API_RESULT appl_encode_octet_to_utf
           (
               /* IN */  UCHAR  ip_uri_code,
               /* OUT */ UCHAR  *op_utf,
               /* OUT */ UCHAR  *op_len
           )
{
    UCHAR temp_c;

    if ((NULL == op_utf) || (NULL == op_len))
    {
        /* Return error when NULL detected as output params */
        return APPL_URI_NULL_PARAMETER_NOT_ALLOWED;
    }

    /* Validate the incoming character */
    if ((
       (BT_URI_UTF_MB_MIN_VAL <= ip_uri_code) &&
       (BT_URI_UTF_MB_MAX_VAL >= ip_uri_code)
       ))
    {
        /* The output encoded length is 2 bytes */
        *op_len = 2;

        /* Encoding the Upper 2 bits */
        temp_c = ip_uri_code;
        temp_c = (temp_c >> BT_URI_8_11_BITS_UTF_UB_ENC_LEN);
        *(op_utf + 0) =  (BT_URI_8_11_BITS_UTF_UB_MASK | temp_c);

        /* Encoding the Lower 6 bits */
        temp_c = ip_uri_code;
        temp_c = (temp_c & BT_URI_8_11_BITS_UTF_LB_INIT_MASK);
        *(op_utf + 1) =  (BT_URI_8_11_BITS_UTF_LB_MASK | temp_c);
    }
    else if (BT_URI_UTF_SB_MAX_VAL >= ip_uri_code)
    {
        /* The output encoded length is 1 bytes */
        *op_len = 1;
        *(op_utf + 0) = ip_uri_code;
    }
    else
    {
       /*
        * TODO:
        * Have encoding for UTF values greater than 0xFF.
        * Right now the input is restricted to 1-Byte Values
        */
    }

    return API_SUCCESS;
}

/**
 *  \fn appl_decode_from_utf_string
 *
 *  \brief
 *
 *  \Description
 *
 *
 *  \param [in]  UCHAR * ip_str This is the part of ADV data which holds the URI
 *               data to be decoded.
 *  \param [in]  UCHAR ip_len This is the length of the URI data which is to be
 *               decoded
 *  \param [out] UCHAR * op_utf This is an allocated by the calling function
 *               which is used to hold the decoded URI data.
 *  \param [out] UCHAR * op_len This holds the length of the decoded URI data.
 *
 *  \return API_RESULT
 *          API_SUCCESS on successful decoding, API_FAILURE otherwise.
 */
API_RESULT appl_decode_from_utf_string
           (
               /* IN */  UCHAR  * ip_str,
               /* IN */  UINT16 ip_len,
               /* OUT */ UCHAR  * op_utf,
               /* OUT */ UINT16 * op_len
           )
{
    UINT16 loop_l;
    UINT32 index;
    UINT16 temp_op_len;
    UCHAR temp_c;
    API_RESULT retval;

    if ((NULL == op_utf) || (NULL == op_len) || (NULL == ip_str))
    {
        /* Return error when NULL detected as output/Input params */
        return APPL_URI_NULL_PARAMETER_NOT_ALLOWED;
    }

    if (0 == ip_len)
    {
        /* Return error when input params is not valid */
        return APPL_URI_INVALID_PARAMETER_VALUE;
    }

    /* Initialize locals */
    temp_op_len = 0;
    loop_l = ip_len;

    /* Initialize Output Value */
    *op_len = 0;

    for (index = 0; index < loop_l; index++)
    {
        /* check if the input byte is 7 bits */
        if(BT_URI_UTF_SB_MAX_VAL >= *(ip_str+index))
        {
            /* Copy the byte as is into temp */
            temp_c = *(ip_str+index);

            /* According to the URI section from BT CSSver6 Spec,
             * Checking only for the URI Scheme at the start of URI.
             * If the first byte is less than 0x80 then checking
             * the Scheme as Signle-Byte UTF sequence.
             */
            retval = appl_uri_map_string_from_octet
                     (
                         (index == 0) ? BT_TRUE: BT_FALSE,
                         temp_c,
                         op_utf,
                         &temp_op_len
                     );

            if (API_SUCCESS != retval)
            {
                return retval;
            }
        }
        /* Check for 2Byte UTF value */
        else if (BT_URI_UTF_MB_MASK == (BT_URI_UTF_MB_MASK & *(ip_str+index)))
        {
            /* Decode the Upper Byte of UTF MultiByte Sequence */
            temp_c = (*(ip_str+index)) << BT_URI_8_11_BITS_UTF_UB_DEC_LEN;

            /* Increment Index to fetch the Lower Byte of UTF Sequence */
            index += 1;

            /* Check if the Lower Byte of UTF Sequence is Valid */
            if (*(ip_str+index) < BT_URI_UTF_MB_MIN_VAL)
            {
                /* Return Encoding error */
                return APPL_URI_INCORRECT_ENCODING;
            }
            else
            {
                /* If valid, decode the Lower Byte of UTF Sequence */
                temp_c |= (*(ip_str+index) & BT_URI_8_11_BITS_UTF_LB_INIT_MASK);
            }

            /* According to the URI section from BT CSSver6 Spec,
             * Checking only for the URI Scheme at the start of URI.
             * If the first byte is greater than 0x7F then checking
             * the Scheme as Multi-Byte UTF sequence.
             * For Multi-Byte, Index has to be 1 instead of zero as the
             * URI Scheme Assigned number in UTF will have 2 Octets.
             */
            retval = appl_uri_map_string_from_octet
                     (
                         (index == 1) ? BT_TRUE : BT_FALSE,
                         temp_c,
                         op_utf,
                         &temp_op_len
                     );

            if (API_SUCCESS != retval)
            {
                return retval;
            }
        }
        else
        {
            /*
             * Only 2 byte Multi-Byte values are decoded as of now.
             * Other values TDB
             */
        }
    }

    /* Update final output len */
    *op_len = temp_op_len;

    return API_SUCCESS;
}


/**
 *  \fn appl_search_uri_scheme_id
 *
 *  \brief
 *
 *  \Description
 *
 *
 *  \param [in] UCHAR uri
 *  \param [in] UINT16 uri_len
 *  \param [in] UCHAR * scheme_id
 *
 *  \return API_RESULT
 *          API_SUCCESS on successful decoding, API_FAILURE otherwise.
 */
API_RESULT appl_search_uri_scheme_id
           (
               /* IN */  UCHAR  * uri,
               /* IN */  UINT16 uri_len,
               /* OUT */ UCHAR  * scheme_id
           )
{
    API_RESULT retval;
    UINT32 i, size, t_len;

    retval = APPL_URI_UNKNOWN_SCHEME;

    size = sizeof(utf_code_name_table)/sizeof(APPL_UTF_CODE_NAME);

    /*
     * check the URI with utf_code_name_table to check if there is a matching
     * scheme available in the uri
     */
     for (i=0; i < size; i++)
     {
         /*
          * check if incoming uri len is greater than corresponding scheme
          * present in utf_code_name_table table
          */
         t_len = BT_str_len(utf_code_name_table[i].name_str);

         if (uri_len > t_len)
         {
             if (0 == BT_mem_cmp(utf_code_name_table[i].name_str,\
                                uri, t_len))
             {
                 /* The Scheme ID is the current index number */
                 *scheme_id = i;

                 retval = API_SUCCESS;

                 /* Break the loop on Success */
                 break;
             }
         }
     }

     return retval;
}

/**
 *  \fn appl_uri_map_string_from_octet
 *
 *  \brief
 *
 *  \Description
 *
 *
 *  \param [in]     UCHAR flag
 *  \param [in]     UCHAR utf_octet
 *  \param [out]    UCHAR * op
 *  \param [in,out] UINT16 op_len
 *
 *  \return API_RESULT
 *          API_SUCCESS on successful decoding, API_FAILURE otherwise.
 */
API_RESULT appl_uri_map_string_from_octet
           (
               /* IN */    UCHAR  flag,
               /* IN */    UCHAR  utf_octet,
               /* OUT */   UCHAR  * op,
               /* INOUT */ UINT16 * op_len
           )
{
    if ((NULL == op) || (NULL == op_len))
    {
        /* Return error when NULL detected as output params */
        return APPL_URI_NULL_PARAMETER_NOT_ALLOWED;
    }

    if (BT_TRUE == flag)
    {
        /* Copy the UTF str name value from the mapping table */
        if (0 != BT_mem_cmp("UNKNOWN", appl_get_utf_str_name(utf_octet), 7))
        {
            BT_mem_copy
            (
                (op + *(op_len)),
                appl_get_utf_str_name(utf_octet),
                BT_str_len(appl_get_utf_str_name(utf_octet))
            );

            /* Update the output len */
            *(op_len) += BT_str_len(appl_get_utf_str_name(utf_octet));
        }
        else
        {
            /* If value is unknown, Keep the decoded value */
            if (0x01 != utf_octet)
            {
                return APPL_URI_INCORRECT_ENCODING;
            }
        }
    }
    else
    {
        /* Keep the byte as is */
        *(op + *(op_len)) = utf_octet;

        /* And, increment the length */
        *(op_len) += 1;
    }

    return API_SUCCESS;
}


/**
 *  \fn appl_encode_uri_adv_data
 *
 *  \brief
 *
 *  \Description This API Encodes the given URI data into an ADV - URI data
 *  which can be advertised over Air
 *
 *  \param [in]  UCHAR * uri
 *  \param [in]  UINT16 uri_len
 *  \param [out] UCHAR * adv_uri
 *  \param [in,out] UINT16 * adv_uri_len
 *
 *  \return API_RESULT
 *          API_SUCCESS on successful decoding, API_FAILURE otherwise.
 */
API_RESULT appl_encode_uri_adv_data
           (
               /* IN */  UCHAR  * uri,
               /* IN */  UINT16 uri_len,
               /* OUT */ UCHAR  * adv_uri,
               /* INOUT */ UINT16 * adv_uri_len
           )
{
    UCHAR test_ip[APPL_URI_MAX_BUFFER], test_op[APPL_URI_MAX_BUFFER];
    UCHAR test_ip_len, test_op_len, temp_len, out_id;
    API_RESULT retval;
    UINT32 index;

    retval = API_FAILURE;
    BT_mem_set(test_ip, 0x0, sizeof(test_ip));
    BT_mem_set(test_op, 0x0, sizeof(test_op));
    test_ip_len = test_op_len = 0;

    /* NULL Check Validation */
    if ((NULL == uri) || (NULL == adv_uri) || (NULL == adv_uri_len))
    {
        /* Return error when NULL detected as output params */
        return APPL_URI_NULL_PARAMETER_NOT_ALLOWED;
    }

    if ((0x0 == uri_len) || (0x0 == *(adv_uri_len)))
    {
        /* Return error when Input params are of insufficient length */
        return APPL_URI_INVALID_PARAMETER_VALUE;
    }

    /* Find the URI Scheme and related Assigned Number */
    retval = appl_search_uri_scheme_id
             (
                 uri,
                 uri_len,
                 &out_id
             );

    if (APPL_URI_UNKNOWN_SCHEME == retval)
    {
        /* Assign the Scheme ID explicitly as 0x00, since scheme is unknown */
        out_id = 0x00;
    }

    /**
     * The Scheme to be UTF encoded is utf_code_name_table[out_id].utf_code
     * The Offset from where URI has to copied as is:
     *      sizeof(utf_code_name_table[out_id].name_str)
     */
    test_ip[0] = utf_code_name_table[out_id].utf_code;

    test_ip_len += 1;

    if (0x00 != out_id)
    {
        BT_mem_copy
        (
            test_ip + 1,
            (uri + BT_str_len(utf_code_name_table[out_id].name_str)),
            (uri_len - BT_str_len(utf_code_name_table[out_id].name_str))
        );

        test_ip_len += \
        (uri_len - BT_str_len(utf_code_name_table[out_id].name_str));
    }
    else
    {
        /* Copy the URI with Unknown Scheme directly as is */
        BT_mem_copy
        (
            test_ip + 1,
            uri,
            uri_len
        );

        test_ip_len += uri_len;
    }

    for (index = 0; index < test_ip_len; index++ )
    {
        if (BT_URI_UTF_SB_MAX_VAL < test_ip[index])
        {
            /* Encode the UTF value */
            appl_encode_octet_to_utf
            (
                test_ip[index],
                &test_op[test_op_len],
                &temp_len
            );

            test_op_len += temp_len;
        }
        else
        {
            test_op[test_op_len] = test_ip[index];

            test_op_len += 1;
        }
    }

    /*
     * Check if the encoded URI length can fit into the output buffer given
     * If the  Encoded URI length is more then return Error.
     * Else if the Encoded URI length is less than the output buffer max size
     * Copy the Encoded Value into output buffer and update its length.
     */
    if (*(adv_uri_len) < test_op_len)
    {
        /* Make the output len as Zero */
        *(adv_uri_len) = 0x00;

        retval = APPL_URI_INSUFFICIENT_BUFFER_LENGTH;
    }
    else
    {
        /* Copy the data to Output Buffer */
        BT_mem_copy
        (
            adv_uri,
            test_op,
            test_op_len
        );

        /* copy the output buffer len */
        *adv_uri_len = test_op_len;

        retval = API_SUCCESS;
    }

    return retval;
}

/**
 *  \fn appl_decode_uri_adv_data
 *
 *  \brief
 *
 *  \Description This API Decodes the received ADV - URI data into complete URI
 *
 *
 *  \param [in] UCHAR * adv_uri
 *  \param [in] UINT16 adv_uri_len
 *  \param [out] UCHAR * uri
 *  \param [out] UINT16 * uri_len
 *
 *  \return API_RESULT
 *          API_SUCCESS on successful decoding, API_FAILURE otherwise.
 */
API_RESULT appl_decode_uri_adv_data
           (
               /* IN */   UCHAR  * adv_uri,
               /* IN */   UINT16 adv_uri_len,
               /* OUT */  UCHAR  * uri,
               /* OUT */  UINT16 * uri_len
           )
{
    /* Call function to decode the adv uri utf data */
    return appl_decode_from_utf_string
           (
               adv_uri,
               adv_uri_len,
               uri,
               uri_len
           );
}

/**
 *  \fn appl_parse_adv_pkt_for_uri
 *
 *  \brief
 *
 *  \Description This Function should be called once ADV Report IND is received.
 *
 *
 *  \param [in] UCHAR * data This is the complete ADV data Received as META
 *              LE Subevent event during Scanning.
 *  \param [in] UINT16 data_len This is the complete ADV data length Received
 *
 *  \return void
 */
void appl_parse_adv_pkt_for_uri (UCHAR * data, UINT16 data_len)
{
    UCHAR temp_len, decode_flag;
    UCHAR * temp_data;
    UINT32 i;
    UCHAR uri_op[APPL_URI_MAX_BUFFER];
    UINT16 uri_op_len;
    API_RESULT retval;

    decode_flag = BT_FALSE;
    BT_mem_set(uri_op, 0, sizeof(uri_op));

    /* Loop the ADV data pkt to find the URI data location */
    for (i=0; i < data_len; i++)
    {
        temp_len = *(data + i);

        if (0x24 == *(data + i + 1))
        {
            temp_data = (data + i + 2);
            temp_len  -= 1;
            decode_flag = BT_TRUE;
            break;
        }
        else
        {
            i = temp_len;
        }
    }

    /* If URI data is Found in the ADV PKT then decode it */
    if (BT_TRUE == decode_flag)
    {
        /* Call URI ADV Decode API */
        retval = appl_decode_uri_adv_data
                 (
                     temp_data,
                     temp_len,
                     uri_op,
                     &uri_op_len
                 );

        if (API_SUCCESS == retval)
        {
            CONSOLE_OUT ("The Decoded URI Data is %s", uri_op);
            appl_dump_bytes(uri_op, uri_op_len);
        }
    }
}


#ifdef APPL_TEST_URI_UTIL

#define APPL_ENCODE_DECODE_TEST_URI(NUM)                                          \
    CONSOLE_OUT("\n URI Encode Decode of Input" #NUM ": \t %s \n ", input_##NUM); \
    appl_encode_decode_uri                                                        \
    (                                                                             \
        input_##NUM,                                                              \
        BT_str_len(input_##NUM)                                                   \
    );

void appl_encode_decode_uri
     (
         UCHAR  * uri,
         UINT16 uri_len
     )
{
    UCHAR test_uri[APPL_URI_MAX_BUFFER], test_adv[APPL_URI_MAX_BUFFER];
    UINT16 test_adv_len, test_uri_len;

    test_adv_len = sizeof(test_adv);
    test_uri_len = 0;

    appl_encode_uri_adv_data
    (
        uri,
        uri_len,
        test_adv,
        &test_adv_len
    );

    CONSOLE_OUT("\n The Ecoded URI ADV data for sample is:\n");
    appl_dump_bytes(test_adv, test_adv_len);

    /* Call URI ADV Decode API */
    appl_decode_uri_adv_data
    (
        test_adv,
        test_adv_len,
        test_uri,
        &test_uri_len
    );

    CONSOLE_OUT("\nThe Decoded URI is :\n");
    appl_dump_bytes(test_uri,test_uri_len);

    if (0 == BT_mem_cmp(uri, test_uri, test_uri_len))
    {
        CONSOLE_OUT("\n URI Test PASS !!!\n");
    }
    else
    {
        CONSOLE_OUT("\n URI Test FAIL !!!\n");
    }
}

/**
 *  \fn appl_uri_test
 *
 *  \brief
 *
 *  \Description Test function to test the URI API stubs with the data present
 *  in BT SIG CSSver6
 *
 *  \param void
 *
 *  \return void
 */
void appl_uri_test(void)
{
    UCHAR input_1[]  = "example://z.com/Alborg";
    UCHAR input_2[]  = "http://www.bluetooth.com";
    UCHAR input_3[]  = "iris.beep://www.mindtree.com" ;
    UCHAR input_4[]  = "iris.xpc://www.mindtree-test.com"  ;
    UCHAR input_5[]  = "iris.xpcs://www.ernd.mindtree.com" ;
    UCHAR input_6[]  = "iris.lwz://www.btpb_mt.com"  ;
    UCHAR input_7[]  = "jabber://www.pandorazbox.in";
    UCHAR input_8[]  = "ftp://ftp.is.co.za/rfc/rfc1808.txt";
    UCHAR input_9[]  = "gopher://spinaltap.micro.umn.edu";
    UCHAR input_10[] = "http://www.math.uio.no/faq/compression-faq/part1.html";
    UCHAR input_11[] = "mailto:mduerst@ifi.unizh.ch";
    UCHAR input_12[] = "news:comp.infosystems.www.servers.unix";
    UCHAR input_13[] = "telnet://melvyl.ucop.edu/";
    UCHAR input_14[] = "ldap://[2001:db8::7]/c=GB?objectClass?one";
    UCHAR input_15[] = "http://www.ietf.org/rfc/rfc2396.txt";
    UCHAR input_16[] = "tel:+1-816-555-1212";
    UCHAR input_17[] = "telnet://192.0.2.16:80/";
    UCHAR input_18[] = "urn:oasis:names:specification:docbook:dtd:xml:4.1.2";
    UCHAR input_19[] = "ms-settings-cloudstorage://@gmail.com";
    UCHAR input_20[] = "com-eventbrite-attendee://.#twitter.com";
    UCHAR input_21[] = "sips:/ #!%^&*|+AC~";
    UCHAR input_22[] = "scheme://www.test.com";
    UCHAR input_23[] = "itms:pic$90}";
    UCHAR input_24[] = "jar:jms:";
    UCHAR input_25[] = "ircs://rUn:@IntErnet.com";
    UCHAR input_26[] = "crid:+1-816-555-1212";
    UCHAR input_27[] = "aaa://www.bluetooth.com";
    UCHAR input_28[] = "1:+1-816-555-1212";
    UCHAR input_29[] = "mind:123456?/";
    UCHAR input_30[] = "aaasa://_www.bluetooth.com";
    UCHAR input_31[] = "reload:1234567://";

    APPL_ENCODE_DECODE_TEST_URI(1);
    APPL_ENCODE_DECODE_TEST_URI(2);
    APPL_ENCODE_DECODE_TEST_URI(3);
    APPL_ENCODE_DECODE_TEST_URI(4);
    APPL_ENCODE_DECODE_TEST_URI(5);
    APPL_ENCODE_DECODE_TEST_URI(6);
    APPL_ENCODE_DECODE_TEST_URI(7);
    APPL_ENCODE_DECODE_TEST_URI(8);
    APPL_ENCODE_DECODE_TEST_URI(9);
    APPL_ENCODE_DECODE_TEST_URI(10);
    APPL_ENCODE_DECODE_TEST_URI(11);
    APPL_ENCODE_DECODE_TEST_URI(12);
    APPL_ENCODE_DECODE_TEST_URI(13);
    APPL_ENCODE_DECODE_TEST_URI(14);
    APPL_ENCODE_DECODE_TEST_URI(15);
    APPL_ENCODE_DECODE_TEST_URI(16);
    APPL_ENCODE_DECODE_TEST_URI(17);
    APPL_ENCODE_DECODE_TEST_URI(18);
    APPL_ENCODE_DECODE_TEST_URI(19);
    APPL_ENCODE_DECODE_TEST_URI(20);
    APPL_ENCODE_DECODE_TEST_URI(21);
    APPL_ENCODE_DECODE_TEST_URI(22);
    APPL_ENCODE_DECODE_TEST_URI(23);
    APPL_ENCODE_DECODE_TEST_URI(24);
    APPL_ENCODE_DECODE_TEST_URI(25);
    APPL_ENCODE_DECODE_TEST_URI(26);
    APPL_ENCODE_DECODE_TEST_URI(27);
    APPL_ENCODE_DECODE_TEST_URI(28);
    APPL_ENCODE_DECODE_TEST_URI(29);
    APPL_ENCODE_DECODE_TEST_URI(30);
    APPL_ENCODE_DECODE_TEST_URI(31);
}
#endif /* APPL_TEST_URI_UTIL */

#endif /* APPL_URI_UTIL */

