
/**
 *  \file BT_fops.c
 *
 *  This file contains all OS Abstraction functions for file
 *  and directory operations.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* ----------------------------------------- Header File Inclusion */
#include "BT_fops.h"


/* ----------------------------------------- External Global Variables */
extern int errno;

/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */


/* ----------------------------------------- Functions */
/**
 *  \fn BT_fops_get_current_directory
 *
 *  \brief To get the current working directory.
 *
 *  \param [in] current_directory
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_get_current_directory
           (
               /* OUT */  UCHAR  * current_directory
           )
{
#ifndef FREERTOS
    DWORD  dwRetVal;
    DWORD  dwAttrs;

    BT_FOPS_TRC(
    "[FOPS] <- REQ to Get Current Directory\n");

    /* NULL Check */
    if(NULL == current_directory)
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    /* Get current directory */
    dwRetVal = GetCurrentDirectory(MAX_PATH, current_directory);
    BT_FOPS_TRC(
    "[FOPS] Current Directory is  %s\n", current_directory);

    if(0 == dwRetVal)
    {
        BT_FOPS_ERR(
        "[FOPS] FAILED to Get Current Working Directory. Reason (%d)\n",
        GetLastError());

        return BT_FOPS_ERR_GET_CURRECT_DIRECTORY;
    }

    /* Get the Attributes of the file/directory */
    dwAttrs = GetFileAttributes(current_directory);

    /* Check if it is Readonly Folder */
    if (FILE_ATTRIBUTE_READONLY == (dwAttrs & FILE_ATTRIBUTE_READONLY))
    {
        BT_FOPS_TRC(
        "[FOPS] Object is 'Readonly Folder'\n");
    }
    else
    {
        BT_FOPS_TRC(
        "[FOPS] Object is 'Not Readonly Folder'\n");
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}


/**
 *  \fn BT_fops_get_file_attributes
 *
 *  \brief To get the file attributes of a file/directory.
 *
 *  \par Description:
 *  This function is used to find out the attributes of a file/directory.
 *  For ex: is file/directory readonly etc.
 *
 *  \param [in] file
 *
 *  \param [out] file_attribute
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_get_file_attributes
           (
               /* IN */   UCHAR   * object_name,
               /* OUT */  UINT32  * file_attribute
           )
{
#ifndef FREERTOS
    DWORD  dwAttrs;

    /* NULL Check */
    if((NULL == object_name) || (NULL == file_attribute))
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    dwAttrs           = 0;
    *file_attribute   = 0;

    BT_FOPS_TRC(
    "[FOPS] <- REQ to Get File Attributes. Object Name = %s\n",
    object_name);

    /* Get the Attributes of the file/directory */
    dwAttrs = GetFileAttributes(object_name);

    /* Check for failure */
    if (INVALID_FILE_ATTRIBUTES == dwAttrs)
    {
        BT_FOPS_ERR(
        "[FOPS] Get File Attributes Failed. Reason (%d)\n",
        GetLastError());

        return BT_FOPS_ERR_GET_FILE_ATTRIBUTES;
    }

    /* Check if it is Folder */
    if(FILE_ATTRIBUTE_DIRECTORY == (dwAttrs & FILE_ATTRIBUTE_DIRECTORY))
    {
        BT_FOPS_TRC(
        "[FOPS] Object is 'Folder'\n");

        /* Set the bit for Readonly Folder */
        BT_FOPS_SET_BIT(*file_attribute, BT_FOPS_MASK_FOLDER);
    }
    else
    {
        BT_FOPS_TRC(
        "[FOPS] Object is 'File'\n");
    }

    /* Check if it is Readonly Folder */
    if (FILE_ATTRIBUTE_READONLY == (dwAttrs & FILE_ATTRIBUTE_READONLY))
    {
        BT_FOPS_TRC(
        "[FOPS] Object is 'Readonly Folder'\n");

        /* Set the bit for Readonly Folder */
        BT_FOPS_SET_BIT(*file_attribute, BT_FOPS_MASK_FOLDER_READONLY);
    }
    else
    {
        BT_FOPS_TRC(
        "[FOPS] Object is 'Not Readonly Folder'\n");
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}


/**
 *  \fn BT_fops_set_path_forward
 *
 *  \brief To change the current working directory forward.
 *
 *  \param [in] folder_name
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_set_path_forward
           (
               /* IN */  UCHAR *folder_name
           )
{
#ifndef FREERTOS
    int result;

    /* NULL Check */
    if(NULL == folder_name)
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    BT_FOPS_TRC(
    "[FOPS] <- REQ to Set Path Forward - Change Directory to %s\n",
    folder_name);

    /* Change Directory */
    result = _chdir(folder_name);

    if(0 != result)
    {
        BT_FOPS_ERR(
        "[FOPS] FAILED to change the Current Working Directory Forward\n");

        return BT_FOPS_ERR_SET_PATH_FORWARD;
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}


/**
 *  \fn BT_fops_set_path_backward
 *
 *  \brief To change the current working directory backward.
 *
 *  \param [in] folder_name
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_set_path_backward( void )
{
#ifndef FREERTOS
    int result;

    BT_FOPS_TRC(
    "[FOPS] <- REQ to Set Path Backward - Change Directory\n");

    result = _chdir("./../");

    if(0 != result)
    {
        BT_FOPS_ERR(
        "[FOPS] FAILED to change the Current Working Directory Backward\n");

        return BT_FOPS_ERR_SET_PATH_BACKWARD;
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}



/**
 *  \fn BT_fops_create_folder
 *
 *  \brief To Create folder/directory
 *
 *  \param [in] folder_name
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_create_folder
           (
               /* IN */  UCHAR * folder_name
           )
{
#ifndef FREERTOS
    int result;

    /* NULL Check */
    if(NULL == folder_name)
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    BT_FOPS_TRC(
    "[FOPS] <- REQ to Create Folder/Directory. Folder Name = %s\n",
    folder_name);

#ifdef FTP_BQB_TPOMABV03I
    /* To generate BPA logs for create folder not supported test case */
    return BT_FOPS_ERR_CREATE_FOLDER;
#endif /* FTP_BQB_TPOMABV03I */
    result = _mkdir(folder_name);

    if(0 != result)
    {
        switch(errno)
        {
        case EEXIST :
            BT_FOPS_INF(
            "[FOPS]  Folder/Directory to be created. Allready exist\n");
            break;

        case ENOENT :
            BT_FOPS_ERR(
            "[FOPS] FAILED to Create Folder. Reason: Path not found\n");
            return BT_FOPS_ERR_CREATE_FOLDER;
            break;

        default :
            BT_FOPS_ERR(
            "[FOPS] FAILED to Create Folder/Directory. Reason = %d\n", errno);
            return BT_FOPS_ERR_CREATE_FOLDER;
        }
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}



/**
 *  \fn BT_fops_file_open
 *
 *  \brief To open file
 *
 *  \param [in] file_name
 *
 *  \param [in] mode
 *
 *  \param [in] file_handle
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_file_open
           (
               /* IN */  UCHAR                * file_name,
               /* IN */  UCHAR                * mode,
               /* OUT */ BT_fops_file_handle  *file_handle
           )
{
#ifndef FREERTOS
    /* NULL Check */
    if((NULL == file_name) || (NULL == mode) || (NULL == file_handle))
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    *file_handle = fopen((char *)file_name, (char *)mode);

    BT_FOPS_TRC(
    "[FOPS] <- REQ to Open File. File Name = %s\n",
    file_name);

    if(NULL == (*file_handle))
    {
        BT_FOPS_ERR(
        "[FOPS] FAILED To open the file. Reason (%d)\n",
        GetLastError());

        return BT_FOPS_ERR_FILE_OPEN;
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}



/**
 *  \fn BT_fops_file_write
 *
 *  \brief To open file
 *
 *  \param [in] buffer
 *
 *  \param [in] buf_length
 *
 *  \param [in] file_handle
 *
 *  \param [in] bytes_written
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_file_write
           (
               /* IN */   UCHAR               * buffer,
               /* IN */   UINT16                buf_length,
               /* IN */   BT_fops_file_handle   file_handle,
               /* OUT */  UINT16              * bytes_written
           )
{
#ifndef FREERTOS
    /* NULL Check */
    if((NULL == buffer)      ||
       (NULL == file_handle) ||
       (NULL == bytes_written))
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    /* If the length of contents to be written is 0, then return SUCCESS */
    if(0 == buf_length)
    {
       *bytes_written = 0; /* Number of bytes written into the file */
       return API_SUCCESS;
    }

    *bytes_written = (UINT16)fwrite(buffer, sizeof(UCHAR), buf_length, file_handle);

    BT_FOPS_TRC(
    "[FOPS] <- REQ to Write %d bytes to file\n", buf_length);

    if(*bytes_written != buf_length)
    {
        BT_FOPS_ERR(
        "[FOPS] FAILED to write to file\n");

        return BT_FOPS_ERR_FILE_WRITE;
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}


/**
 *  \fn BT_fops_file_read
 *
 *  \brief To open file
 *
 *  \param [in] buffer
 *
 *  \param [in] buf_length
 *
 *  \param [in] file_handle
 *
 *  \param [in] bytes_read
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_file_read
           (
               /* IN */   UCHAR               * buffer,
               /* IN */   UINT16                buf_length,
               /* IN */   BT_fops_file_handle   file_handle,
               /* OUT */  UINT16              * bytes_read
           )
{
#ifndef FREERTOS
    /* NULL Check */
    if((NULL == buffer)      ||
       (NULL == file_handle) ||
       (NULL == bytes_read))
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    /* If the length of contents to be read is 0, then return SUCCESS */
    if(0 == buf_length)
    {
       *bytes_read = 0; /* Number of bytes read from the file*/
       return API_SUCCESS;
    }

    *bytes_read = (UINT16)fread(buffer, sizeof(UCHAR), buf_length, file_handle);

    BT_FOPS_TRC(
    "[FOPS] <- REQ to Read %d bytes from file\n", buf_length);

    if(0x00 == *bytes_read)
    {
        BT_FOPS_ERR(
        "[FOPS] FAILED to Read from file\n");

        return BT_FOPS_ERR_FILE_READ;
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}



/**
 *  \fn BT_fops_file_close
 *
 *  \brief To open file
 *
 *  \param [in] file_handle
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_file_close
           (
               /* IN */  BT_fops_file_handle  file_handle
           )
{
#ifndef FREERTOS
    BT_FOPS_TRC(
    "[FOPS] <- REQ to close the file\n");

    /* NULL Check */
    if(NULL == file_handle)
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }


    if(0 == fclose(file_handle))
    {
        file_handle = NULL;

        return API_SUCCESS;
    }
#endif /* FREERTOS */

    return API_FAILURE;
}


/**
 *  \fn BT_fops_object_delete
 *
 *  \brief To open file
 *
 *  \param [in] file_name
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_object_delete
           (
               /* IN */  UCHAR * object_name
           )
{
#ifndef FREERTOS
    DWORD  dwAttrs;
    BOOL   retval;
    DWORD error;
    HANDLE h;
    WIN32_FIND_DATA FindFileData;
    UCHAR file_folder_name[MAX_PATH];

    /* NULL Check */
    if(NULL == object_name)
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    /* Get the Attributes of the file/directory */
    dwAttrs = GetFileAttributes((LPCTSTR)object_name);
    BT_FOPS_TRC(
    "[FOPS] Get File Attributes returned = %x\n",
     dwAttrs);

    if(0xFFFFFFFF == dwAttrs)
    {
        BT_FOPS_TRC(
        "[FOPS] Get File Attributes Failed. Reason (%d)\n",
        GetLastError());

        return BT_FOPS_ERR_FILE_NOT_FOUND;
    }

    if (FILE_ATTRIBUTE_DIRECTORY == (dwAttrs & FILE_ATTRIBUTE_DIRECTORY))
    {
        /* This is a folder */
        BT_FOPS_TRC(
        "[FOPS] Request to Delete a folder. Folder = %s\n",
        object_name);

        /* Delete/Remove Folder */
        retval = RemoveDirectory(object_name);

        if(0 == retval)
        {
            error = GetLastError();

            BT_FOPS_TRC(
            "[FOPS] FAILED to remove the directory. Reason (%d)\n",
            error);

            /* Recursively delete non-empty folder */
            if (ERROR_DIR_NOT_EMPTY == error)
            {
                BT_str_copy(file_folder_name, object_name);
                BT_str_cat(file_folder_name, "\\*.*");

                /* Get first directory entry and delete */
                h = FindFirstFile(file_folder_name, &FindFileData);
                if (INVALID_HANDLE_VALUE == h)
                {
                    BT_FOPS_ERR(
                    "[FOPS] Invalid File Handle. Get Last Error reports  %d\n",
                    GetLastError());

                    return API_FAILURE;
                }
                else
                {
                    if ((0 != BT_str_cmp(".", FindFileData.cFileName)) &&
                        (0 != BT_str_cmp("..", FindFileData.cFileName)))
                    {
                        BT_str_copy(file_folder_name, object_name);
                        BT_str_cat(file_folder_name, "\\");
                        BT_str_cat(file_folder_name, FindFileData.cFileName);

                        BT_FOPS_INF(
                        "[FOPS] Delete the first file/directory found is %s\n",
                        file_folder_name);

                        BT_fops_object_delete(file_folder_name);
                    }
                }

                /* Delete remaining directory entries */
                error = 0;
                while (ERROR_NO_MORE_FILES != error)
                {
                    retval = FindNextFile(h, &FindFileData);

                    if (0 == retval)
                    {
                        error = GetLastError();
                    }
                    else
                    {
                        if ((0 == BT_str_cmp(".", FindFileData.cFileName)) ||
                            (0 == BT_str_cmp("..", FindFileData.cFileName)))
                        {
                            continue;
                        }

                        BT_str_copy(file_folder_name, object_name);
                        BT_str_cat(file_folder_name, "\\");
                        BT_str_cat(file_folder_name, FindFileData.cFileName);

                        BT_FOPS_INF(
                            "[FOPS] Delete file/directory %s\n",
                            file_folder_name);

                        BT_fops_object_delete(file_folder_name);
                    }
                }

                FindClose(h);

                /* Must be empty now. Try deleting folder again */
                retval = RemoveDirectory(object_name);

                if (0 == retval)
                {
                    error = GetLastError();

                    BT_FOPS_ERR(
                    "[FOPS] FAILED to remove the directory. Reason (%d)\n",
                    error);

                    return BT_FOPS_ERR_FOLDER_DELETE;
                }
            }
            else
            {
                return BT_FOPS_ERR_FOLDER_DELETE;
            }
        }
    }
    else
    {
        /* This is not a folder */
        BT_FOPS_TRC(
        "[FOPS] Request to Delete a file. File = %s\n",
        object_name);

        /* Delete File */
        retval = DeleteFile(object_name);

        BT_FOPS_TRC(
        "[FOPS] DeleteFile Return value = %d\n",
        retval);

        if(0 == retval)
        {
            BT_FOPS_ERR(
            "[FOPS] FAILED to remove the file. Reason (%d)\n",
            GetLastError());

            return BT_FOPS_ERR_FILE_DELETE;
        }
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}

/**
 *  \fn BT_fops_file_size
 *
 *  \brief To find size of a file
 *
 *  \param [in] file_name
 *
 *  \param [out] file_size
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 */
API_RESULT BT_fops_file_size
           (
               /* IN */  BT_fops_file_handle   file_handle,
               /* OUT */ UINT32              * file_size
           )
{
#ifndef FREERTOS
    if ((NULL == file_handle) || (NULL == file_size))
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    fseek (file_handle, 0L, SEEK_END);

    *file_size = ftell (file_handle);

    fseek (file_handle, 0L, SEEK_SET);
#endif /* FREERTOS */

    return API_SUCCESS;

}

/**
 *  \fn BT_fops_file_seek
 *
 *  \brief To set the file position
 *
 *  \param [in] file_handle
 *
 *  \param [in] offset
 *
 *  \param [in] whence
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 */
API_RESULT BT_fops_file_seek
           (
               /* IN */  BT_fops_file_handle   file_handle,
               /* IN */  INT32                 offset,
               /* IN */  INT32                 whence
           )
{
#ifndef FREERTOS
    int retval;

    if (NULL == file_handle)
    {
        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    retval = fseek (file_handle, offset, whence);

    /* Verify if failed */
    if (0 > retval)
    {
        return BT_FOPS_ERR_FILE_SEEK_FAILED;
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}

/**
 *  \fn BT_fops_file_copy
 *
 *  \brief To copy file
 *
 *  \param [in] existing_file_name  Name of an existing file.
 *
 *  \param [in] new_file_name  Name of the new file.
 *
 *  \param [in] fail_if_exists If this parameter is BT_TRUE and
 *                             the new file specified by new_file_name
 *                             already exists, the function fails.
 *                             If this parameter is BT_FALSE and
 *                             the new file already exists,
 *                             the function overwrites the existing file
 *                             and succeeds.
 *
 *  \return API_RESULT: API_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 *
 *  \if PSEUDO_CODE
 *  <B> Pseudo Code </B>
 *  \code
 *
 *  \endcode
 *  \endif
 */
API_RESULT BT_fops_file_copy
           (
               /* IN */  UCHAR * existing_file_name,
               /* IN */  UCHAR * new_file_name,
               /* IN */  UCHAR   fail_if_exists
           )
{
#ifndef FREERTOS
    BOOL copy_file;

    /* NULL Check */
    if ((NULL == existing_file_name) || (NULL == new_file_name))
    {
        BT_FOPS_ERR(
        "[FOPS] File Copy. Invalid Parameters\n");

        return BT_FOPS_INVALID_PARAMETER_VALUE;
    }

    /* Copy File */
    copy_file = CopyFile
                (
                    existing_file_name,
                    new_file_name,
                    fail_if_exists
                );

    BT_FOPS_TRC(
    "[FOPS] CopyFile returned = 0x%02X\n", copy_file);

    if(0 == copy_file)
    {
        BT_FOPS_ERR(
        "[FOPS] FAILED to copy file. Reason (%d)\n",
        GetLastError());

        return BT_FOPS_ERR_FILE_COPY;
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}


/**
 *  \fn BT_vfops_create_object_name
 *
 *  \brief
 *
 *  \Description
 *
 *
 *  \param path
 *  \param object
 *  \param obj_name
 *
 *  \return None
 */
API_RESULT BT_vfops_create_object_name
           (
               /* IN */  UCHAR * path,
               /* IN */  UCHAR * object,
               /* OUT */ UCHAR * obj_name
           )
{
#ifndef FREERTOS
    if ((NULL == path) || (NULL == object) || (NULL == obj_name))
    {
        return API_FAILURE;
    }

    /* Append the path of the new directory */
    BT_str_copy (obj_name, path);
    BT_str_cat (obj_name, "\\");
    BT_str_cat (obj_name, object);
#endif /* FREERTOS */

    return API_SUCCESS;
}

/**
 *  \fn BT_vfops_set_path_backward
 *
 *  \brief
 *
 *  \Description
 *
 *
 *  \param path
 *
 *  \return None
 */

API_RESULT BT_vfops_set_path_backward
           (
               /* OUT */ UCHAR *path
           )
{
#ifndef FREERTOS
    int i;

    if (NULL == path)
    {
        return API_FAILURE;
    }

    /* Get the length of the current path */
    i = BT_str_len (path);

    /* Ignore the first '\' */
    i--;

    /* search till the '\' from back */

    while (i)
    {
        if ('\\' == path[i])
        {
            path[i] = '\0';
            break;
        }
        i--;
    }
#endif /* FREERTOS */

    return API_SUCCESS;
}


/**
 *  \fn BT_vfops_set_path_forward
 *
 *  \brief
 *
 *  \Description
 *
 *
 *  \param path
 *  \param folder
 *
 *  \return None
 */
API_RESULT BT_vfops_set_path_forward
           (
               /* INOUT */  UCHAR * path,
               /* IN */     UCHAR * folder
           )
{
#ifndef FREERTOS
    API_RESULT retval;
    UINT32 file_attr;

    if ((NULL == path) || (NULL == folder))
    {
        return API_FAILURE;
    }

    /* Append the path of the new directory */
    BT_str_cat (path, "\\");
    BT_str_cat (path, folder);

    /* Check if the directory exists or not */
    retval = BT_fops_get_file_attributes (path, &file_attr);

    if ((API_SUCCESS != retval) || !(file_attr & BT_FOPS_MASK_FOLDER))
    {
        /* its either not a valid object or its not a folder */
        BT_vfops_set_path_backward (path);
    }

    return retval;
#else
    return API_SUCCESS;
#endif /* FREERTOS */
}
