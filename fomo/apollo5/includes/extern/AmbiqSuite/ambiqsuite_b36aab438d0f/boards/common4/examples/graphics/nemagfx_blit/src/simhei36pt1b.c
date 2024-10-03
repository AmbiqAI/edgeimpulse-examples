//*****************************************************************************
//
//! @file simhei36pt1b.c
//!
//! @brief NemaGFX example.
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

#ifndef SIMHEI36PT1B_C
#define SIMHEI36PT1B_C

#include "simhei36pt1b.h"

#ifndef NEMA_GPU_MEM
#define NEMA_GPU_MEM
#endif // NEMA_GPU_MEM

// This will be read by the GPU only
/*
#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = 16
const unsigned char g_ui8Simhei36pt1bBitmaps[] =
#else
const unsigned char g_ui8Simhei36pt1bBitmaps[]  __attribute__ ((aligned (16))) =
#endif
{
*/

const uint8_t g_ui8Simhei36pt1bBitmaps[] = {
  // 0x9c40 - 0x9c49
  0x1F, 0xFF, 0xCF, 0xFF, 0xFE, 0x00, 0x0F, 0xFF, 0xCF, 0xFF, 0xFE, 0x00,
  0x0E, 0x01, 0xC0, 0x07, 0x00, 0x00, 0x0E, 0x01, 0xC0, 0x07, 0x00, 0x00,
  0x0F, 0xFF, 0xC3, 0x87, 0x00, 0x00, 0x0F, 0xFF, 0xC7, 0x07, 0x00, 0x00,
  0x0E, 0x01, 0xC7, 0x07, 0x00, 0x00, 0x0E, 0x01, 0xCF, 0xFF, 0xFF, 0x00,
  0x0E, 0x01, 0xCF, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xCF, 0xFF, 0xFF, 0x00,
  0x0F, 0xFF, 0xC0, 0x1F, 0x80, 0x00, 0x0E, 0x04, 0x00, 0x1F, 0x80, 0x00,
  0x0E, 0x0E, 0x00, 0x1F, 0x80, 0x00, 0x0E, 0x0F, 0x00, 0x3F, 0x80, 0x00,
  0x0E, 0x07, 0x80, 0x7B, 0x80, 0x00, 0x1E, 0x3F, 0xC0, 0xF3, 0x81, 0xC0,
  0x1E, 0xFD, 0xE1, 0xF3, 0x81, 0xC0, 0x1F, 0xE0, 0xC3, 0xE3, 0xC1, 0xC0,
  0x3F, 0x80, 0x8F, 0xC3, 0xFF, 0xC0, 0x1E, 0x18, 0x1F, 0x81, 0xFF, 0x80,
  0x08, 0x1E, 0x06, 0x00, 0xFF, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x7F, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
  0x01, 0xF0, 0x00, 0x7C, 0x00, 0x00, 0x03, 0xE0, 0x00, 0xF8, 0x00, 0x00,
  0x07, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFC, 0x00,
  0x3F, 0xC0, 0x07, 0x00, 0x1C, 0x00, 0x1D, 0xC0, 0x07, 0x00, 0x1C, 0x00,
  0x09, 0xC0, 0x07, 0x00, 0x1C, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFC, 0x00,
  0x01, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x01, 0xC0, 0x07, 0x00, 0x1C, 0x00,
  0x01, 0xC0, 0x07, 0x00, 0x1C, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFC, 0x00,
  0x01, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x0F, 0x01, 0x80, 0x18, 0x07, 0x80, 0x1E, 0x01, 0xC0, 0x1C, 0x03, 0xC0,
  0x38, 0x01, 0xC0, 0x1C, 0x01, 0xE0, 0xF8, 0x00, 0xE0, 0x0E, 0x00, 0xF0,
  0x30, 0x00, 0x80, 0x0C, 0x00, 0x40, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x07, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x7F, 0xFF, 0xC0,
  0x07, 0x00, 0x30, 0x7F, 0xFF, 0xC0, 0x07, 0x00, 0x70, 0x78, 0xF3, 0xC0,
  0x0F, 0xFC, 0x78, 0x01, 0xE0, 0x00, 0x0F, 0xF8, 0x3C, 0x01, 0xC0, 0x00,
  0x1C, 0x38, 0x1C, 0x03, 0xC0, 0x00, 0x1C, 0x30, 0x1E, 0x07, 0x80, 0x00,
  0x38, 0x70, 0x0E, 0x0F, 0x01, 0x80, 0x38, 0x70, 0x0C, 0x1F, 0x01, 0xC0,
  0x70, 0xE0, 0x00, 0x7F, 0x83, 0xC0, 0x70, 0xE0, 0x01, 0xFB, 0x87, 0x80,
  0xFF, 0xFE, 0x00, 0x71, 0xC7, 0x00, 0xFF, 0xFE, 0x00, 0x43, 0xCF, 0x00,
  0x58, 0xCE, 0x00, 0x03, 0xFE, 0x00, 0x18, 0xCE, 0x7C, 0x07, 0xFC, 0x00,
  0x18, 0xCE, 0x7C, 0x0F, 0xF8, 0x00, 0x18, 0xCE, 0x7C, 0x1E, 0x78, 0x00,
  0x18, 0xCE, 0x0C, 0x3C, 0x7C, 0x00, 0x1F, 0xFE, 0x0C, 0x78, 0x7E, 0x00,
  0x1F, 0xFE, 0x0C, 0xF0, 0xFF, 0x00, 0x1F, 0xFE, 0x0C, 0x60, 0xFF, 0x80,
  0x18, 0xCE, 0x0C, 0x01, 0xFB, 0xC0, 0x18, 0xCE, 0x0C, 0x03, 0xBB, 0xE0,
  0x18, 0xCE, 0x0C, 0x07, 0xB9, 0xE0, 0x18, 0xCE, 0x0C, 0x0F, 0x38, 0xF0,
  0x38, 0xCE, 0x0C, 0x1E, 0x38, 0x60, 0x38, 0xCE, 0x0C, 0x3C, 0x38, 0x40,
  0x3F, 0xFE, 0x0C, 0x78, 0x38, 0x00, 0x3F, 0xFE, 0x0C, 0xF0, 0x38, 0x00,
  0x00, 0x00, 0x0C, 0x60, 0x38, 0x00, 0x00, 0x00, 0x0C, 0x40, 0x38, 0x00,
  0x00, 0x00, 0x0C, 0x00, 0x78, 0x00, 0x33, 0x33, 0x0C, 0x07, 0xF0, 0x00,
  0x73, 0x33, 0x1E, 0x07, 0xF0, 0x00, 0x73, 0x33, 0x1F, 0x03, 0xE0, 0x00,
  0x73, 0x33, 0x3F, 0xC3, 0x80, 0x00, 0x63, 0x3B, 0xF1, 0xF0, 0x00, 0x00,
  0x63, 0x38, 0xF0, 0x7F, 0xFF, 0xF0, 0xE3, 0x01, 0xE0, 0x3F, 0xFF, 0xE0,
  0xE0, 0x00, 0xE0, 0x07, 0xFF, 0xE0, 0x00, 0x00, 0x40, 0x00, 0x1F, 0xC0,
  0x03, 0x00, 0x00, 0x00, 0x30, 0x00, 0x03, 0xC0, 0x00, 0xE0, 0x3C, 0x00,
  0x03, 0x80, 0x00, 0xE0, 0x38, 0x00, 0x07, 0x00, 0x00, 0xE0, 0x78, 0x00,
  0x07, 0xFE, 0x38, 0xE0, 0x7F, 0xF0, 0x0F, 0xFC, 0x38, 0xE0, 0x7F, 0xE0,
  0x0F, 0xFC, 0x38, 0xE0, 0xFF, 0xE0, 0x1E, 0x38, 0x38, 0xE0, 0xE0, 0xE0,
  0x1C, 0x38, 0x38, 0xE1, 0xC1, 0xE0, 0x3C, 0x30, 0x38, 0xE3, 0xE1, 0xC0,
  0x38, 0x70, 0x38, 0xE3, 0xB1, 0xC0, 0x78, 0x60, 0x38, 0xE7, 0xBB, 0x80,
  0x70, 0xE0, 0x38, 0xE7, 0x93, 0x80, 0xFF, 0xFF, 0x38, 0xEF, 0x87, 0x00,
  0xFF, 0xFF, 0x3F, 0xEF, 0xCF, 0x00, 0x1C, 0xE7, 0x3F, 0xE4, 0xCE, 0x00,
  0x1C, 0xE7, 0x00, 0xE0, 0x9E, 0x00, 0x1C, 0xE7, 0x00, 0xE0, 0x3C, 0x00,
  0x1C, 0xE7, 0x00, 0xE0, 0x78, 0x00, 0x1C, 0xE7, 0x00, 0xE0, 0xF3, 0x80,
  0x1F, 0xFF, 0x00, 0xE3, 0xE3, 0x80, 0x1F, 0xFF, 0x00, 0xE7, 0xC3, 0x80,
  0x1F, 0xFF, 0x00, 0xE7, 0x83, 0x80, 0x1C, 0xE7, 0x20, 0xE0, 0x03, 0x80,
  0x1C, 0xE7, 0x3F, 0xE0, 0x03, 0x80, 0x1C, 0xE7, 0x3F, 0xE7, 0xFF, 0xF0,
  0x1C, 0xE7, 0x0C, 0xE7, 0xFF, 0xF0, 0x1C, 0xE7, 0x0C, 0xE0, 0x03, 0x80,
  0x1F, 0xFF, 0x0C, 0xE0, 0x03, 0x80, 0x1F, 0xFF, 0x1C, 0xE0, 0x03, 0x80,
  0x1F, 0xFF, 0x1C, 0xE1, 0x03, 0x80, 0x00, 0x00, 0x1C, 0xE3, 0x83, 0x80,
  0x00, 0x00, 0x1C, 0xE3, 0x83, 0x80, 0x00, 0x00, 0x1C, 0xE1, 0xC3, 0x80,
  0x38, 0x36, 0x1C, 0xE0, 0xE3, 0x80, 0x33, 0x37, 0x38, 0xE0, 0xE3, 0x80,
  0x33, 0x33, 0x38, 0xE0, 0x63, 0x80, 0x33, 0x33, 0x38, 0xE0, 0x43, 0x80,
  0x73, 0x33, 0xB8, 0xE0, 0x03, 0x80, 0x73, 0x33, 0x70, 0xE0, 0x03, 0x80,
  0x63, 0x30, 0x70, 0xE0, 0x1F, 0x80, 0x63, 0x00, 0xE0, 0xE0, 0x1F, 0x80,
  0xE0, 0x00, 0x60, 0xE0, 0x0F, 0x00, 0x00, 0x00, 0x20, 0x00, 0x0C, 0x00,
  0x00, 0x00, 0x00, 0x30, 0x0C, 0x00, 0x01, 0xC0, 0x00, 0x78, 0x0E, 0x00,
  0x01, 0xE0, 0x00, 0x38, 0x1E, 0x00, 0x03, 0xC0, 0x00, 0x3C, 0x1C, 0x00,
  0x03, 0x80, 0x00, 0x1C, 0x1C, 0x00, 0x07, 0x80, 0x00, 0x18, 0x38, 0x00,
  0x07, 0xFF, 0x80, 0x00, 0x38, 0x00, 0x0F, 0xFF, 0x0F, 0xFF, 0xFF, 0xE0,
  0x0E, 0x07, 0x0F, 0xFF, 0xFF, 0xE0, 0x1E, 0x0E, 0x0F, 0xFF, 0xFF, 0xE0,
  0x3C, 0x0E, 0x00, 0x07, 0x80, 0x00, 0x38, 0x0C, 0x00, 0x07, 0x00, 0x00,
  0x78, 0x1C, 0x00, 0x07, 0x00, 0x00, 0xFF, 0xFF, 0xE0, 0x07, 0x00, 0x00,
  0x6F, 0xFF, 0xE7, 0xFF, 0xFF, 0xC0, 0x0E, 0x38, 0xE7, 0xFF, 0xFF, 0xC0,
  0x0E, 0x38, 0xE0, 0x0E, 0x00, 0x00, 0x0E, 0x38, 0xE0, 0x0E, 0x00, 0x00,
  0x0E, 0x38, 0xE0, 0x0E, 0x00, 0x00, 0x0E, 0x38, 0xEF, 0xCE, 0x03, 0xF0,
  0x0F, 0xFF, 0xEF, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0xEF, 0xFF, 0xFF, 0xF0,
  0x0E, 0x38, 0xE0, 0x1C, 0x00, 0x00, 0x0E, 0x38, 0xE0, 0x3C, 0x00, 0x00,
  0x0E, 0x38, 0xE0, 0x38, 0x00, 0x00, 0x0E, 0x38, 0xE0, 0x3F, 0xFF, 0xC0,
  0x0E, 0x38, 0xE0, 0x7F, 0xFF, 0xC0, 0x0F, 0xFF, 0xE0, 0x7F, 0xFF, 0xC0,
  0x0F, 0xFF, 0xE0, 0x71, 0x81, 0xC0, 0x00, 0x00, 0x00, 0xE3, 0x81, 0xC0,
  0x00, 0x00, 0x00, 0xE3, 0x81, 0xC0, 0x00, 0x00, 0x01, 0xC3, 0x81, 0xC0,
  0x3D, 0x9C, 0xC1, 0xFF, 0xFF, 0xC0, 0x39, 0xCC, 0xE3, 0xBF, 0xFF, 0xC0,
  0x39, 0xCC, 0xE7, 0xBF, 0xFF, 0xC0, 0x39, 0xCC, 0x67, 0x03, 0x83, 0x80,
  0x71, 0xCE, 0x7F, 0x03, 0x83, 0x80, 0x70, 0xCE, 0x0E, 0x03, 0x03, 0x80,
  0x70, 0xC0, 0x1E, 0x07, 0x03, 0x80, 0xE0, 0x80, 0x3C, 0x07, 0x03, 0x80,
  0x20, 0x00, 0x79, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x31, 0xFF, 0xFF, 0xF0,
  0x00, 0x00, 0x11, 0xFF, 0xFF, 0xF0, 0x01, 0x80, 0x00, 0x03, 0x80, 0x00,
  0x01, 0xE0, 0x00, 0x03, 0x80, 0x00, 0x03, 0xC0, 0x00, 0x03, 0x80, 0x00,
  0x03, 0x80, 0x1F, 0xFF, 0xFF, 0xF0, 0x03, 0x80, 0x1F, 0xFF, 0xFF, 0xF0,
  0x07, 0xFF, 0x9C, 0x03, 0x80, 0x70, 0x07, 0xFF, 0x00, 0x03, 0x80, 0x00,
  0x0F, 0xFF, 0x00, 0x03, 0x80, 0x00, 0x0E, 0x0E, 0x07, 0xFF, 0xFF, 0x80,
  0x1C, 0x0E, 0x07, 0xFF, 0xFF, 0x80, 0x1C, 0x0E, 0x07, 0xFF, 0xFF, 0x80,
  0x38, 0x1C, 0x07, 0x03, 0x83, 0x80, 0x78, 0x1C, 0x07, 0x03, 0x83, 0x80,
  0xFF, 0xFF, 0xC7, 0xFF, 0xFF, 0x80, 0xEF, 0xFF, 0xC7, 0xFF, 0xFF, 0x80,
  0x2C, 0x31, 0xC7, 0x03, 0x83, 0x80, 0x0C, 0x31, 0xC7, 0x03, 0x83, 0x80,
  0x0C, 0x31, 0xC7, 0x03, 0x83, 0x80, 0x0C, 0x31, 0xC7, 0xFF, 0xFF, 0x80,
  0x0C, 0x31, 0xC7, 0xFF, 0xFF, 0x80, 0x0F, 0xFF, 0xC0, 0x03, 0x80, 0x00,
  0x0F, 0xFF, 0xC0, 0x03, 0x81, 0xC0, 0x0C, 0x31, 0xC0, 0x03, 0x9F, 0xC0,
  0x0C, 0x31, 0xC7, 0xFF, 0xFF, 0xC0, 0x0C, 0x31, 0xCF, 0xFF, 0xFE, 0xE0,
  0x0C, 0x31, 0xCF, 0xFC, 0x00, 0xE0, 0x0C, 0x31, 0xC6, 0x00, 0x1C, 0x80,
  0x0C, 0x31, 0xC0, 0x00, 0x1C, 0x00, 0x0F, 0xFF, 0xC0, 0x00, 0x1C, 0x00,
  0x0F, 0xFF, 0xDF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x1F, 0xFF, 0xFF, 0xF0,
  0x00, 0x00, 0x1F, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00,
  0x30, 0x0C, 0xE0, 0x00, 0x1C, 0x00, 0x39, 0x8C, 0x60, 0xC0, 0x1C, 0x00,
  0x39, 0x8C, 0x61, 0xC0, 0x1C, 0x00, 0x31, 0x8C, 0x60, 0xE0, 0x1C, 0x00,
  0x71, 0xCE, 0x70, 0xF0, 0x1C, 0x00, 0x71, 0xCE, 0x30, 0x70, 0x1C, 0x00,
  0x71, 0xC6, 0x30, 0x78, 0x1C, 0x00, 0xE1, 0xC6, 0x00, 0x31, 0xFC, 0x00,
  0xE0, 0xC0, 0x00, 0x20, 0xFC, 0x00, 0x20, 0x00, 0x00, 0x00, 0xF8, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
  0x01, 0x00, 0x00, 0x03, 0x80, 0x00, 0x03, 0xC0, 0x00, 0x03, 0x80, 0x00,
  0x03, 0x80, 0x00, 0x01, 0xC0, 0x00, 0x03, 0x80, 0x00, 0x01, 0xC0, 0x00,
  0x07, 0x00, 0x0F, 0xFF, 0xFF, 0xF0, 0x07, 0x00, 0x0F, 0xFF, 0xFF, 0xF0,
  0x0F, 0xFF, 0x0E, 0x00, 0xE0, 0x30, 0x0F, 0xFE, 0x0E, 0x00, 0xE0, 0x00,
  0x1C, 0x0E, 0x0E, 0x00, 0xE0, 0x00, 0x1C, 0x0E, 0x0E, 0x3F, 0xFF, 0xC0,
  0x38, 0x1C, 0x0E, 0x3F, 0xFF, 0xC0, 0x78, 0x1C, 0x0E, 0x3F, 0xFF, 0xC0,
  0x70, 0x1C, 0x0E, 0x00, 0xE1, 0xC0, 0xFF, 0xFF, 0xCE, 0x00, 0xE1, 0xC0,
  0x7F, 0xFF, 0xCE, 0x7F, 0xFF, 0xF0, 0x1C, 0x71, 0xCE, 0x7F, 0xFF, 0xF0,
  0x1C, 0x71, 0xCE, 0x7F, 0xFF, 0xF0, 0x1C, 0x71, 0xCE, 0x00, 0xE1, 0xC0,
  0x1C, 0x71, 0xCE, 0x00, 0xE1, 0xC0, 0x1C, 0x71, 0xCE, 0x3F, 0xE1, 0xC0,
  0x1F, 0xFF, 0xCE, 0x3F, 0xFF, 0xC0, 0x1F, 0xFF, 0xCE, 0x3F, 0xFF, 0xC0,
  0x1F, 0xFF, 0xCE, 0x00, 0xE0, 0x00, 0x1C, 0x71, 0xCE, 0x00, 0xE0, 0x00,
  0x1C, 0x71, 0xCE, 0x00, 0xE0, 0x00, 0x1C, 0x71, 0xCE, 0x7F, 0xFF, 0xC0,
  0x1C, 0x71, 0xCE, 0x7F, 0xFF, 0xC0, 0x1C, 0x71, 0xCE, 0x70, 0xE1, 0xC0,
  0x1F, 0xFF, 0xCE, 0x70, 0xE1, 0xC0, 0x1F, 0xFF, 0xCE, 0x70, 0xE1, 0xC0,
  0x00, 0x00, 0x0E, 0x7F, 0xFF, 0xC0, 0x00, 0x00, 0x0E, 0x7F, 0xFF, 0xC0,
  0x00, 0x00, 0x0E, 0x70, 0xE1, 0xC0, 0x39, 0x9B, 0x8E, 0x70, 0xE1, 0xC0,
  0x39, 0x99, 0x8C, 0x70, 0xE1, 0xC0, 0x39, 0x99, 0x9C, 0x7F, 0xFF, 0xC0,
  0x39, 0x9D, 0xDC, 0x7F, 0xFF, 0xC0, 0x31, 0xDC, 0xDC, 0x70, 0xE1, 0xC0,
  0x71, 0xCC, 0x9C, 0x70, 0xE1, 0xC0, 0x71, 0xCC, 0x3C, 0x70, 0xE1, 0xC0,
  0x70, 0xC0, 0x38, 0x70, 0xE1, 0xC0, 0xF0, 0x00, 0x38, 0x70, 0xE7, 0xC0,
  0x00, 0x00, 0x38, 0x70, 0xE7, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x01, 0x80, 0x00, 0x07, 0x80, 0x00,
  0x01, 0xE0, 0x00, 0x03, 0x80, 0x00, 0x03, 0xC0, 0x00, 0x03, 0x80, 0x00,
  0x03, 0x80, 0x00, 0x03, 0x80, 0x00, 0x07, 0x80, 0x03, 0x00, 0x00, 0x60,
  0x07, 0xFF, 0x83, 0xFF, 0xFF, 0xE0, 0x07, 0xFF, 0x03, 0xFF, 0xFF, 0xE0,
  0x0E, 0x07, 0x00, 0x00, 0x18, 0x00, 0x0E, 0x0E, 0x00, 0x30, 0x1E, 0x00,
  0x1C, 0x0E, 0x00, 0x70, 0x1C, 0x00, 0x3C, 0x0C, 0x00, 0x38, 0x3C, 0x00,
  0x38, 0x1C, 0x00, 0x38, 0x38, 0x00, 0x78, 0x1C, 0x00, 0x38, 0x38, 0x00,
  0xFF, 0xFF, 0xCF, 0xFF, 0xFF, 0xF0, 0x6F, 0xFF, 0xCF, 0xFF, 0xFF, 0xF0,
  0x0C, 0x31, 0xC0, 0x00, 0x00, 0x00, 0x0C, 0x31, 0xC0, 0x00, 0x00, 0x00,
  0x0C, 0x31, 0xC0, 0x00, 0x00, 0x00, 0x0C, 0x31, 0xC1, 0xFF, 0xFF, 0x80,
  0x0C, 0x31, 0xC1, 0xFF, 0xFF, 0x80, 0x0F, 0xFF, 0xC1, 0xC0, 0x03, 0x80,
  0x0F, 0xFF, 0xC1, 0xC0, 0x03, 0x80, 0x0C, 0x31, 0xC1, 0xC0, 0x03, 0x80,
  0x0C, 0x31, 0xC1, 0xFF, 0xFF, 0x80, 0x0C, 0x31, 0xC1, 0xFF, 0xFF, 0x80,
  0x0C, 0x31, 0xC1, 0xFF, 0xFF, 0x80, 0x0C, 0x31, 0xC1, 0xC0, 0x03, 0x80,
  0x0C, 0x31, 0xC1, 0xC0, 0x03, 0x80, 0x1F, 0xFF, 0xC1, 0xC0, 0x03, 0x80,
  0x1F, 0xFF, 0xC1, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0x80,
  0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00,
  0x39, 0x9C, 0xC0, 0x03, 0x80, 0x00, 0x39, 0x9C, 0xCC, 0x03, 0x80, 0x30,
  0x31, 0x8C, 0xEF, 0xFF, 0xFF, 0xF0, 0x31, 0x8C, 0x6F, 0xFF, 0xFF, 0xF0,
  0x71, 0x8C, 0x60, 0x03, 0x80, 0x00, 0x71, 0x8C, 0x60, 0x03, 0x80, 0x00,
  0xE1, 0x8E, 0x70, 0x03, 0x80, 0x00, 0xE1, 0xC0, 0x00, 0x03, 0x80, 0x00,
  0x20, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00,
  0x03, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x07, 0x80, 0x00,
  0x03, 0x80, 0x00, 0x03, 0x80, 0x00, 0x07, 0x00, 0x00, 0x03, 0x80, 0x00,
  0x07, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x0F, 0xFF, 0x0F, 0xFF, 0xFF, 0xE0,
  0x0F, 0xFE, 0x0F, 0xFF, 0xFF, 0xE0, 0x1F, 0xFE, 0x0E, 0x01, 0x80, 0x00,
  0x1C, 0x0E, 0x0E, 0x01, 0x80, 0x00, 0x3C, 0x1C, 0x0E, 0x01, 0x80, 0x00,
  0x38, 0x1C, 0x0E, 0x01, 0x80, 0x00, 0x78, 0x18, 0x0E, 0x7F, 0xFF, 0x80,
  0xF0, 0x38, 0x0E, 0x7F, 0xFF, 0x80, 0xFF, 0xFF, 0xCE, 0x01, 0x83, 0x80,
  0x7F, 0xFF, 0xCE, 0x01, 0x83, 0x80, 0x1C, 0x71, 0xCE, 0x01, 0x83, 0x80,
  0x1C, 0x71, 0xCE, 0x01, 0x83, 0x80, 0x1C, 0x71, 0xCE, 0xFF, 0xFF, 0xF0,
  0x1C, 0x71, 0xCE, 0xFF, 0xFF, 0xF0, 0x1C, 0x71, 0xCE, 0x01, 0x83, 0x80,
  0x1F, 0xFF, 0xCE, 0x01, 0x83, 0x80, 0x1F, 0xFF, 0xCE, 0x01, 0x83, 0x80,
  0x1F, 0xFF, 0xCE, 0x7F, 0x83, 0x80, 0x1C, 0x71, 0xCE, 0x7F, 0xFF, 0x80,
  0x1C, 0x71, 0xCE, 0x7F, 0xFF, 0x80, 0x1C, 0x71, 0xCE, 0x01, 0x80, 0x00,
  0x1C, 0x71, 0xCE, 0x01, 0x80, 0x00, 0x1C, 0x71, 0xCE, 0x41, 0x81, 0x00,
  0x1F, 0xFF, 0xCE, 0xE1, 0xC3, 0x80, 0x1F, 0xFF, 0xCE, 0xE1, 0xC7, 0xC0,
  0x00, 0x00, 0x0C, 0x71, 0xEF, 0x00, 0x00, 0x00, 0x0C, 0x33, 0xFE, 0x00,
  0x20, 0x3B, 0x1C, 0x0F, 0xB8, 0x00, 0x39, 0x9B, 0x9C, 0x1F, 0xBC, 0x00,
  0x39, 0x99, 0x9C, 0x3D, 0x9C, 0x00, 0x31, 0x99, 0x9C, 0x79, 0x8F, 0x00,
  0x31, 0x99, 0xFD, 0xF1, 0x87, 0x80, 0x71, 0x99, 0xFB, 0xC1, 0x83, 0xE0,
  0x71, 0x9C, 0x39, 0x81, 0x81, 0xF0, 0x61, 0xD0, 0x78, 0x83, 0x80, 0xE0,
  0xE0, 0x00, 0x78, 0x1F, 0x80, 0x40, 0x20, 0x00, 0xF0, 0x1F, 0x80, 0x00,
  0x00, 0x00, 0x70, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00,
  0x03, 0xC0, 0x07, 0xFF, 0xFF, 0xC0, 0x03, 0xC0, 0x07, 0xFF, 0xFF, 0xC0,
  0x03, 0x80, 0x07, 0xFF, 0xFF, 0xC0, 0x07, 0x80, 0x00, 0x03, 0x80, 0x00,
  0x07, 0xFF, 0x00, 0x03, 0x80, 0x00, 0x0F, 0xFF, 0x00, 0x03, 0x80, 0x00,
  0x0E, 0x0F, 0x1F, 0xFF, 0xFF, 0xF0, 0x1E, 0x0E, 0x1F, 0xFF, 0xFF, 0xF0,
  0x1C, 0x0E, 0x0F, 0xFF, 0xFF, 0xF0, 0x3C, 0x1C, 0x0C, 0x03, 0x80, 0x70,
  0x78, 0x1C, 0x0C, 0x03, 0x80, 0x70, 0x70, 0x1C, 0x0C, 0x03, 0x80, 0x70,
  0xFF, 0xFF, 0xDC, 0xF3, 0x9E, 0x70, 0x6F, 0xFF, 0xDC, 0xF3, 0x9E, 0x70,
  0x0C, 0x31, 0xC0, 0xF3, 0x9E, 0x00, 0x0C, 0x31, 0xC0, 0x03, 0x80, 0x00,
  0x0C, 0x31, 0xC0, 0x03, 0x80, 0x00, 0x0C, 0x31, 0xC0, 0x03, 0x80, 0x00,
  0x0C, 0x31, 0xC1, 0xF3, 0x9F, 0x00, 0x0F, 0xFF, 0xC1, 0xF3, 0x9F, 0x00,
  0x0F, 0xFF, 0xC0, 0x03, 0x80, 0x00, 0x0F, 0xFF, 0xC0, 0x03, 0x80, 0x00,
  0x0C, 0x31, 0xC0, 0x03, 0x80, 0x00, 0x0C, 0x31, 0xC0, 0x00, 0x00, 0x00,
  0x0C, 0x31, 0xC7, 0xFF, 0xFF, 0xC0, 0x0C, 0x31, 0xC7, 0xFF, 0xFF, 0xC0,
  0x0C, 0x31, 0xC7, 0xFF, 0xFF, 0xC0, 0x0F, 0xFF, 0xC0, 0x00, 0x01, 0xC0,
  0x1F, 0xFF, 0xC0, 0x00, 0x01, 0xC0, 0x1F, 0xFF, 0xC0, 0x00, 0x01, 0xC0,
  0x00, 0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xC0,
  0x38, 0x08, 0xC1, 0xFF, 0xFF, 0xC0, 0x39, 0x98, 0xC1, 0xFF, 0xFF, 0xC0,
  0x31, 0x9C, 0xC0, 0x00, 0x01, 0xC0, 0x31, 0x9C, 0xC0, 0x00, 0x01, 0xC0,
  0x31, 0x9C, 0xE0, 0x00, 0x01, 0xC0, 0x71, 0x8C, 0x67, 0x00, 0x01, 0xC0,
  0x71, 0x8C, 0x67, 0xFF, 0xFF, 0xC0, 0x61, 0x8E, 0x77, 0xFF, 0xFF, 0xC0,
  0xE1, 0xC0, 0x07, 0x80, 0x01, 0xC0, 0xE1, 0x80, 0x00, 0x00, 0x01, 0xC0,
  0x00, 0x1E, 0x10, 0x06, 0x00, 0x00, 0x06, 0x1E, 0x1C, 0x0F, 0x00, 0x00,
  0x07, 0x1C, 0x38, 0x0E, 0x00, 0x00, 0x03, 0x1C, 0x30, 0x0E, 0x00, 0x00,
  0x03, 0x9C, 0x70, 0x1C, 0x00, 0x00, 0x01, 0x1C, 0x20, 0x1F, 0xFF, 0xC0,
  0x00, 0x1C, 0x00, 0x3F, 0xFF, 0xC0, 0x1F, 0xFF, 0xFE, 0x38, 0x0E, 0x00,
  0x1F, 0xFF, 0xFE, 0x78, 0x0E, 0x00, 0x1C, 0x1C, 0x0E, 0xF8, 0x0C, 0x00,
  0x1C, 0x1C, 0x0F, 0xFC, 0x1C, 0x00, 0x1C, 0xDC, 0x0E, 0xDE, 0x1C, 0x00,
  0x1C, 0xDC, 0xCE, 0x0F, 0x18, 0x00, 0x1D, 0xDC, 0xEE, 0x07, 0xB8, 0x00,
  0x1D, 0x9C, 0xEE, 0x03, 0xF0, 0x00, 0x1D, 0x9C, 0x6E, 0x01, 0xF0, 0x00,
  0x1C, 0x9C, 0x4E, 0x01, 0xF8, 0x00, 0x1C, 0x1C, 0x0E, 0x07, 0xFE, 0x00,
  0x1C, 0x1C, 0x0E, 0xFF, 0x1F, 0xF0, 0x1C, 0x1C, 0x0E, 0x7C, 0x0F, 0xE0,
  0x00, 0x02, 0x00, 0x30, 0x03, 0xC0, 0x00, 0x07, 0x80, 0x00, 0x00, 0x40,
  0x00, 0x0F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0xFF, 0xC0, 0x00,
  0x00, 0x3F, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x3C, 0x00, 0x00,
  0x03, 0xE0, 0x00, 0x78, 0x00, 0x00, 0x1F, 0xFF, 0xFF, 0xFF, 0xFC, 0x00,
  0x3F, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x1C, 0xE0, 0x07, 0x00, 0x1C, 0x00,
  0x10, 0xE0, 0x07, 0x00, 0x1C, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFC, 0x00,
  0x00, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFC, 0x00,
  0x01, 0xE0, 0x07, 0x00, 0x1C, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFC, 0x00,
  0x01, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0C, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x80, 0x38, 0x07, 0x80,
  0x1C, 0x03, 0xC0, 0x38, 0x03, 0xC0, 0x7C, 0x01, 0xC0, 0x1C, 0x01, 0xE0,
  0xF8, 0x01, 0xE0, 0x1C, 0x00, 0xF0, 0x30, 0x00, 0x80, 0x0C, 0x00, 0x40
};

//!< This struct will be read by the CPU only
static const nema_glyph_t g_sSimhei36pt1bGlyphs0[] = {
  {     0,  45,  48,    2,  -37 },   // 0x9C40
  {   258,  45,  48,    2,  -38 },   // 0x9C41
  {   516,  44,  48,    2,  -38 },   // 0x9C42
  {   780,  44,  48,    2,  -38 },   // 0x9C43
  {  1038,  44,  48,    2,  -38 },   // 0x9C44
  {  1302,  44,  48,    2,  -39 },   // 0x9C45
  {  1572,  44,  48,    2,  -38 },   // 0x9C46
  {  1836,  44,  48,    2,  -38 },   // 0x9C47
  {  2100,  44,  48,    2,  -39 },   // 0x9C48
  {  2364,  44,  48,    2,  -38 },   // 0x9C49
  {  2628,   0,   0,    0,    0 }
};

//!< This struct will be read by the CPU only
static const nema_font_range_t g_sSimhei36pt1bRanges[] = {
  {0x00009c40, 0x00009c49, g_sSimhei36pt1bGlyphs0},
  {0, 0, NULL}
};

//!< This struct will be read by the CPU only
nema_font_t g_sSimhei36pt1b = {
  {
    .base_virt = (void *) g_ui8Simhei36pt1bBitmaps,
    .base_phys = (uintptr_t) g_ui8Simhei36pt1bBitmaps,
    .size      = (int32_t) sizeof(g_ui8Simhei36pt1bBitmaps)
  },
  g_sSimhei36pt1bRanges,
  (int32_t)sizeof(g_ui8Simhei36pt1bBitmaps),
  g_ui8Simhei36pt1bBitmaps,
  0,
  24, 55, 40, 1
};
#endif // SIMHEI36PT1B_C
