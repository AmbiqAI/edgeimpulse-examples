//*****************************************************************************
//
//! @file  ios_link
//!
//! @brief Functions for using the IOS interface
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef IOS_LINK_H
#define IOS_LINK_H

#ifdef __cplusplus
extern "C"
{
#endif

#define I2C_ADDR                        0x10
#define IOS_LINK_DEFUALT_VALUE          0x20

#define LINK_FRAME_HEAD                 0x3f
#define LINK_FRAME_HEAD_LENGTH          1

#define LINK_FRAME_SIZE                 64
#define LINK_FRAM_DATA_SIZE             (LINK_FRAME_SIZE - 4)

#define LINK_EOK                        'Y'
#define LINK_ERR                        'N'

enum LINK_CMD
{
    LINK_CMD_VERSION = 0xF1,
    LINK_CMD_MUX_MODE ,
    LINK_CMD_MUX_DATA ,
};

typedef union
{
   uint8_t buf[LINK_FRAME_SIZE];
   struct
   {
      uint8_t head;
      uint8_t cmd;
      uint8_t state;
      uint8_t len;
      uint8_t data[LINK_FRAM_DATA_SIZE];
   }frame;

}link_frame_t;



#ifdef __cplusplus
}
#endif


#endif // IOS_LINK_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
