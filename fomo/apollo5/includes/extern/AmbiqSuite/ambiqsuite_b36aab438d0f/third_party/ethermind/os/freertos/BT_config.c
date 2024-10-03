
/**
 *  \file BT_config.c
 *
 *  This file contains an utility function to read configuration details
 *  from a specified configuration file.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */


/* ------------------------------------------ Header File Inclusion */
#include "BT_config.h"


/* ------------------------------------------ External Global Variables */


/* ------------------------------------------ Static Global Variables */


/* ------------------------------------------ Exported Global Variables */


/* ------------------------------------------ Functions */

/**
 *  An utility routine to read Configuration Parameter, related to
 *  EtherMind Bluetooth Protocol Stack from a Configuration File.
 */
INT32 BT_config_read
      (
          /* IN */ CHAR *    filename,
          /* IN */ CHAR *    to_find,
          /* IN */ CHAR *    buffer
      )
{
    INT32 len;
    FILE  *fp;
    CHAR  *token_1, *token_2;
    CHAR  read_line[BT_CONFIG_MAX_CHARS_PER_LINE];
    CHAR  temp_line[BT_CONFIG_MAX_CHARS_PER_LINE];

    /* Open Config File */
    fp = fopen (filename, "r");
    if (NULL == fp)
    {
        return -1;
    }

    BT_LOOP_FOREVER()
    {
        /* Read a Line from the Config File */
        if (NULL == fgets(read_line, BT_CONFIG_MAX_CHARS_PER_LINE, fp))
        {
            /* Close Config File */
            fclose (fp);
            return -2;
        }

        /* Get rid of trailing newline character */
        len = (INT32)BT_str_len (read_line);
        read_line[len - 1] = '\0';

        /* Ignore empty/comment lines */
        if (('\0' == read_line[0]) || ('#' == read_line[0]) ||
            (('/' == read_line[0]) && ('/' == read_line[1])))
        {
            continue;
        }

        /* Make a temporary copy */
        BT_str_copy (temp_line, read_line);

        /* Search for value token */
        token_1 = strtok (temp_line, " \t=");
        if (NULL == token_1)
        {
            continue;
        }

        /* Found a value, does it match ? */
        if (0 == BT_str_cmp(token_1, to_find))
        {
            /* It does ! */
            token_2 = strtok (NULL, " \t=\r\n");
            if (NULL == token_2)
            {
                fclose (fp);
                return -3;
            }

            /* Copy value & return */
            BT_str_copy (buffer, token_2);
            break;
        }
    }

    fclose (fp);
    return 0;
}


