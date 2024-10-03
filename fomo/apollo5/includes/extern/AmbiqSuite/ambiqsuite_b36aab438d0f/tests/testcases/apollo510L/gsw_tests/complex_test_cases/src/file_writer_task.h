//*****************************************************************************
//
//! @file file_writer_task.h
//!
//! @brief Functions and variables related to the file writer task
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef FILE_WRITER_TASK_H
#define FILE_WRITER_TASK_H

#include "common_emmc_fatfs.h"

//*****************************************************************************
//
// XIPMM task handles.
//
//*****************************************************************************
extern TaskHandle_t file_writer_task_handle;
extern TaskHandle_t file_writer_scaffold_task_handle;

typedef enum
{
    FILE_WRITER_CREATE,
    FILE_WRITER_APPEND,
    FILE_WRITER_CLOSE
} eFileWriterOp;

typedef struct
{
    uint8_t* buffer;
    UINT bytes;
    TCHAR filename[32];
    eFileWriterOp op;
} FileWriterMetadata_t;

extern FileWriterMetadata_t g_sFileWriterMetadata;
extern SemaphoreHandle_t g_xFileWriterBufMutex;
extern volatile bool g_bFileWriterTaskComplete;


//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

extern void FileWriterTaskSetup(void);
extern void FileWriterScaffoldTaskSetup(void);
extern void FileWriterTask(void *pvParameters);
extern void FileWriterScaffoldTask(void *pvParameters);

#endif // FILE_WRITER_TASK_H
