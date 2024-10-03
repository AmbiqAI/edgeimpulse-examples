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

#include "nema_dc_jdi.h"
#include "nema_dc.h"
#include "nema_dc_regs.h"
#include "nema_dc_mipi.h"

#include <stdarg.h>

#define CUSTOM_ASSERT
#ifndef CUSTOM_ASSERT

#include <assert.h>
#define ASSERT(expr) assert(expr)

#else

//Replace here your implementation
#define ASSERT(expr)  \
        if (!(expr)){ \
            return; \
        }


#endif

#define JDI_CFG_DEFAULT       ( MIPICFG_DBI_EN    | \
                                MIPICFG_RESX      | \
                                MIPICFG_DMA       | \
                                MIPICFG_SPI_HOLD  )

#define SPI_PHY_CONFIG        ( MIPICFG_SPI_CSX_V | \
                                MIPICFG_DIS_TE    )

static uint32_t default_config    = 0;
static uint32_t default_data_mode = 0;
static uint32_t default_phy       = 0;

void
nemadc_jdi_configure(uint32_t phy, uint32_t data_mode, uint32_t extra_flags)
{
    default_phy = phy;
    switch (phy) {
        case JDI_PHY_SPI4_SHARP:
            default_config = JDI_CFG_DEFAULT | MIPICFG_SPI4 | MIPICFG_INV_ADDR;
            break;
        case JDI_PHY_SPI3:
            default_config = JDI_CFG_DEFAULT | MIPICFG_SPI3;
            break;
        case JDI_PHY_SPI4:
        default:
            default_config = JDI_CFG_DEFAULT | MIPICFG_SPI4;
            break;
    }

    default_data_mode = data_mode;
    switch (data_mode) {
        case JDI_DATAMODE_1BIT:
        case JDI_DATAMODE_SHARP:
            default_config |= MIPICFG_1RGB111_OPT3;
            break;
        case JDI_DATAMODE_4BIT:
            default_config |= MIPICFG_1RGB111_OPT2;
            break;
        case JDI_DATAMODE_3BIT:
        default:
            default_config |= MIPICFG_1RGB111_OPT4;
            break;
    }

    default_config |= extra_flags | SPI_PHY_CONFIG;
    nemadc_MIPI_CFG_out( default_config );
}

static inline void
nemadc_jdi_cmd_release(void) {
    nemadc_MIPI_CFG_out( default_config & ((uint32_t ) ~(MIPICFG_SPI_HOLD )) );
    while( (nemadc_reg_read(NEMADC_REG_STATUS)&0x1000U) != 0U) { } //make sure command is sent

    if (default_config != 0U) {
        nemadc_MIPI_CFG_out( default_config );
    }
}

static inline void
nemadc_jdi_cmd_send(uint32_t cmd) {
    nemadc_MIPI_out ( (unsigned)MIPI_DBIB_CMD | (unsigned)cmd );
    nemadc_MIPI_out ( (unsigned)MIPI_DBIB_CMD | (unsigned)JDI_CMD_NOP ); //pad to 16bit
}

static inline uint8_t bit_invert(uint8_t val) {
    uint8_t inv_val = 0;
    for (uint8_t i = 0U; i < 8U; ++i) {
        inv_val |= ((val>>i)&1U) << (7U-i);
    }
    return inv_val;
}

void
nemadc_jdi_send_one_frame(int starty)
{
    //check if nemadc_jdi_configure() has been called
    if ( default_config == 0U ) {
        return;
    }

    if (starty <= 0) {
        starty = 1;
    }


    if (default_phy == (unsigned)JDI_PHY_SPI4_SHARP) {
        //Start frame transfer
        nemadc_MIPI_out ( (unsigned)MIPI_DBIB_CMD | 0x80U );
        //Start line address
        nemadc_MIPI_out ( (unsigned)MIPI_DBIB_CMD | (unsigned)MIPI_DBIB_STORE_BASE_ADDR | bit_invert( (uint8_t)starty) );
    }else{
        nemadc_MIPI_out ( (unsigned)MIPI_DBIB_CMD | default_data_mode );
        nemadc_MIPI_out ( (unsigned)MIPI_DBIB_CMD | (unsigned)MIPI_DBIB_STORE_BASE_ADDR | (unsigned)starty );
    }
    nemadc_set_mode(NEMADC_ONE_FRAME);
    while( !(nemadc_reg_read(NEMADC_REG_STATUS)&0x2U)) { } //make sure command is sent
}

void
nemadc_jdi_clear(void)
{
    //check if nemadc_jdi_configure() has been called
    if ( default_config == 0U ) {
        return;
    }
    nemadc_jdi_cmd_send( JDI_CMD_CLEAR );
    nemadc_jdi_cmd_release();
}

void
nemadc_jdi_blink_off(void)
{
    //check if nemadc_jdi_configure() has been called
    if ( default_config == 0U ) {
        return;
    }
    nemadc_jdi_cmd_send( JDI_CMD_BLINKOFF );
    nemadc_jdi_cmd_release();
}

void
nemadc_jdi_blink_inv_colors(void)
{
    //check if nemadc_jdi_configure() has been called
    if ( default_config == 0U ) {
        return;
    }
    nemadc_jdi_cmd_send( JDI_CMD_BLINKINVERT );
    nemadc_jdi_cmd_release();
}

void
nemadc_jdi_blink_white(void)
{
    //check if nemadc_jdi_configure() has been called
    if ( default_config == 0U ) {
        return;
    }
    nemadc_jdi_cmd_send( JDI_CMD_BLINKWHITE );
    nemadc_jdi_cmd_release();
}

void
nemadc_jdi_blink_black(void)
{
    //check if nemadc_jdi_configure() has been called
    if ( default_config == 0U ) {
        return;
    }
    nemadc_jdi_cmd_send( JDI_CMD_BLINKBLACK );
    nemadc_jdi_cmd_release();
}


static MiP_display_config_t  mip_display;
static int vck_idle = 0;
static int offset   = 0;

void
nemadc_set_mip_panel_parameters( MiP_display_config_t *display){

    mip_display.resx = display->resx;
    mip_display.resy = display->resy;

    mip_display.XRST_INTB_delay = display->XRST_INTB_delay - 1;
    mip_display.XRST_INTB_width = display->XRST_INTB_width;

    mip_display.VST_GSP_delay   = display->VST_GSP_delay - 1;
    mip_display.VST_GSP_width   = display->VST_GSP_width;

    mip_display.VCK_GCK_delay             = display->VCK_GCK_delay - display->VST_GSP_delay - 2;
    mip_display.VCK_GCK_width             = display->VCK_GCK_width - 1;
    mip_display.VCK_GCK_closing_pulses    = display->VCK_GCK_closing_pulses;

    mip_display.HST_BSP_delay  = display->HST_BSP_delay;
    mip_display.HST_BSP_width  = display->HST_BSP_width;

    mip_display.HCK_BCK_data_start  = display->HCK_BCK_data_start;

    mip_display.ENB_GEN_delay  = display->ENB_GEN_delay;
    mip_display.ENB_GEN_width  = display->ENB_GEN_width;

}

#ifdef MIP_WA
uint32_t first_partial_start[4];
#endif

static void
update_partials_per_layer( int layer0_active, nemadc_layer_t *layer0,
                           int layer1_active, nemadc_layer_t *layer1,
                           int layer2_active, nemadc_layer_t *layer2,
                           int layer3_active, nemadc_layer_t *layer3,
                           int partial_region, int partial_region_start, int partial_region_end){

    int per_layer_region_start, per_layer_region_end;

    int layer_active[4];
    nemadc_layer_t *layer[4];

    layer_active[0] = layer0_active;
    layer_active[1] = layer1_active;
    layer_active[2] = layer2_active;
    layer_active[3] = layer3_active;
    layer[0]        = layer0;
    layer[1]        = layer1;
    layer[2]        = layer2;
    layer[3]        = layer3;

    offset   = 0;

    for (int i = 0; i < 4; i++ ){

        if (layer_active[i]){

            int layer_first_row = ( layer[i]->starty < 0 ) ? 0 : layer[i]->starty;
            int layer_last_row  =  layer[i]->resy + layer[i]->starty - 1;

            // set vck_idle to 1 only if the first partial region starts on zero else set equal to 2.
            if (partial_region == 0) {

                if (partial_region_start == 0) {
                    vck_idle = 1;

                    // if we are on region 0 and it start from 0 and ends with (panel_resy - 1)
                    // then we have a full frame and we dont want to set the offset
                    if (partial_region_end < (mip_display.resy-1)){
                        offset   = 1;
                    }

                } else{
                    vck_idle = 2;
                    offset   = 0;
                }

            }else{
                // NOT full frame transmission
                if (vck_idle == 1) {
                    offset = 1;
                } else {
                    offset = 0;
                }
            }

            if( ( layer_last_row  >= partial_region_start )  &&
                ( layer_first_row <= partial_region_end   )
            ){

                per_layer_region_start = ( layer_first_row >= partial_region_start )?  0 : (partial_region_start - layer_first_row);

                per_layer_region_end   = ( layer_last_row  >= partial_region_end   )? (partial_region_end - layer_first_row)  : (layer_last_row - layer_first_row);

                while((nemadc_reg_read(NEMADC_REG_STATUS)&(1U<<5))!=0);
                nemadc_reg_write(NEMADC_REG_LAYER_PARTIAL,    ( ( ( (per_layer_region_start*2) + offset ) ) | ( ( (per_layer_region_end*2) + 2 + offset ) << 12 ) | ( 1 << 29 ) | ( i << 30 ) ) );

#ifdef MIP_WA
                if(partial_region == 0){
                    if( (partial_region_start != 0) && layer[i]->flipy_en){

                        first_partial_start[i] = ( (per_layer_region_start * 2) + offset );
                    }else{

                        first_partial_start[i] = 0;
                    }
                }
#endif

            }

        }

    }

}

static void
reset_mip_fifo(void){

    nemadc_reg_write(NEMADC_REG_MIP_CFG3, ( 1 << 30 ) );

    // Enable layers to reset MiP FiFo
    uint32_t config = nemadc_reg_read(NEMADC_REG_CONFIG);
    for (int i = 0; i<4; i++){
        if( config & ( 1U << (8+(i*4)) ) ){
            nemadc_reg_write(NEMADC_REG_LAYER_MODE(i), (1 << 31));
        }
    }


    for (int i = 0; i<4; i++){
        if( config & ( 1U << (8+(i*4)) ) ){
            nemadc_reg_write(NEMADC_REG_LAYER_MODE(i), 0 );
        }
    }

    nemadc_reg_write(NEMADC_REG_MIP_CFG3, 0 );

}

void
nemadc_mip_setup( int layer0_active, nemadc_layer_t *layer0,
                  int layer1_active, nemadc_layer_t *layer1,
                  int layer2_active, nemadc_layer_t *layer2,
                  int layer3_active, nemadc_layer_t *layer3,
                  int partial_regions, ...)
{
    vck_idle = 0;
    int fastforward_VCK_width = 0;
    int en_partial_update     = 0;
    int end_of_partial_offset = 0;

    int partial_region_start, partial_region_end;
    int next_region_start = 0;
    int previous_region_end = -1;
    int increase_timing = 0;

    int layer_active[4];
    nemadc_layer_t *layer[4];

    layer_active[0] = layer0_active;
    layer_active[1] = layer1_active;
    layer_active[2] = layer2_active;
    layer_active[3] = layer3_active;
    layer[0]        = layer0;
    layer[1]        = layer1;
    layer[2]        = layer2;
    layer[3]        = layer3;

#ifdef MIP_WA
    if (layer[0]->flipy_en || layer[1]->flipy_en || layer[2]->flipy_en || layer[3]->flipy_en){
        ASSERT((partial_regions >= 0) && (partial_regions <= 1));
    }
#endif

    ASSERT((partial_regions >= 0) && (partial_regions <= 16));

    // Reset MiP FiFo
    //--------------------------------------------------
    reset_mip_fifo();
    //--------------------------------------------------

    // push parameters
    if (partial_regions > 0)
    {
        va_list vl;

        va_start(vl,partial_regions);

        partial_region_start = va_arg(vl,int);

        for(int j = 0; j < partial_regions; j++)
        {

            partial_region_end   = va_arg(vl,int);

            //--------------------------------------------------
            ASSERT(partial_region_end >= partial_region_start); // Send only one line.
            ASSERT(partial_region_start > previous_region_end); // User must set the partial regions in incremental order. Must not contain overlapping
            //--------------------------------------------------

            if (j != (partial_regions-1)){
                next_region_start   = va_arg(vl,int);
            }

            // if there is a gap between two consecutive regions send as is.
            if( next_region_start != (partial_region_end + 1) ){

                update_partials_per_layer(layer0_active, layer0,
                                          layer1_active, layer1,
                                          layer2_active, layer2,
                                          layer3_active, layer3,
                                          j, partial_region_start, partial_region_end);

                while((nemadc_reg_read(NEMADC_REG_STATUS)&(1U<<9))!=0);
                nemadc_reg_write((NEMADC_REG_MIP_PARTIAL),    ( ( partial_region_start*2 ) + offset ) | ( (( partial_region_end*2 ) + 2 + offset) << 12 )  );


                partial_region_start = next_region_start;
                //--------------------------------------------------
                previous_region_end = partial_region_end;

            // else recalculate the new partial_region_end
            // this will automatically reduce the number of regions defined by user.
            // (i.e: two partials 17~26 & 27~33 ==> one partial 17~33)
            } else{

                partial_regions-- ;
                j--;
            }

        }

        va_end(vl);

        increase_timing = 4;
        end_of_partial_offset = 4;
        fastforward_VCK_width = 3;
        en_partial_update     = 1;

        if( (layer_active[2] || layer_active[3]) && (nemadc_get_ipversion() == 0x00230202) ){
            previous_region_end += 2;
        }

        mip_display.XRST_INTB_width = (previous_region_end*2) + mip_display.VCK_GCK_closing_pulses + 2;

    }else{ // Full Frame Update.

        update_partials_per_layer(layer0_active, layer0,
                                  layer1_active, layer1,
                                  layer2_active, layer2,
                                  layer3_active, layer3,
                                  partial_regions, 0, mip_display.resy-1);

        while((nemadc_reg_read(NEMADC_REG_STATUS)&(1U<<9))!=0);
        nemadc_reg_write((NEMADC_REG_MIP_PARTIAL),   offset | ( ( ( ( mip_display.resy - 1 ) *2) + 3 + offset ) << 12 )  );

        increase_timing = 1;
        end_of_partial_offset = 2;
        previous_region_end = mip_display.resy;
        fastforward_VCK_width = 0;
        en_partial_update     = 0;
    }


    // Configure Framegen
    nemadc_timing( mip_display.resx, 1, 1, 1,
                  (2*previous_region_end) + increase_timing, 1, 1, 0);

    // Configure Layers
    for (int i = 0; i < 4; i++ ){
        if(layer_active[i] == 1){

            nemadc_set_layer(i, layer[i]);

#ifdef MIP_WA
            if (first_partial_start[i]){

                uint32_t base_addr = nemadc_reg_read(NEMADC_REG_LAYER_BASEADDR(i));

                nemadc_set_layer_addr(i, base_addr - (2 * (( ( first_partial_start[i] >> 1 ) * nemadc_stride_size(layer[i]->format, layer[i]->resx)))) );
            }
#endif

        }
    }

    // Configure MiP Interface
    int VCK_pol               = 0;
    int HCK_pol               = 1;
    int ENB_starting_value    = 0;

    int HCK_max_pos           = mip_display.resx + 3;
    int end_of_image          = (previous_region_end*2) + end_of_partial_offset;

    nemadc_reg_write(NEMADC_REG_FORMAT_CTRL,  ( ( HCK_pol << 1 ) | ( VCK_pol <<  2 ) | ( mip_display.VST_GSP_delay <<  3 )  | (mip_display.VST_GSP_width<<13) | (mip_display.HST_BSP_delay<<23) | (mip_display.HST_BSP_width<<26) ));
    nemadc_reg_write(NEMADC_REG_FORMAT_CTRL2, ( ( mip_display.ENB_GEN_delay   ) | ( mip_display.ENB_GEN_width << 10 ) | ( mip_display.XRST_INTB_delay << 20 ) ) );
    nemadc_reg_write(NEMADC_REG_FORMAT_CTRL3, ( ( mip_display.XRST_INTB_width ) | ( HCK_max_pos << 16 ) ) );
    nemadc_reg_write(NEMADC_REG_MIP_CFG1,     ( ( mip_display.HCK_BCK_data_start ) | ( fastforward_VCK_width << 24 ) ) );
    nemadc_reg_write(NEMADC_REG_MIP_CFG2,     ( ( end_of_image       ) | ( mip_display.VCK_GCK_width << 12 ) | ( mip_display.VCK_GCK_delay << 24 ) ) );
    nemadc_reg_write(NEMADC_REG_MIP_CFG3,     ( ( ENB_starting_value ) | ( mip_display.VCK_GCK_closing_pulses << 10 ) | ( vck_idle  << 15 ) | ( en_partial_update     << 31 ) ) );

}
