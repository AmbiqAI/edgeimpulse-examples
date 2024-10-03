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
// FILE NAME  : assist.c
// KEYWORDS   :
// PROJECT    : OpenVG
// PURPOSE    : Assist functions for debugging (open/close files, dump Graphics Memory, etc)
// DEPARTMENT : online IP Products
// AUTHOR     : IS
// GENERATION : 9/30/2008  12:53
// RECEIVER   : TR
// -----------------------------------------------------------------------------
// RELEASE HISTORY for module architecture
// VERSION    DATE                      AUTHOR            DESCRIPTION
//  1.0       9/30/2008  12:53            IS            Initial Release
//  1.1      11/11/2010                   IS            Major recoding to improve portability
// -----------------------------------------------------------------------------

#ifndef _ASSIST_C_
#define _ASSIST_C_
//-------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>

#include "nema_graphics.h"
#include "assist.h"
//-------------------------------------------
#define DEBUG_TEXT
//-------------------------------------------
#ifndef PRINTF
#define PRINTF(...)
#endif
//-------------------------------------------
T2D_TESTTYPE ref_imp = T2D_IMPL_TEST;
//-------------------------------------------------------------
// Replacement for common std C function in case they are not available
// Random Number generation
//-------------------------------------------------------------
unsigned col2rgba(uint32_t  col_in, int col_mode) ;
static unsigned long int next = 1;
static int   fb_format;
static void *fb_basevirt;
static int   fb_size;
//-------------------------------------------------------------
static char snapshot_imp_path_str[400];
// static char snapshot_ref_path_str[400];
//-------------------------------------------------------------
int ts_rand(void) // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next&0x7fffffff);
//    return (unsigned int)(next/65536) % 32768;
}
//-------------------------------------------------------------
void ts_srand(unsigned int seed)
{
    next = seed;
}
//-------------------------------------------------------------
int ts_strlen(const char * text)
{
    int i=0;
    while (text[i++]!=0);
    return i;
}
//-------------------------------------------------------------
void ts_strcpy(char * s1, const char * s2)
{
    strcpy(s1, s2);
}
//-------------------------------------------------------------
char * ts_strcat(char *dest, const char *src)
{
    size_t i,j;
    for (i = 0; dest[i] != '\0'; i++)
        ;
    for (j = 0; src[j] != '\0'; j++)
        dest[i+j] = src[j];
    dest[i+j] = '\0';
    return dest;
}

// ASCII String Manipulation
/* reverse:  reverse string s in place */
//-------------------------------------------------------------
void reverse(char s[])
{
    int i, j;

    for (i = 0, j = ts_strlen(s)-1; i<j; i++, j--) {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa_0(int n, char s[])
{
    s[0]=s[1]=s[2]=s[3]='0';
         if(n<10)   { s[3]='0'+n; }
    else if(n<100)  { s[2]='0'+(n/10);   s[3]='0'+(n%10);       }
    else if(n<1000) { s[1]='0'+(n/100);  s[2]='0'+((n%100)/10); s[3]='0'+(n%10);  }
    else            { s[0]='0'+(n/1000); s[1]='0'+(n/100);      s[2]='0'+((n%100)/10); s[3]='0'+(n%10); }

}
//-------------------------------------------------------------
void itoa_2(int n, char s[])
{
         if(n<10)       { s[0]='0'+n;          s[1]='\0';}
    else if(n<100)      { s[0]='0'+(n/10);     s[1]='0'+(n%10);             s[2]='\0';}
    else if(n<1000)     { s[0]='0'+(n/100);    s[1]='0'+((n%100)/10);       s[2]='0'+(n%10);            s[3]='\0'; }
    else if(n<10000)    { s[0]='0'+(n/1000);   s[1]='0'+((n%1000)/100);     s[2]='0'+((n%100)/10);      s[3]='0'+(n%10);         s[4]='\0'; }
    else if(n<100000)   { s[0]='0'+(n/10000);  s[1]='0'+((n%10000)/1000);   s[2]='0'+((n%1000)/100);    s[3]='0'+((n%100)/10);   s[4]='0'+(n%10);       s[5]='\0'; }
    else                { s[0]='0'+(n/100000); s[1]='0'+((n%100000)/10000); s[2]='0'+((n%10000)/1000);  s[3]='0'+((n%1000)/100); s[4]='0'+((n%100)/10); s[5]='0'+(n%10); s[6]='\0';}
}

void itoa(int n, char s[])
{
         if(n<10)  { s[0]='0'+n; s[1]='\0';}
    else if(n<100) { s[0]='0'+(n/10);  s[1]='0'+(n%10);       s[2]='\0';}
    else           { s[0]='0'+(n/100); s[1]='0'+((n%100)/10); s[2]='0'+(n%10); s[3]='\0'; }

}

//-------------------------------------------------------------
/* itoa:  convert n to characters in s */
void aitoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';

////    reverse(s);
}
//-------------------------------------------------------------
void decimal_to_binary(int n,char s[]){
    int temp_n, i;
    int divider[8]={128,64,32,16,8,4,2,1};
    temp_n=n;
    for (i=0; i<8; i++) {
        int calc=temp_n/divider[i];
        if (calc) {s[7-i]='1' - 48; temp_n=temp_n-divider[i];}
        else {s[7-i]='0' - 48;}
    }
    s[8]='\0';
}

// Prints text on the console
//-----------------------------------------------------------------------------------------------------------------------
void print_t(char * text)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    PRINTF("%s\n", text);
#endif
}

// Same as print_t without new line character
//-----------------------------------------------------------------------------------------------------------------------
extern void aprint(const char * text)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    PRINTF("%s", text);
#endif
}

// Print a Hex number on screen
//-----------------------------------------------------------------------------------------------------------------------
void printh(int number)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    PRINTF("%08x\n", number);
#endif
}


// Print a decimal number on screen
//-----------------------------------------------------------------------------------------------------------------------
int get_assist_time(void)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    int t0;
    struct timeval tim;

    gettimeofday(&tim, NULL);
    t0 = tim.tv_sec*1000000 + tim.tv_usec;

    return t0;
#endif
}

// Print a decimal number on screen
//-----------------------------------------------------------------------------------------------------------------------
void nprintd(int number)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    PRINTF("%d\n", number);
#endif
}

// Print a decimal number on screen
//-----------------------------------------------------------------------------------------------------------------------
void printd(int number)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    PRINTF("%d\n", number);
#endif
}


// Get a snapshot of Graphics Memory
//-----------------------------------------------------------------------------------------------------------------------
void snapshot(void)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Get a snapshot of Graphics Memory. Need to be implemented\n", __func__);
}
//-----------------------------------------------------------------------------------------------------------------------



// Coverts RGBA8888 to other color formats
//-----------------------------------------------------------------------------------------------------------------------
inline unsigned col2rgba(uint32_t  col_in, int col_mode) {
    int r,g,b,a;
    unsigned col_out;
    char *ccol_out = (char *)&col_out;

    switch(col_mode){
    case NEMA_RGBX8888 :
    case NEMA_RGBA8888 : col_out = col_in;                              break;
    case NEMA_XRGB8888 :
    case NEMA_ARGB8888 :
                       b = (col_in >> 24)&0xff;
                       g = (col_in >> 16)&0xff;
                       r = (col_in >>  8)&0xff;
                       a = (col_in >>  0)&0xff;
                       col_out = (r<<24|g<<16|b<<8| a); break;
    case NEMA_RGBA5650 : r = (col_in>>11)&0x1f;
                       g = (col_in>> 5)&0x3f;
                       b = (col_in    )&0x1f;
                       r=r<<3|r>>2;
                       g=g<<2|g>>4;
                       b=b<<3|b>>2;
                       col_out = (r<<24|g<<16|b<<8| 0xff); break;
    case NEMA_RGBA5551 : r = (col_in>>11)&0x1f;
                       g = (col_in>> 6)&0x1f;
                       b = (col_in>> 1)&0x1f;
                       a = (col_in    )&0x01;
                       r=r<<3|r>>2;
                       g=g<<3|g>>2;
                       b=b<<3|b>>2;
                       a=a*0xff;
                       col_out = (r<<24|g<<16|b<<8| a); break;
    case NEMA_RGBA4444 : r = (col_in>>12)&0xf;
                       g = (col_in>> 8)&0xf;
                       b = (col_in>> 4)&0xf;
                       a = (col_in    )&0xf;
                       r=r<<4|r;
                       g=g<<4|g;
                       b=b<<4|b;
                       a=a<<4|a;
                       col_out = (r<<24|g<<16|b<<8| a); break;
    case NEMA_RGBA0800 : col_out = col_in;                              break;
    case NEMA_RGBA0008 : col_out = col_in;                              break;
    case NEMA_L8       : g = (col_in&0xff);
                       col_out = (g<<24|g<<16|g<<8| 0xff); break;
    case NEMA_RGBA3320 : col_out = col_in;                              break;
    case NEMA_BW1      : col_out = col_in;                              break;
    default          : col_out = col_in;                              break;
    }

    col_out = ccol_out[0] << 24 | ccol_out[1] << 16 | ccol_out[2] << 8 | ccol_out[3];

    return col_out;
}

// Get a snapshot of Graphics Memory - filename defined as argument
//-----------------------------------------------------------------------------------------------------------------------
void snapshot_name(char * filename, int index)
//-----------------------------------------------------------------------------------------------------------------------
{
    char filepath[400];
    char filepath_png[400];
    char filepath_rgba[400];
//    char cmd[400];

    if( fb_format == (int)NEMA_TSC4 ) {
        snprintf(filepath     , 400, "%s/%s.%d.fpga.tsc4" , snapshot_imp_path_str, filename, index);
        snprintf(filepath_png , 400, "%s/%s.%d.fpga.png" , snapshot_imp_path_str, filename, index);
        snprintf(filepath_rgba, 400, "%s/%s.%d.fpga.rgba", snapshot_imp_path_str, filename, index);
    }
    else if( fb_format == (int)NEMA_TSC6 ) {
        snprintf(filepath     , 400, "%s/%s.%d.fpga.tsc6" , snapshot_imp_path_str, filename, index);
        snprintf(filepath_png , 400, "%s/%s.%d.fpga.png" , snapshot_imp_path_str, filename, index);
        snprintf(filepath_rgba, 400, "%s/%s.%d.fpga.rgba", snapshot_imp_path_str, filename, index);
    }
    else if( fb_format == (int)NEMA_TSC6A ) {
        snprintf(filepath     , 400, "%s/%s.%d.fpga.tsc6a" , snapshot_imp_path_str, filename, index);
        snprintf(filepath_png , 400, "%s/%s.%d.fpga.png" , snapshot_imp_path_str, filename, index);
        snprintf(filepath_rgba, 400, "%s/%s.%d.fpga.rgba", snapshot_imp_path_str, filename, index);
    }
    else {
        snprintf(filepath     , 400, "%s/%s.%d.fpga.rgba", snapshot_imp_path_str, filename, index);
    }

//    PRINTF("Taking snapshot to file %s\n", filepath);

    FILE *fp = 0;
    // fp=fopen(filepath, "w+");
    PRINTF("%s: Get snapshot to a file. Need to be implemented\n", __func__);

    if (fp == 0) {
        PRINTF("Error opening file %s\n", filepath);
        return;
    }

    uint8_t  *fb8  = ( uint8_t  *)fb_basevirt;
    uint16_t *fb16 = ( uint16_t *)fb_basevirt;
    uint32_t *fb32 = ( uint32_t *)fb_basevirt;

    int pix = 0;
    uint32_t color;


    switch(fb_format){
    case NEMA_XRGB8888 :
    case NEMA_ARGB8888 :
                    for (pix = 0; pix < fb_size/4; ++pix) {
                        color = col2rgba(fb32[pix],fb_format);
                        PRINTF("%s: Write color %x data to snapshot file. Need to be implemented\n", __func__, color);
                    }
                    break;
    case NEMA_RGBA5650 :
    case NEMA_RGBA5551 :
    case NEMA_RGBA4444 :
                    for (pix = 0; pix < fb_size/2; ++pix) {
                        color = col2rgba(fb16[pix],fb_format);
                        PRINTF("%s: Write color %x data to snapshot file. Need to be implemented\n", __func__, color);
                    }
                    break;
    case NEMA_L8 :
                    for (pix = 0; pix < fb_size; ++pix) {
                        color = col2rgba(fb8[pix],fb_format);
                        PRINTF("%s: Write color %x data to snapshot file. Need to be implemented\n", __func__, color);
                    }
                    break;
    // default          PRINTF("%s: Write data to snapshot file. Need to be implemented\n", __func__);
    }

    // fclose(fp);
}

void snapshot_format(int format, void *base_virt, int image_size) {
    fb_format   =  format;
    fb_basevirt = base_virt;
    fb_size     = image_size;
}

// Define Graphics Memory snapshot path - file path defined as argument
//-----------------------------------------------------------------------------------------------------------------------
void snapshot_imp_path(char * filepath)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Define Graphics Memory implementation image path. Need to be implemented\n", __func__);
}


// Define Graphics Memory snapshot path - file path defined as argument
//-----------------------------------------------------------------------------------------------------------------------
void snapshot_ref_path(char * filepath)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Define Graphics Memory reference image path. Need to be implemented\n", __func__);
}

// Force image into Graphics Memory
//-----------------------------------------------------------------------------------------------------------------------
void fast_image_load(char * filename, int resx, int resy, int offset, int mode_size)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Need to be implemented\n", __func__);
}

// Force Data load into Graphics Memory
//-----------------------------------------------------------------------------------------------------------------------
void fast_data_load(const char * filename, int size, int offset, int b32)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Need to be implemented\n", __func__);
}


// Clear (set all-white) Graphics Memory
//-----------------------------------------------------------------------------------------------------------------------
void fast_clr_fb(int baseaddr, int resx, int resy, int color)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Clear (set all-white) Graphics Memory. Need to be implemented\n", __func__);
}

// Set VGA2BMP Index number
//-----------------------------------------------------------------------------------------------------------------------
void tb_VGA2BMPindex(int index)
{
    PRINTF("%s: not supported\n", __func__);
}

// Set testbench clocks
//-----------------------------------------------------------------------------------------------------------------------
void tb_set_clk_frequency(unsigned int frequency){
    PRINTF("%s: Need to be implemented\n", __func__);
}

void tb_set_clks(unsigned int clocks){
    PRINTF("%s: Need to be implemented\n", __func__);
}

unsigned int tb_get_clk_frequency(){
    PRINTF("%s: Need to be implemented\n", __func__);
    return -1;
}

// Enables AHB delay for reads on iMemCtrl1 AHB memory
void tb_set_ahb_delay(unsigned int delay){
    PRINTF("%s: Need to be implemented\n", __func__);
    return;
}

unsigned int tb_get_ahb_delay(){
    PRINTF("%s: Need to be implemented\n", __func__);
    return -1;
}

// Compare Images with Reference Images
//-----------------------------------------------------------------------------------------------------------------------
int cmp_image(char * filename, int ref_index, int imp_index)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Compare reference with implementation image. To be implemented.\n", __func__);
    return -1;
}

// Compare written values with  Reference txt
//-----------------------------------------------------------------------------------------------------------------------
int cmp_insns(char *filename, void *base_addr, int num)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: To be implemented.\n", __func__);
    return -1;
}

// End the Simulation
//-----------------------------------------------------------------------------------------------------------------------
void theend()
//-----------------------------------------------------------------------------------------------------------------------
{
   PRINTF("%s: not supported\n", __func__);
}

// Show the current time and difference from last invocation
//-----------------------------------------------------------------------------------------------------------------------
void showtime()
//-----------------------------------------------------------------------------------------------------------------------
{
   PRINTF("%s: not supported\n", __func__);
}

// Reads data from Opened filename
//-----------------------------------------------------------------------------------------------------------------------
int read_file(void)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Need to be implemented\n", __func__);
    return 1;
}

// Close Filename for reading
//-----------------------------------------------------------------------------------------------------------------------
void read_closefile(void)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Need to be implemented\n", __func__);
}

// Open a file for Writing
//-----------------------------------------------------------------------------------------------------------------------
void write_openfile(char * text)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Need to be implemented\n", __func__);
}

// Write a byte on the previously opened file
//-----------------------------------------------------------------------------------------------------------------------
void write_file(int byte)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Need to be implemented\n", __func__);
}

// Closes Opened File
//-----------------------------------------------------------------------------------------------------------------------
void write_closefile(void)
//-----------------------------------------------------------------------------------------------------------------------
{
    PRINTF("%s: Need to be implemented\n", __func__);
}

// Delay Ticks
//--------------------------------------------------
void delay(int ticks) {
//--------------------------------------------------
    PRINTF("%s: Need to be implemented\n", __func__);
}

// Get Test Number
//--------------------------------------------------
int tb_testno(void) {
    PRINTF("%s: Need to be implemented\n", __func__);
    return 0;
}
//--------------------------------------------------

// Get Destination Format Number
//--------------------------------------------------
int tb_testmode(void) {
    PRINTF("%s: Need to be implemented\n", __func__);
    return 0;
}
//--------------------------------------------------


// Tb-Assist Read General Purpose Input value
//--------------------------------------------------
unsigned int tb_read_gpi(void) {
    PRINTF("%s: Need to be implemented\n", __func__);
    return 0;
}
//--------------------------------------------------

// Tb-Assist Read General Purpose Output value
//--------------------------------------------------
unsigned int tb_read_gpo(void) {
    PRINTF("%s: Need to be implemented\n", __func__);
    return 0;
}
//--------------------------------------------------

// Tb-Assist Read General Purpose Output value
//--------------------------------------------------
void tb_set_gpo(unsigned int gpo) {
    PRINTF("%s: Need to be implemented\n", __func__);
}

// Tb-Assist Set Panel model ResX ResY
//-----------------------------------------------------------------------------------------------------------------------
void tb_panel_resolution(int resx, int resy)
{
    PRINTF("%s: Need to be implemented\n", __func__);
}

int test_report(int result,const char *text,...) {
    tsi_print("%d: ", result);

    va_list va;
    va_start(va, text);
    vprintf(text, va);
    va_end(va);

    return 0;
}

int test_check (int crc_ok, char* filename, unsigned int index,unsigned int tb_testno){

    if (crc_ok) {
        test_report( crc_ok , "\033[1;32m*** TEST %u PASS *** :)\033[0m \n",tb_testno);
    }else{
        test_report( crc_ok , "\033[1;31m*** TEST %u FAILURE *** :(\033[0m \n",tb_testno);
    }

    return crc_ok;

}

#endif // _TSI_FPGA_ASSIST_C_
