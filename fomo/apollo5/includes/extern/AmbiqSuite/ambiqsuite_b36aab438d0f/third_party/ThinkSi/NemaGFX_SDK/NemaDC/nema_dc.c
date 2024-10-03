/* TSI 2023.gen */
// -----------------------------------------------------------------------------
// Copyright (c) 2008-23 Think Silicon Single Member PC
// Think Silicon Single Member PC Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon Single
//  Member PC The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by using the
//  same degree of care, but not less then a reasonable degree of care, as the
//  receiver uses to protect receiver's own Confidential Information. The entire
//  notice must be reproduced on all authorized copies and copies may only be
//  made to the extent permitted by a licensing agreement from Think Silicon
//  Single Member PC.
//
//  The software/data is provided 'as is', without warranty of any kind,
//  expressed or implied, including but not limited to the warranties of
//  merchantability, fitness for a particular purpose and noninfringement. In no
//  event shall Think Silicon Single Member PC be liable for any claim, damages
//  or other liability, whether in an action of contract, tort or otherwise,
//  arising from, out of or in connection with the software.
//
//  For further information please contact:
//
//                    Think Silicon Single Member PC
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#include "nema_dc_hal.h"
#include "nema_dc_regs.h"
#include "nema_dc.h"
#include "nema_dc_intern.h"

#ifndef NEMADC_SKIP_RATIO
#define NEMADC_SKIP_RATIO 1.87f
#endif

#define XY16TOREG32(x, y) ( (((unsigned)(x)) << 16) | (((unsigned)(y)) & 0xffffU))
#define CEIL(f)  ((int)(f) + ( (int)(f) < (f) ))
#define ABS(a)    (((a)< 0 )?(-(a)):(a))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
//#define NEMADC_PRINT printf

static nemadc_display_t     nemadc_display;
static uint32_t nemadc_config, nemadc_colmod;

inline static void nemadc_read_display_setup(void) {
    //read current configuration
    uint32_t tmp;

    tmp = nemadc_reg_read(NEMADC_REG_RESXY       );
    nemadc_display.resx = tmp >> 16;
    nemadc_display.resy = tmp & 0xffffU;
    tmp = nemadc_reg_read(NEMADC_REG_FRONTPORCHXY);
    nemadc_display.fpx  = tmp >> 16;
    nemadc_display.fpy  = tmp & 0xffffU;
    tmp = nemadc_reg_read(NEMADC_REG_BLANKINGXY  );
    nemadc_display.blx  = tmp >> 16;
    nemadc_display.bly  = tmp & 0xffffU;
    tmp = nemadc_reg_read(NEMADC_REG_BACKPORCHXY );
    nemadc_display.bpx  = tmp >> 16;
    nemadc_display.bpy  = tmp & 0xffffU;
}

// -------------------------------- UTILS --------------------------------------
// Function that returns the size in bytes for each display mode
//-------------------------------------------------------------------------------
static int
nemadc_format_size(uint32_t format)
{
    switch(format&0x3f) {
    case NEMADC_RGBA8888:
    case NEMADC_ARGB8888:
    case NEMADC_ABGR8888:
    case NEMADC_BGRA8888: return 4;
    case NEMADC_BGR24   :
    case NEMADC_RGB24   : return 3;
    case NEMADC_RGBA4444:
    case NEMADC_BGRA4444:
    case NEMADC_ARGB4444:
    case NEMADC_ABGR4444:
    case NEMADC_RGBA5551:
    case NEMADC_BGRA5551:
    case NEMADC_ARGB1555:
    case NEMADC_ABGR1555:
    case NEMADC_RGB565  :
    case NEMADC_BGR565  :
    case NEMADC_AL88    : return 2;
    default             : return 1;
    }
}


static int
nemadc_addr_offsetX(uint32_t format, int sx)
{
    switch(format&0x3f) {
    case NEMADC_L1      : return sx/8;
    case NEMADC_L4      : return sx/2;
    case NEMADC_TSC4    : return sx*2;
    case NEMADC_TSC6    :
    case NEMADC_TSC6A   :
    case NEMADC_TSC6AP  :
    case NEMADC_TSC12   :
    case NEMADC_TSC12A  : return sx*3;
    default             : return sx*nemadc_format_size(format);
    }
}


static int
nemadc_addr_offsetY(uint32_t format, int sy, int stride)
{
    switch(format&0x3f) {
    case NEMADC_TSC4    :
    case NEMADC_TSC6    :
    case NEMADC_TSC6A   : return sy*stride/4;
    case NEMADC_TSC6AP  : return sy*stride/4;
    case NEMADC_TSC12   :
    case NEMADC_TSC12A  : return sy*stride/2;
    default             : return sy*stride;
    }
}

static int
nemadc_adjust_sx(uint32_t format, int sx)
{
    switch(format&0x3f) {
    case NEMADC_L1      : return sx & (~7);
    case NEMADC_V_YUV420:
    case NEMADC_L4      : return sx & (~1);
    case NEMADC_TSC4    :
    case NEMADC_TSC6    :
    case NEMADC_TSC6A   :
    case NEMADC_TSC6AP  :
    case NEMADC_TSC12   :
    case NEMADC_TSC12A  : return sx & (~3);
    default             : return sx;
    }
}

static int
nemadc_adjust_sy(uint32_t format, int sy)
{
    switch(format&0x3f) {
    case NEMADC_TSC4    :
    case NEMADC_TSC6    :
    case NEMADC_TSC6A   : return sy & (~3);
    case NEMADC_TSC6AP  : return sy & (~3);
    case NEMADC_V_YUV420:
    case NEMADC_TSC12   :
    case NEMADC_TSC12A  : return sy & (~1);
    default             : return sy;
    }
}

//-------------------------------------------------------------------------------
int
nemadc_stride_size(uint32_t format, int width)
{
    int stride;
    switch(format & 0x3f) {
    case NEMADC_L1     : stride=(width  +7)/8;   break;
    case NEMADC_L4     : stride=(width*4+7)/8;   break;
    case NEMADC_TSC4   : stride=width*2;         break;
    case NEMADC_TSC6   :
    case NEMADC_TSC6A  :
    case NEMADC_TSC6AP :
    case NEMADC_TSC12  :
    case NEMADC_TSC12A : stride=width*3;         break;
    default            : stride=width*nemadc_format_size(format); break;

    }
    return stride;
}

int
nemadc_valid_width(uint32_t format, int width)
{
    switch(format&0x3f) {
    case NEMADC_RGBA4444:
    case NEMADC_BGRA4444:
    case NEMADC_ARGB4444:
    case NEMADC_ABGR4444:
    case NEMADC_RGBA5551:
    case NEMADC_BGRA5551:
    case NEMADC_ARGB1555:
    case NEMADC_ABGR1555:
    case NEMADC_RGB565  :
    case NEMADC_BGR565  :
    case NEMADC_AL88    : return (width+1)&(~1);
    case NEMADC_TSC4    :
    case NEMADC_TSC6    :
    case NEMADC_TSC6A   :
    case NEMADC_TSC6AP  :
    case NEMADC_TSC12   :
    case NEMADC_TSC12A  :
    case NEMADC_RGB24   :
    case NEMADC_BGR24   :
    case NEMADC_AL44    :
    case NEMADC_RGB332  :
    case NEMADC_RGBA2222:
    case NEMADC_BGRA2222:
    case NEMADC_ARGB2222:
    case NEMADC_ABGR2222:
    case NEMADC_A8      :
    case NEMADC_L8      : return (width+3)&(~3);
    case NEMADC_L4      : return (width+7)&(~7);
    case NEMADC_L1      : return (width+31)&(~31);
    default             : return width;
    }
}


uint32_t nemadc_get_config(void)
{
    return nemadc_reg_read(NEMADC_REG_CONFIG);;
}

uint32_t nemadc_get_crc(void)
{
    return nemadc_reg_read(NEMADC_REG_CRC);
}

uint32_t nemadc_get_ipversion(void)
{
    // IP Version register was included after 22.03.01 release at the time that FlipY feature was included.
    if(NEMADC_COLMOD_FLIPY & nemadc_colmod){
        return nemadc_reg_read(NEMADC_REG_IP_VERSION);
    }else{
        return 0x00210000U;
    }
}

static uintptr_t
nemadc_addr_offset_flipXY(nemadc_layer_t *layer, uint32_t resx_final, uint32_t resy_final, uint32_t addr_end_visible){

    uintptr_t adj_baseaddr_phys = 0;

    uint32_t layer_resx_minus_startx_upper = ( (layer->resx+layer->startx)%4==0)             ?   (layer->resx+layer->startx)                : ( ( (layer->resx+layer->startx) |3 ) +1 );
    uint32_t layer_resx_minus_startx_lower = (((layer->resx+layer->startx)-resx_final)%4==0) ? ( (layer->resx+layer->startx) - resx_final ) : ( ( ( (layer->resx+layer->startx) - resx_final ) >> 2 ) << 2 );

    uint32_t addr   = layer->baseaddr_phys;
    int      resx   = layer->resx;
    int      resy   = layer->resy;
    int    stride   = ( layer->stride < 0) ? nemadc_stride_size(layer->format, layer->resx) : layer->stride;
    int    startx   = layer->startx;
    int    starty   = layer->starty;
    int output_resx = (int)nemadc_display.resx;

    int  resx_minus_starty_round;
    float format_size  = 0;
    float format_bytes = 0;

    int tsc4_format  = ((layer->format & 0x3f)==NEMADC_TSC4) ;
    int tsc6_format  = ((layer->format & 0x3f)==NEMADC_TSC6A) | ((layer->format & 0x3f)==NEMADC_TSC6) | ((layer->format & 0x3f)==NEMADC_TSC6AP);
    int tsc12_format = ((layer->format & 0x3f)==NEMADC_TSC12) | ((layer->format & 0x3f)==NEMADC_TSC12A) ;
    int tsc_format   = (tsc4_format | tsc6_format | tsc12_format) ;

    switch((layer->format & 0x3f)) {
        case NEMADC_TSC4   : format_size  = 2;                                 break;

        case NEMADC_TSC6   :
        case NEMADC_TSC6A  :
        case NEMADC_TSC6AP :
        case NEMADC_TSC12  :
        case NEMADC_TSC12A : format_size  = 3;                                 break;

        default            : format_size  = nemadc_format_size(layer->format);
                             format_bytes = format_size;                       break;
    }

    // Keep the layer's resx if ...
    if (( resx > output_resx ) && ( startx >= 0 ) && ( !( ( output_resx - startx > 16   ) && ( format_bytes == 4 ) ) && // is RGBA 32-bits and output resx - startx is not bigger than 16
                                                       !( ( output_resx - startx > 2*16 ) && ( format_bytes == 2 ) )    // is RGBA 16-bits and output resx - startx is not bigger than 16
                                                     )
       )
    {
        resx = output_resx;
    }

    //-------------------------------------------------------------------
    float burst = 16.0f;

    if(tsc_format)
    {
        if(startx < 0) {
            burst = (float)( ( layer_resx_minus_startx_upper - layer_resx_minus_startx_lower ) / 4 ) * (float)(format_size);
        } else {
            if (resx_final%4==0) {
                burst =  (float)( (resx_final)/4 )*(float)format_size;
            }
            else {
                burst =  (float)(( (resx_final|3)+1)/4 )*(float)format_size;
            }
        }
    }
    else
        burst = ( ( resx_final * (float)format_size ) / (float)(4) );

    //-------------------------------------------------------------------
    // Only FlipY Enabled ..
    if( ( layer->flipy_en == 1 ) && ( layer->flipx_en == 0 )){

        // ..and color mode is TSC* ...
        if( tsc_format ){

            // ...and StartY has negative values ....
            if (starty < 0){

                if( (ABS(starty)%4 != 0)&&!tsc12_format)
                {
                    // ....and starty is Not multiple of 4.
                    resx_minus_starty_round = ((resy+starty) >> 2) << 2;
                    adj_baseaddr_phys = addr + ((resx_minus_starty_round/4)*stride);
                }
                else if((ABS(starty)%2 != 0)&&tsc12_format)
                {
                    resx_minus_starty_round = ((resy+starty) >> 1) << 1;
                    adj_baseaddr_phys = addr + ((resx_minus_starty_round/2)*stride);
                }
                else if((ABS(starty)%4 == 0)&&!tsc12_format)
                {
                        // ....and starty is multiple of 4.
                        adj_baseaddr_phys = addr + (((resy+starty)/4)*stride) - (stride);
                }
                else if ((ABS(starty)%2 == 0)&&tsc12_format)
                {
                    adj_baseaddr_phys = addr + (((resy+starty)/2)*stride) - (stride);
                }
            }
            else{
                if(tsc6_format | tsc4_format)
                {
                    // ...and StartY has positive values.
                    adj_baseaddr_phys = addr + ((resy/4)*stride) - (stride);
                }
                else
                {
                    // ...and StartY has positive values.
                    adj_baseaddr_phys = addr + ((resy/2)*stride) - (stride);
                }
            }

        } else{
            // TODO: check with an image with different resx/y and output resolution smaller than that.
            // ..and color mode is Other Than TSC*
            adj_baseaddr_phys = addr + (resy*stride) - (stride);
        }

        return adj_baseaddr_phys;
    }


    //-------------------------------------------------------------------
    // Only FlipX Enabled ..
    if( ( layer->flipy_en == 0 ) && ( layer->flipx_en == 1 ) ){

        uint8_t FB = 16;

        // ..Calculate FB value ...
        if( ( (uint16_t)burst >= 16 )){
            // Keep FB=16 not need to change, unless...
            // This is for negative startx of 8-bit color formats.
            //   (startx neg) && (-----------) && ( --8-bit format--  or  --24-bit format--)
            if ( (startx < 0) && (-startx % 4) && ( (format_bytes==1) || (format_bytes==3) ) ){
                FB = FB-1;
            }
            if ( (startx < 0) && (-startx % 2) && ( (format_bytes==2)) ){
                FB = FB-1;
            }
        }else{
            // -- Adjust FB --
            FB = CEIL(burst);

            // This check is for negative startx of 8-bit color formats.
            //   (------------------- || -----------------) && (startx neg) && ( 8-bit format )
            if ( (startx < 0) && (format_bytes==1)  && (((burst>15.0) && (resx_final!=(uint32_t)output_resx)) || ((burst>15.0)&&(resx_final==(uint32_t)output_resx) && ((-startx % 4)!=0))))
            {
                FB = FB-1;
            }

            // This check is for negative startx of 24-bit color formats.
            //   (------------------- || -----------------) && (startx neg) && (24-bit format )
            if ( (startx < 0) && (format_bytes==3) && (((burst==15.75) && (resx_final!=(uint32_t)output_resx)) || ((burst==15.75)&&(resx_final==(uint32_t)output_resx) && ((-startx % 4)!=0))))
            {
                FB = FB-1;
            }

            // This check is for negative startx of 16-bit color formats.
            //   (------------------- || -----------------) && (startx neg) && (16-bit format )
            if ( (startx < 0) && (format_bytes==2) && (((burst==15.5) && (resx_final!=(uint32_t)output_resx)) || ((burst==15.5)&&(resx_final==(uint32_t)output_resx) && ((-startx % 2)!=0))))
            {
                FB = FB-1;
            }

            // This check is for TSC4 color format.
            // ( ---TSC---- && ( ----------------- ) && (------) )
            if ( tsc_format && (FB%(int)format_size) && (FB!=16) ){
                FB = FB + ( (int)(format_size) - (FB%(int)format_size) );
            }
        }

        // Calculation of baseaddress when flipX is enabled ..
        //----------------------------------------------------------------------------
        // ..when TSC color formats and startx < 0
        if((startx < 0) && tsc_format){

            if( (FB <= 16) ){

                if( ABS(startx)%4 != 0){
                    addr = addr + (layer_resx_minus_startx_upper)*(int)(format_size);
                } else {
                    addr = addr + ( (layer->resx+layer->startx) * (int)(format_size) );
                }
            }

            // This adjustment applies only to tsc with startx < 0
            adj_baseaddr_phys = addr - (4*FB);

        }else{

            // This adjustment applies to all color formats and tsc with startx > 0
            adj_baseaddr_phys = addr + (stride) - (4*FB);
        }
        //----------------------------------------------------------------------------
        return adj_baseaddr_phys;
    }

    // FlipY Enabled and FlipX Enabled
    if( ( layer->flipy_en == 1 ) && ( layer->flipx_en == 1 ) ){

        uint8_t FB = 16;

        /*****************FlipX calculations*****************/
        // ..Calculate FB value ...
        if( ( (uint16_t)burst >= 16 )){
            // Keep FB=16 not need to change, unless...
            // This is for negative startx of 8-bit color formats.
            //   (startx neg) && (-----------) && ( --8-bit format--  or  --24-bit format--)
            if ( (startx < 0) && (-startx % 4) && ( (format_bytes==1) || (format_bytes==3) ) ){
                FB = FB-1;
            }
            if ( (startx < 0) && (-startx % 2) && ( (format_bytes==2)) ){
                FB = FB-1;
            }
        }else{
            // -- Adjust FB --
            FB = CEIL(burst);

             // This check is for negative startx of 8-bit color formats.
            //   (------------------- || -----------------) && (startx neg) && ( 8-bit format )
            if ( (startx < 0) && (format_bytes==1)  && (((burst>15.0) && (resx_final!=(uint32_t)output_resx)) || ((burst>15.0)&&(resx_final==(uint32_t)output_resx) && ((-startx % 4)!=0))))
            {
                FB = FB-1;
            }

            // This check is for negative startx of 24-bit color formats.
            //   (------------------- || -----------------) && (startx neg) && (24-bit format )
            if ( (startx < 0) && (format_bytes==3) && (((burst==15.75) && (resx_final!=(uint32_t)output_resx)) || ((burst==15.75)&&(resx_final==(uint32_t)output_resx) && ((-startx % 4)!=0))))
            {
                FB = FB-1;
            }

            // This check is for negative startx of 16-bit color formats.
            //   (------------------- || -----------------) && (startx neg) && (16-bit format )
            if ( (startx < 0) && (format_bytes==2) && (((burst==15.5) && (resx_final!=(uint32_t)output_resx)) || ((burst==15.5)&&(resx_final==(uint32_t)output_resx) && ((-startx % 2)!=0))))
            {
                FB = FB-1;
            }

            // This check is for TSC4 color format.
            // ( ---TSC---- && ( ----------------- ) && (------) )
            if ( tsc_format && (FB%(int)format_size) && (FB!=16) ){
                FB = FB + ( (int)(format_size) - (FB%(int)format_size) );
            }
        }
        /*****************FlipX calculations*****************/

        /*****************FlipY calculations*****************/
        // ..and color mode is TSC* ...
        if( tsc_format ){
            // If FlipY Enabled and FlipX Enabled and color mode is TSC*
            if( startx < 0 ){

                if( (FB <= 16) ){

                    if( ABS(startx)%4 != 0){
                        addr = addr + (layer_resx_minus_startx_upper)*(int)(format_size);
                    } else {
                        addr = addr + ( (layer->resx+layer->startx) * (int)(format_size) );
                    }
                }

                // This adjustment applies only to tsc with startx < 0
                addr = addr - (4*FB);

            }else{

                // This adjustment applies to all color formats and tsc with startx > 0
                addr = addr + (stride) - (4*FB);
            }
            //-------------------------------------------------------
            if ( starty < 0 ){

                if( (ABS(starty)%4 != 0)&&!tsc12_format)
                {
                    // ....and starty is Not multiple of 4.
                    resx_minus_starty_round = ((resy+starty) >> 2) << 2;
                    adj_baseaddr_phys = addr + ((resx_minus_starty_round/4)*stride);
                }
                else if((ABS(starty)%2 != 0)&&tsc12_format)
                {
                    resx_minus_starty_round = ((resy+starty) >> 1) << 1;
                    adj_baseaddr_phys = addr + ((resx_minus_starty_round/2)*stride);
                }
                else if((ABS(starty)%4 == 0)&&!tsc12_format)
                {
                        // ....and starty is multiple of 4.
                        adj_baseaddr_phys = addr + (((resy+starty)/4)*stride) - (stride);
                }
                else if ((ABS(starty)%2 == 0)&&tsc12_format)
                {
                    adj_baseaddr_phys = addr + (((resy+starty)/2)*stride) - (stride);
                }
            }
            else{
                if(tsc6_format | tsc4_format)
                {
                    // ...and StartY has positive values.
                    adj_baseaddr_phys = addr + ((resy/4)*stride) - (stride);
                }
                else
                {
                    // ...and StartY has positive values.
                    adj_baseaddr_phys = addr + ((resy/2)*stride) - (stride);
                }
            }
        }
        else{
            adj_baseaddr_phys = addr + (resy*stride) - (4*FB);
        }
        /*****************FlipY calculations*****************/
        return adj_baseaddr_phys;
    }

    return adj_baseaddr_phys;
}

int nemadc_init(void)
{
    int ret = 0;
    ret = nemadc_sys_init();
    if (ret != 0) {
        return ret;
    }

    if ( nemadc_reg_read(NEMADC_REG_IDREG) != NEMADC_MAGIC ) {
        return -1;
    }

    nemadc_config = nemadc_get_config();
    nemadc_colmod = nemadc_get_col_mode();

    nemadc_read_display_setup();

    return 0;
}

void nemadc_set_bgcolor(uint32_t rgba)
{
    nemadc_reg_write(NEMADC_REG_BGCOLOR, rgba);
}


void nemadc_timing(int resx, int fpx, int blx, int bpx,
                   int resy, int fpy, int bly, int bpy)
//-----------------------------------------------------------------------
{
    nemadc_display.resx = resx; nemadc_display.resy = resy;
    nemadc_display.fpx  = fpx;  nemadc_display.fpy  = fpy;
    nemadc_display.blx  = blx;  nemadc_display.bly  = bly;
    nemadc_display.bpx  = bpx;  nemadc_display.bpy  = bpy;

    int dc_fpx = resx+fpx; int dc_blx = dc_fpx+blx; int dc_bpx = dc_blx + bpx;
    int dc_fpy = resy+fpy; int dc_bly = dc_fpy+bly; int dc_bpy = dc_bly + bpy;

    nemadc_reg_write(NEMADC_REG_RESXY       , XY16TOREG32(resx  , resy        ));
    nemadc_reg_write(NEMADC_REG_FRONTPORCHXY, XY16TOREG32(dc_fpx, dc_fpy      ));
    nemadc_reg_write(NEMADC_REG_BLANKINGXY  , XY16TOREG32(dc_blx, dc_bly      ));
    nemadc_reg_write(NEMADC_REG_BACKPORCHXY , XY16TOREG32(dc_bpx, dc_bpy      ));
    nemadc_reg_write(NEMADC_REG_STARTXY     , XY16TOREG32(resx+fpx, resy+fpy-1));
}
//-----------------------------------------------------------------------

void nemadc_set_mode(int mode)
{
    nemadc_reg_write(NEMADC_REG_MODE, mode);
    nemadc_reg_write(NEMADC_REG_PLAY, 0);
}

uint32_t nemadc_get_status (void)
{
    return nemadc_reg_read(NEMADC_REG_STATUS);
}

void nemadc_request_vsync_non_blocking(void)
{
    nemadc_reg_write(NEMADC_REG_INTERRUPT, 1);
}


#define __floor(f) ((int)(f) - ( (int)(f) > (f) ))

void nemadc_set_layer_no_scale(int layer_no, nemadc_layer_t *layer)
{

    // Preserve backwards combatibility now that we mask format argument.
    // extra_bits argument is also masked to apply to certain bits.
    uint32_t extra_bits = (layer->extra_bits | layer->format ) & 0x74000020;

    int stride   = ( layer->stride < 0) ? nemadc_stride_size(layer->format, layer->resx) : layer->stride;
    int u_stride = layer->u_stride;
    int v_stride = layer->v_stride;
    int u_base   = layer->u_base;
    int v_base   = layer->v_base;

    if ( (layer->format & 0x3f) == NEMADC_V_YUV420 ) {
        if (u_stride < 0 || v_stride < 0) {
            u_stride = (layer->resx+1)>>1;
            v_stride = (layer->resx+1)>>1;
        }

        if ( layer->u_base == 0) {
            u_base = layer->baseaddr_phys +  layer->resx*layer->resy;
            v_base = u_base               + (layer->resx*layer->resy)/4;
        }
    }

    uint32_t  addr  = layer->baseaddr_phys;
    uint32_t  resx  = layer->resx;
    uint32_t  resy  = layer->resy;
    int       sx    = layer->startx;
    int       sy    = layer->starty;
    int       szx   = resx;
    int       szy   = resy;
    int  round_sx   = 0;
    int  round_sy   = 0;
    int       csx   = 0;
    int       csy   = 0;
    int tsc_format  = ((layer->format & 0x3f) == NEMADC_TSC4) | ((layer->format & 0x3f) == NEMADC_TSC6) | ((layer->format & 0x3f) == NEMADC_TSC6A) | ((layer->format & 0x3f) == NEMADC_TSC6AP) | ((layer->format & 0x3f) == NEMADC_TSC12) | ((layer->format & 0x3f) == NEMADC_TSC12A) ;
    int tsc12_format= ((layer->format & 0x3f) == NEMADC_TSC12) | ((layer->format & 0x3f) == NEMADC_TSC12A) ;
    //find visible area
    //-----------------------------------------
    int32_t ox_min = 0, ox_max = 0, lx_min = 0, lx_max = 0;
    int32_t oy_min = 0, oy_max = 0, ly_min = 0, ly_max = 0;

    int32_t oresx  = (int)nemadc_display.resx;
    int32_t oresy  = (int)nemadc_display.resy;
    int32_t lresx  = (int)layer->resx;
    int32_t lresy  = (int)layer->resy;
    int32_t startx = (int)layer->startx;
    int32_t starty = (int)layer->starty;

    uint32_t resx_final = 0;
    uint32_t resy_final = 0;

    // calculating the visible layer part resolution (x,y)
    if(startx>=0 && starty>=0)
    {
        startx = ABS(startx);
        starty = ABS(starty);

        // visible output rectangle
        ox_min = 0;
        oy_min = 0;
        ox_max = oresx;
        oy_max = oresy;

        // layer rectangle
        lx_min = startx;
        ly_min = starty;
        lx_max = startx+lresx;
        ly_max = starty+lresy;
    }
    else if(startx>=0 && starty<0)
    {
        startx = ABS(startx);
        starty = ABS(starty);

        // visible output rectangle
        ox_min = 0;
        oy_min = starty;
        ox_max = oresx;
        oy_max = starty+oresy;

        // layer rectangle
        lx_min = startx;
        ly_min = 0;
        lx_max = startx+lresx;
        ly_max = lresy;
    }
    else if(startx<0 && starty>=0)
    {
        startx = ABS(startx);
        starty = ABS(starty);

        // visible output rectangle
        ox_min = startx;
        oy_min = 0;
        ox_max = startx+oresx;
        oy_max = oresy;

        // layer rectangle
        lx_min = 0;
        ly_min = starty;
        lx_max = lresx;
        ly_max = starty+lresy;
    }
    else if(startx<0 && starty<0)
    {
        startx = ABS(startx);
        starty = ABS(starty);

        // visible output rectangle
        ox_min = startx;
        oy_min = starty;
        ox_max = startx+oresx;
        oy_max = starty+oresy;

        // layer rectangle
        lx_min = 0;
        ly_min = 0;
        lx_max = lresx;
        ly_max = lresy;
    }

    resx_final = MIN(ox_max,lx_max) - MAX(ox_min,lx_min);
    resy_final = MIN(oy_max,ly_max) - MAX(oy_min,ly_min);

    uint32_t addr_end_visible= MIN(ox_max,lx_max);
    //--(-(FlipY is set ) or (-(FlipX is set)- && ( ------- FlipX module is enabled on Hardware -------- )-)-)--
    if( ( ( layer->flipy_en == 1 ) || ( ( layer->flipx_en == 1 ) && ( NEMADC_COLMOD_LAYER_FLIPX(layer_no) & nemadc_colmod ) ) ) ){
        addr = (uint32_t)nemadc_addr_offset_flipXY(layer,resx_final,resy_final,addr_end_visible);
    }

    // Negative startX and ( not TSC ||  TSC with absolute startx mult of 4 and flipX disable )
    // |---------------------------|   |------------------------------------------------------|
    if ( (sx < 0) && ( !tsc_format   || (tsc_format && ((ABS(sx)%4)==0) && ( layer->flipx_en==0 ) ) ) ) {
        if( layer->flipx_en == 1 ) {
            addr += (uint32_t)nemadc_addr_offsetX(layer->format, sx);
        } else{
            addr -= (uint32_t)nemadc_addr_offsetX(layer->format, sx);
        }
        sx    = nemadc_adjust_sx(layer->format, sx);

        if ( (layer->format & 0x3f) == NEMADC_V_YUV420 ) {
            u_base -= sx/2;
            v_base -= sx/2;
        }
        csx = ABS(sx);
        sx    = 0;

    }
    // Negative startX and TSC and [ ( absolute startx isn't mult of 4 and flipX disable ) or ( flipX enable ) ]
    else if( ( sx < 0 ) && tsc_format && ( ( (ABS(sx)%4!=0) && ( layer->flipx_en==0 ) ) || ( layer->flipx_en==1 )) ) {
        // This case is for no flipX with negative values on startX
        if( layer->flipx_en == 0 ) {
            round_sx = ( (ABS(sx) >> 2) << 2);
            addr    -= (uint32_t)nemadc_addr_offsetX(layer->format, -round_sx);
        }
        csx   = ABS(sx);
        sx    = 0;
    }

    // Negative startY and ( not TSC ||  TSC with absolute starty mult of 4 and flipY disable )
    // |---------------------------|   |------------------------------------------------------|
    if ( (sy < 0) && ( !tsc_format   || ( tsc_format && ((ABS(sy)%4)==0) && !tsc12_format && ( layer->flipy_en==0 ) ) ) ) {
        sy    = nemadc_adjust_sy(layer->format, sy);

        if(layer->flipy_en == 1) {
            addr += (uint32_t)nemadc_addr_offsetY(layer->format, sy, stride);
        } else {
            addr -= (uint32_t)nemadc_addr_offsetY(layer->format, sy, stride);
        }
        if ( (layer->format & 0x3f) == NEMADC_V_YUV420 ) {
            u_base -= (sy*u_stride) / 2;
            v_base -= (sy*v_stride) / 2;
        }

        sy    = 0;
    }
    // Negative startY and ( not TSC ||  TSC12 with absolute starty mult of 2 and flipY disable )
    // |---------------------------|   |------------------------------------------------------|
    else if ( (sy < 0) && ( !tsc_format   || ( tsc_format && ((ABS(sy)%2)==0) && tsc12_format && ( layer->flipy_en==0 ) ) ) ) {
        sy    = nemadc_adjust_sy(layer->format, sy);

        if(layer->flipy_en == 1) {
            addr += (uint32_t)nemadc_addr_offsetY(layer->format, sy, stride);
        } else {
            addr -= (uint32_t)nemadc_addr_offsetY(layer->format, sy, stride);
        }

        sy    = 0;
    }
    // Negative startY and TSC and [ ( absolute startY isn't mult of 4 and flipY disable ) or ( flipY enable ) ]
    else if( (sy < 0) && tsc_format && ( ( ((ABS(sy)%4)!=0) && (layer->flipy_en==0) && !tsc12_format ) || ((layer->flipy_en==1)&&!tsc12_format) ) ) {
        // This case is for no flipY with negative values on startY
        if(layer->flipy_en == 0 ){
            round_sy = ((ABS(sy) >> 2) << 2);
            addr += ((round_sy/4) * stride);
        }
        csy   = ABS(sy);
        sy    = 0;
    }
    // Negative startY and TSC12 and [ ( absolute startY isn't mult of 2 and flipY disable ) or ( flipY enable ) ]
    else if( (sy < 0)  && ( ( ((ABS(sy)%2)!=0) && (layer->flipy_en==0) && tsc12_format ) || ((layer->flipy_en==1)&&tsc12_format) ) ) {
        // This case is for no flipY with negative values on startY
        if(layer->flipy_en == 0 ){
            round_sy = ((ABS(sy) >> 1) << 1);
            addr += ((round_sy/2) * stride);
        }
        csy   = ABS(sy);
        sy    = 0;
    }

    resx = resx_final;
    resy = resy_final;
    szx  = resx_final;
    szy  = resy_final;

#ifdef HW_FIX_PREFETCH
    uint32_t limit;
    uint32_t bytes_to_prefetch = 1024;
    uint32_t pixel_number = ( bytes_to_prefetch / nemadc_format_size(layer->format) );

    switch((layer->format)&0x3f) {
        case NEMADC_RGBA8888:
        case NEMADC_ARGB8888:
        case NEMADC_ABGR8888:
        case NEMADC_BGRA8888: limit = 2; break;
        case NEMADC_RGB24   : limit = 3; break;
        case NEMADC_RGBA5551:
        case NEMADC_RGBA4444:
        case NEMADC_ARGB4444:
        case NEMADC_RGB565  : limit = 5; break;
        case NEMADC_TSC4    : limit = resy*6; pixel_number = 2112; break;
        case NEMADC_TSC6    :
        case NEMADC_TSC6A   :
        case NEMADC_TSC6AP  :
        case NEMADC_TSC12   :
        case NEMADC_TSC12A  : limit = resy*3; pixel_number = 1430; break;
        default             : limit = 9; break;
    }

    if ((resy < limit) && (pixel_number > resx*resy)){
        resy = limit;
    }
#endif

    nemadc_reg_write(NEMADC_REG_LAYER_BASEADDR         (layer_no),  addr);
    nemadc_reg_write(NEMADC_REG_LAYER_STARTXY          (layer_no), XY16TOREG32(  sx,   sy));
    nemadc_reg_write(NEMADC_REG_LAYER_SIZEXY           (layer_no), XY16TOREG32( szx,  szy));
    nemadc_reg_write(NEMADC_REG_LAYER_RESXY            (layer_no), XY16TOREG32(resx, resy));
    nemadc_reg_write(NEMADC_REG_LAYER_STRIDE           (layer_no), XY16TOREG32(layer->buscfg, stride));

    if(nemadc_get_ipversion()>=COLORFORM_REG_VERSION){
        nemadc_reg_write(NEMADC_REG_LAYER_COLORDEC_STARTXY (layer_no), XY16TOREG32( csx,  csy));
    }
    if ((NEMADC_CFG_LAYER_SCALER(layer_no) & nemadc_config)){
        int scalex = 0x4000;
        int scaley = 0x4000;
        nemadc_reg_write(NEMADC_REG_LAYER_SCALEX  (layer_no), scalex);
        nemadc_reg_write(NEMADC_REG_LAYER_SCALEY  (layer_no), scaley);
    }
    if ( ( (1<<1U) & nemadc_colmod) ||
         ( (1<<2U) & nemadc_colmod) ||
         ( (1<<5U) & nemadc_colmod) ||
         ( (1<<7U) & nemadc_colmod)    )
    {
        if (layer_no==0) {
            nemadc_reg_write(NEMADC_REG_LAYER0_UBASE, u_base);
            nemadc_reg_write(NEMADC_REG_LAYER0_VBASE, v_base);
            nemadc_reg_write(NEMADC_REG_LAYER0_UVSTRIDE, XY16TOREG32(u_stride, v_stride));
        }
        else if (layer_no==1) {
            nemadc_reg_write(NEMADC_REG_LAYER1_UBASE, u_base);
            nemadc_reg_write(NEMADC_REG_LAYER1_VBASE, v_base);
            nemadc_reg_write(NEMADC_REG_LAYER1_UVSTRIDE, XY16TOREG32(u_stride, v_stride));
        }
        else if (layer_no==2) {
            nemadc_reg_write(NEMADC_REG_LAYER2_UBASE, u_base);
            nemadc_reg_write(NEMADC_REG_LAYER2_VBASE, v_base);
            nemadc_reg_write(NEMADC_REG_LAYER2_UVSTRIDE, XY16TOREG32(u_stride, v_stride));
        }
        else if (layer_no==3) {
            nemadc_reg_write(NEMADC_REG_LAYER3_UBASE, u_base);
            nemadc_reg_write(NEMADC_REG_LAYER3_VBASE, v_base);
            nemadc_reg_write(NEMADC_REG_LAYER3_UVSTRIDE, XY16TOREG32(u_stride, v_stride));
        } else {

        }
    }

    layer->mode = NEMADC_LAYER_ENABLE | NEMADC_LAYER_AHBLOCK | ((layer->alpha & 0xff) << 16) | (layer->format & 0x1fU) | (layer->blendmode << 8) | (layer->flipx_en << 6) | (layer->flipy_en << 7) | extra_bits;

    nemadc_reg_write(NEMADC_REG_LAYER_MODE    (layer_no),  layer->mode);

    if(nemadc_get_ipversion()>=0x00230200U){
        nemadc_reg_write(NEMADC_REG_LAYER_COLOR_FORMAT(layer_no),  layer->format);
    }

    return;
}


void nemadc_set_layer(int layer_no, nemadc_layer_t *layer)
{
//-----------------------------------------------------------------------
    int resx  = layer->resx;
    int resy  = layer->resy;
    int szx   = layer->sizex;
    int szy   = layer->sizey;
//-----------------------------------------------------------------------
    if (!(NEMADC_CFG_LAYER_SCALER(layer_no) & nemadc_config) ||
        (resx == szx && resy == szy) ) {
        nemadc_set_layer_no_scale(layer_no, layer);
        return;
    }

//-----------------------------------------------------------------------
    layer->mode = NEMADC_LAYER_ENABLE | NEMADC_LAYER_AHBLOCK | ((layer->alpha&0xff)<<16) | (layer->format) | (layer->blendmode << 8);

    int stride   = layer->stride;
    int u_stride = layer->u_stride;
    int v_stride = layer->v_stride;
    int u_base   = layer->u_base;
    int v_base   = layer->v_base;

    if ( (layer->format & 0x3f) == NEMADC_V_YUV420 ) {
        if (u_stride < 0 || v_stride < 0) {
            u_stride = (layer->resx+1)>>1;
            v_stride = (layer->resx+1)>>1;
        }

        if ( layer->u_base == 0) {
            u_base = layer->baseaddr_phys +  layer->resx*layer->resy;
            v_base = u_base               + (layer->resx*layer->resy)/4;
        }
    }

    uint32_t addr  = layer->baseaddr_phys;
    // uint32_t resx  = layer->resx;
    // uint32_t resy  = layer->resy;
    int      sx    = layer->startx;
    int      sy    = layer->starty;
    // uint32_t szx   = layer->sizex;
    // uint32_t szy   = layer->sizey;

    ///Scaling
    ///-----------------------------------------
    int scalex = 0x4000;
    int scaley = 0x4000;


    if ( resx != szx ) {
        scalex = (0x4000 * (resx-1) / (szx) ) ;
    }
    if ( resy != szy ) {
        scaley = (0x4000 * (resy-1) / (szy) );
    }

    if ( scalex & ~(0x3ffff) || scaley & ~(0x3ffff) ) {
        return;
    }

    float fscalex = (resx == szx) ? 1.f : (float)(resx-1)/szx;
    float fscaley = (resy == szy) ? 1.f : (float)(resy-1)/szy;

    ///find visible area
    ///-----------------------------------------
    int endx       = sx + szx;
    int endy       = sy + szy;

    if (sx < 0) {
        int tx = (float)sx * fscalex;

        tx    = nemadc_adjust_sx(layer->format, tx);
        sx    = __floor((float)tx/fscalex);

        unsigned diff = (unsigned)nemadc_addr_offsetX(layer->format, tx);
        addr -= diff;
        szx  += sx;
        endx  = szx;

        if ( (layer->format & 0x3f) == NEMADC_V_YUV420 ) {
            u_base -= tx / 2;
            v_base -= tx / 2;
        }

        sx    = 0;
    }

    if (sy < 0) {
        int ty = (float)sy * fscaley;

        ty    = nemadc_adjust_sy(layer->format, ty);
        sy    = __floor((float)ty/fscaley);

        if ( (layer->format & 0x3f) == NEMADC_V_YUV420 ) {
            // ty += 4;
            if (ty > 0) {
                ty = 0;
            }
        } else {
            // ++ty;
            if (ty > 0) {
                ty = 0;
            }
        }

        addr -= (unsigned)nemadc_addr_offsetY(layer->format, ty, stride);
        szy  += sy;
        endy  = szy;

        if ( (layer->format & 0x3f) == NEMADC_V_YUV420 ) {
            u_base -= (ty*u_stride) / 2;
            v_base -= (ty*v_stride) / 2;
        }

        sy    = 0;
    }

    if ( endx > (int)nemadc_display.resx ) {
        endx = (int)nemadc_display.resx;
    }

    if ( endy > (int)nemadc_display.resy ) {
        endy = (int)nemadc_display.resy;
    }

    szx = endx-sx;
    szy = endy-sy;

    resx = ((int)(fscalex*szx+15))&(~7);
    resy = ((int)(fscaley*szy+15))&(~7);

    if (resx < 256) {
        resx = 256;
    }

    do {
        if ( fscaley <= NEMADC_SKIP_RATIO ) {
            break;
        }

        if (stride & 0x4000) {
            break;
        }

        resy   >>= 1;
//            szy  >>= 1;
        stride <<= 1;
        u_stride <<= 1;
        v_stride <<= 1;
        scaley >>= 1;
        fscaley *= 0.5f;
    } while(1);

    nemadc_reg_write(NEMADC_REG_LAYER_BASEADDR(layer_no), addr);
    nemadc_reg_write(NEMADC_REG_LAYER_STARTXY (layer_no), XY16TOREG32(  sx,   sy));
    nemadc_reg_write(NEMADC_REG_LAYER_SIZEXY  (layer_no), XY16TOREG32( szx,  szy));
    nemadc_reg_write(NEMADC_REG_LAYER_RESXY   (layer_no), XY16TOREG32(resx, resy));
    nemadc_reg_write(NEMADC_REG_LAYER_STRIDE  (layer_no), (layer->buscfg<<16) | (stride & 0xffff));
    nemadc_reg_write(NEMADC_REG_LAYER_SCALEX  (layer_no), scalex);
    nemadc_reg_write(NEMADC_REG_LAYER_SCALEY  (layer_no), scaley);
    if (layer_no==0) {
        nemadc_reg_write(NEMADC_REG_LAYER0_UBASE,    u_base);
        nemadc_reg_write(NEMADC_REG_LAYER0_VBASE,    v_base);
        nemadc_reg_write(NEMADC_REG_LAYER0_UVSTRIDE, XY16TOREG32(u_stride, v_stride));
    }
    else if (layer_no==1) {
        nemadc_reg_write(NEMADC_REG_LAYER1_UBASE,    u_base);
        nemadc_reg_write(NEMADC_REG_LAYER1_VBASE,    v_base);
        nemadc_reg_write(NEMADC_REG_LAYER1_UVSTRIDE, XY16TOREG32(u_stride, v_stride));
    }
    else if (layer_no==2) {
        nemadc_reg_write(NEMADC_REG_LAYER2_UBASE,    u_base);
        nemadc_reg_write(NEMADC_REG_LAYER2_VBASE,    v_base);
        nemadc_reg_write(NEMADC_REG_LAYER2_UVSTRIDE, XY16TOREG32(u_stride, v_stride));
    }
    else if (layer_no==3) {
        nemadc_reg_write(NEMADC_REG_LAYER3_UBASE,    u_base);
        nemadc_reg_write(NEMADC_REG_LAYER3_VBASE,    v_base);
        nemadc_reg_write(NEMADC_REG_LAYER3_UVSTRIDE, XY16TOREG32(u_stride, v_stride));
    } else {

    }

    layer->mode = NEMADC_LAYER_ENABLE | NEMADC_LAYER_AHBLOCK | ((layer->alpha&0xffU)<<16) | ((layer->format)& 0x1fU) | (layer->blendmode << 8);

    nemadc_reg_write(NEMADC_REG_LAYER_MODE    (layer_no),  layer->mode);

    return;
}
//-----------------------------------------------------------------------

void nemadc_set_layer_addr(int layer_no, uintptr_t addr) {
    nemadc_reg_write(NEMADC_REG_LAYER_BASEADDR(layer_no),  addr);
}

void nemadc_clkdiv(int div, int div2, int dma_prefetch, int phase)
{
    if (dma_prefetch < 0) {
        dma_prefetch = 4;
    }

    //div == 0 -> divide with 1
    //div == 1 -> divide with 1
    //div == 2 -> divide with 2
    //div == 3 -> divide with 3
    //etc...
    if (div <= 0) {
        div = 0;
    }

    //div2 == 0 -> divide with 1
    //div2 == 1 -> divide with 1
    //div2 == 2 -> divide with 2
    //div2 == 3 -> divide with 3
    //etc...
    if (div2 <= 0) {
        div2 = 0;
    }
#ifdef NEMADC_MODIFY_DIV2
    else {
        --div2;
    }
#endif

    if(nemadc_get_ipversion()>=0x00230200U){
        //div: 7 bits
        if (div > 0x7f) {
            div = 0x7f;
        }

        //div2: 7 bits
        if (div2 > 0x7f) {
            div2 = 0x7f;
        }
    } else {
        //div: 6 bits
        if (div > 0x3f) {
            div = 0x3f;
        }

        //div2: 5 bits
        if (div2 > 0x1f) {
            div2 = 0x1f;
        }
    }

    //dma_prefetch: 6 bits
    if (dma_prefetch > 0x3f) {
        dma_prefetch = 0x3f;
    }

    //phase: 3 bits
    if (phase > 0x07) {
        phase = 0x07;
    }

    if(nemadc_get_ipversion()>=0x00230200U){
        nemadc_reg_write(NEMADC_REG_CLKCTRL, ((unsigned)div) | ((unsigned)dma_prefetch<<8) | ((unsigned)div2<<25) | ((unsigned)phase<<24) );
    } else {
        nemadc_reg_write(NEMADC_REG_CLKCTRL, ((unsigned)div) | ((unsigned)dma_prefetch<<8) | ((unsigned)div2<<27) | ((unsigned)phase<<24) );
    }
}

void nemadc_clkctrl(uint32_t ctrl)
{
    nemadc_reg_write(NEMADC_REG_GPIO, ctrl);
}

//-----------------------------------------------------------------------
//@function nemadc_layer_enable
//@brief Enables a layer
//@param int layer_no desc:layer to enable
//@return void desc:void
//-----------------------------------------------------------------------
void nemadc_layer_enable(int layer_no)
{
    uint32_t cur_mode = nemadc_reg_read(NEMADC_REG_LAYER_MODE(layer_no));
    nemadc_reg_write(NEMADC_REG_LAYER_MODE(layer_no), cur_mode | NEMADC_LAYER_ENABLE);
}

//-----------------------------------------------------------------------
//@function nemadc_layer_disable
//@brief Disables a layer
//@param int layer_no desc:layer to disable
//@return void desc:void
//-----------------------------------------------------------------------
void nemadc_layer_disable(int layer_no)
{
    uint32_t cur_mode = nemadc_reg_read(NEMADC_REG_LAYER_MODE(layer_no));
    nemadc_reg_write( NEMADC_REG_LAYER_MODE(layer_no), cur_mode & (~NEMADC_LAYER_ENABLE) );
}
//-----------------------------------------------------------------------

unsigned char nemadc_check_config(uint32_t flag) {
    uint32_t tmp_config = nemadc_reg_read(NEMADC_REG_CONFIG);

    return (tmp_config & flag) != 0;
}
//-----------------------------------------------------------------------

uint32_t nemadc_get_col_mode(void)
{
    return nemadc_reg_read(NEMADC_REG_COLMOD);
}
//-----------------------------------------------------------------------

//@function nemadc_set_palette
//@brief Sets an entry in the lut8 Palatte Gamma table
//@param int index desc: Color Index
//@param int colour desc: 32-bit RGBA colour value or Gamma index
//@return void desc:void
//-----------------------------------------------------------------------
void nemadc_set_palette(uint32_t index, uint32_t colour)
{
    nemadc_reg_write(NEMADC_REG_PALETTE+4U*index, colour);
}
//-----------------------------------------------------------------------

//@function nemadc_get_palette
//@brief Reads an entry from the lut8 Palatte Gamma table
//@param int index desc: Color Index
//@return int desc: Return Colour for given palette index
//-----------------------------------------------------------------------
int nemadc_get_palette(uint32_t index)
{
    return nemadc_reg_read(NEMADC_REG_PALETTE+4U*index);
}


void nemadc_set_layer_gamma_lut(int layer, int index, int colour)
{
    nemadc_reg_write(NEMADC_REG_GAMMALUT(layer)+4*index, colour);
}

//@function nemadc_get_layer_gamma_lut
//@brief Sets an entry in the lut8 Palatte Gamma table for a layer
//@param int layer desc: layer to read Gammalut
//@param int index desc: Color Index
//@return int desc: Return Pallete index
//-----------------------------------------------------------------------
int nemadc_get_layer_gamma_lut(int layer, int index)
{
    return nemadc_reg_read(NEMADC_REG_GAMMALUT(layer)+4*index);
}
//-----------------------------------------------------------------------

void nemadc_cursor_enable(int enable) {
    uint32_t cur_mode = nemadc_reg_read(NEMADC_REG_MODE);
    // unset the ONE_FRAME bit
    // or else it will cause NemaDC to resend a frame
    cur_mode &= ~NEMADC_ONE_FRAME;

    if (enable != 0) {
        nemadc_reg_write(NEMADC_REG_MODE,
                         cur_mode | (  NEMADC_CURSOR ));
    } else {
        nemadc_reg_write(NEMADC_REG_MODE,
                         cur_mode & (~(NEMADC_CURSOR)));
    }

    nemadc_reg_write(NEMADC_REG_PLAY, 0);
}

void nemadc_cursor_xy(int x, int y)
//-----------------------------------------------------------------------
{
    nemadc_reg_write(NEMADC_REG_CURSORXY, XY16TOREG32(x, y));
}
//-----------------------------------------------------------------------

void nemadc_set_cursor_img(unsigned char *img)
{
    uint32_t addr = 0;

    for (addr = 0U; addr < 128U; ++addr) {
        uint32_t data = 0;
        for (unsigned i = 0; i < 32U; i += 4U) {
            data |= ((*img)&0xfU) << i;
            ++img;
        }
        nemadc_reg_write(NEMADC_REG_CURSOR_IMAGE+4U*addr, data);
    }
}

void nemadc_set_cursor_lut(uint32_t index, uint32_t colour)
{
    if (index < 16U) {
        nemadc_reg_write(NEMADC_REG_CURSOR_LUT+4U*index, (colour>>8));
    }
}

int
nemadc_get_layer_count(void)
{
    int l_count = 0;

    while ( l_count < 4 && NEMADC_CFG_LAYER_EXISTS(l_count) & nemadc_config ) {
        ++l_count;
    }

    return l_count;
}
