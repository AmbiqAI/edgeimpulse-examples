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
// FILE NAME  : Sw/Src/assist.c
// KEYWORDS   :
// PROJECT    : OpenVG
// PURPOSE    : Assist functions for debugging (open/close files, dump Graphics Memory, etc)
//              Works with Assist.v
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

#ifndef _TSI_FPGA_ASSIST_C_
#define _TSI_FPGA_ASSIST_C_
//-------------------------------------------
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdarg.h>

#include "nema_graphics.h"
#include "assist.h"

//-------------------------------------------
#define DEBUG_TEXT
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
static int total_tests  = 0;
static int pass_tests   = 0;
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

//// Gets a comparison return value
////-----------------------------------------------------------------------------------------------------------------------
//int get_return_value(void)
////-----------------------------------------------------------------------------------------------------------------------
//{
//    return (PTR_TEXTPRINT->extra4==PASS_VALUE);
//}
////-----------------------------------------------------------------------------------------------------------------------


// Prints text on the console
//-----------------------------------------------------------------------------------------------------------------------
void print_t(char * text)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    printf("%s\n", text);
#endif
}

// Same as print_t without new line character
//-----------------------------------------------------------------------------------------------------------------------
extern void aprint(const char * text)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    printf("%s", text);
#endif
}

// To be removed
//-----------------------------------------------------------------------------------------------------------------------
void nprint(int number)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    printf("%08x\n", number);
#endif
}

// Print a Hex number on screen
//-----------------------------------------------------------------------------------------------------------------------
void printh(int number)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    printf("%08x\n", number);
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
    printf("%d\n", number);
#endif
}

// Print a decimal number on screen
//-----------------------------------------------------------------------------------------------------------------------
void printd(int number)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifdef DEBUG_TEXT
    printf("%d\n", number);
#endif
}


// Get a snapshot of Graphics Memory - filename is generated automatically (Assist.v)
//-----------------------------------------------------------------------------------------------------------------------
void snapshot(void)
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("%s: not supported\n", __func__);
//    PTR_TEXTPRINT->character=0x0000AE22;
}
//-----------------------------------------------------------------------------------------------------------------------

static char snapshot_imp_path_str[400];
static char snapshot_ref_path_str[400];


// Coverts RGBA8888 to other color formats
//-----------------------------------------------------------------------------------------------------------------------
inline unsigned col2rgba(uint32_t  col_in, int col_mode) {
    int r,g,b,a;
    unsigned col_out;
    char *ccol_out = (char *)&col_out;

    switch(col_mode){
    case NEMA_RGBX8888 :
    case NEMA_RGBA8888 :
                       col_out = col_in;                   break;
    case NEMA_XRGB8888 :
    case NEMA_ARGB8888 :
                       b = (col_in >> 24)&0xff;
                       g = (col_in >> 16)&0xff;
                       r = (col_in >>  8)&0xff;
                       a = (col_in >>  0)&0xff;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_BGRA8888 :
    case NEMA_BGRX8888 :
                       a = (col_in >> 24)&0xff;
                       r = (col_in >> 16)&0xff;
                       g = (col_in >>  8)&0xff;
                       b = (col_in >>  0)&0xff;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_ABGR8888 :
    case NEMA_XBGR8888 :
                       r = (col_in >> 24)&0xff;
                       g = (col_in >> 16)&0xff;
                       b = (col_in >>  8)&0xff;
                       a = (col_in >>  0)&0xff;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_RGB24    :
                       b = (col_in>>16)&0xff;
                       g = (col_in>> 8)&0xff;
                       r = (col_in>> 0)&0xff;
                       col_out = (r<<24|g<<16|b<<8| 0xff); break;
    case NEMA_BGR24    :
                       r = (col_in>>16)&0xff;
                       g = (col_in>> 8)&0xff;
                       b = (col_in>> 0)&0xff;
                       col_out = (r<<24|g<<16|b<<8| 0xff); break;
    case NEMA_RGBA5650 :
                       r = (col_in>>11)&0x1f;
                       g = (col_in>> 5)&0x3f;
                       b = (col_in    )&0x1f;
                       r=r<<3|r>>2;
                       g=g<<2|g>>4;
                       b=b<<3|b>>2;
                       col_out = (r<<24|g<<16|b<<8| 0xff); break;
    case NEMA_BGRA5650 :
                       b = (col_in>>11)&0x1f;
                       g = (col_in>> 5)&0x3f;
                       r = (col_in    )&0x1f;
                       r=r<<3|r>>2;
                       g=g<<2|g>>4;
                       b=b<<3|b>>2;
                       col_out = (r<<24|g<<16|b<<8| 0xff); break;
    case NEMA_RGBA5551 :
                       r = (col_in>>11)&0x1f;
                       g = (col_in>> 6)&0x1f;
                       b = (col_in>> 1)&0x1f;
                       a = (col_in    )&0x01;
                       r=r<<3|r>>2;
                       g=g<<3|g>>2;
                       b=b<<3|b>>2;
                       a=a*0xff;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_BGRA5551 :
                       b = (col_in>>11)&0x1f;
                       g = (col_in>> 6)&0x1f;
                       r = (col_in>> 1)&0x1f;
                       a = (col_in    )&0x01;
                       r=r<<3|r>>2;
                       g=g<<3|g>>2;
                       b=b<<3|b>>2;
                       a=a*0xff;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_ARGB1555 :
                       r = (col_in>>10)&0x1f;
                       g = (col_in>> 5)&0x1f;
                       b = (col_in>> 0)&0x1f;
                       a = (col_in>>15)&0x01;
                       r=r<<3|r>>2;
                       g=g<<3|g>>2;
                       b=b<<3|b>>2;
                       a=a*0xff;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_ABGR1555 :
                       b = (col_in>>10)&0x1f;
                       g = (col_in>> 5)&0x1f;
                       r = (col_in>> 0)&0x1f;
                       a = (col_in>>15)&0x01;
                       r=r<<3|r>>2;
                       g=g<<3|g>>2;
                       b=b<<3|b>>2;
                       a=a*0xff;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_RGBA4444 :
                       r = (col_in>>12)&0xf;
                       g = (col_in>> 8)&0xf;
                       b = (col_in>> 4)&0xf;
                       a = (col_in    )&0xf;
                       r=r<<4|r;
                       g=g<<4|g;
                       b=b<<4|b;
                       a=a<<4|a;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_BGRA4444 :
                       b = (col_in>>12)&0xf;
                       g = (col_in>> 8)&0xf;
                       r = (col_in>> 4)&0xf;
                       a = (col_in    )&0xf;
                       r=r<<4|r;
                       g=g<<4|g;
                       b=b<<4|b;
                       a=a<<4|a;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_ARGB4444 :
                       a = (col_in>>12)&0xf;
                       r = (col_in>> 8)&0xf;
                       g = (col_in>> 4)&0xf;
                       b = (col_in    )&0xf;
                       r=r<<4|r;
                       g=g<<4|g;
                       b=b<<4|b;
                       a=a<<4|a;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_ABGR4444 :
                       a = (col_in>>12)&0xf;
                       b = (col_in>> 8)&0xf;
                       g = (col_in>> 4)&0xf;
                       r = (col_in    )&0xf;
                       r=r<<4|r;
                       g=g<<4|g;
                       b=b<<4|b;
                       a=a<<4|a;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_AL88     :
                       g = (col_in>> 0)&0xff;
                       a = (col_in>> 8)&0xff;
                       col_out = (g<<24|g<<16|g<<8| a);    break;
    case NEMA_RGBA2222 :
                       r = (col_in>> 6)&0x3;
                       g = (col_in>> 4)&0x3;
                       b = (col_in>> 2)&0x3;
                       a = (col_in>> 0)&0x3;
                       r=r<<6|r<<4|r<<2|r;
                       g=g<<6|g<<4|g<<2|g;
                       b=b<<6|b<<4|b<<2|b;
                       a=a<<6|a<<4|a<<2|a;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_ARGB2222 :
                       r = (col_in>> 4)&0x3;
                       g = (col_in>> 2)&0x3;
                       b = (col_in>> 0)&0x3;
                       a = (col_in>> 6)&0x3;
                       r=r<<6|r<<4|r<<2|r;
                       g=g<<6|g<<4|g<<2|g;
                       b=b<<6|b<<4|b<<2|b;
                       a=a<<6|a<<4|a<<2|a;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_BGRA2222 :
                       r = (col_in>> 2)&0x3;
                       g = (col_in>> 4)&0x3;
                       b = (col_in>> 6)&0x3;
                       a = (col_in>> 0)&0x3;
                       r=r<<6|r<<4|r<<2|r;
                       g=g<<6|g<<4|g<<2|g;
                       b=b<<6|b<<4|b<<2|b;
                       a=a<<6|a<<4|a<<2|a;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_ABGR2222 :
                       r = (col_in>> 0)&0x3;
                       g = (col_in>> 2)&0x3;
                       b = (col_in>> 4)&0x3;
                       a = (col_in>> 6)&0x3;
                       r=r<<6|r<<4|r<<2|r;
                       g=g<<6|g<<4|g<<2|g;
                       b=b<<6|b<<4|b<<2|b;
                       a=a<<6|a<<4|a<<2|a;
                       col_out = (r<<24|g<<16|b<<8| a);    break;
    case NEMA_RGBA0800 :
                       col_out = col_in;                   break;
    case NEMA_RGBA0008 :
                       col_out = col_in;                   break;
    case NEMA_L8       :
                       g = (col_in&0xff);
                       col_out = (g<<24|g<<16|g<<8| 0xff); break;
    case NEMA_AL44     :
                       g = (col_in>> 0)&0xf;
                       a = (col_in>> 4)&0xf;
                       g=g<<4|g;
                       a=a<<4|a;
                       col_out = (g<<24|g<<16|g<<8| a);    break;
    case NEMA_RGBA3320 :
                       col_out = col_in;                   break;
    case NEMA_BW1      :
                       col_out = col_in;                   break;
    default            :
                       col_out = col_in;                   break;
    }

    col_out = ccol_out[0] << 24 | ccol_out[1] << 16 | ccol_out[2] << 8 | ccol_out[3];

    return col_out;
}

// Get a snapshot of Graphics Memory - filename defined as argument (Assist.v)
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
    else if( fb_format == (int)NEMA_TSC12 ) {
        snprintf(filepath     , 400, "%s/%s.%d.fpga.tsc12" , snapshot_imp_path_str, filename, index);
        snprintf(filepath_png , 400, "%s/%s.%d.fpga.png" , snapshot_imp_path_str, filename, index);
        snprintf(filepath_rgba, 400, "%s/%s.%d.fpga.rgba", snapshot_imp_path_str, filename, index);
    }
   else if( fb_format == (int)NEMA_TSC12A ) {
        snprintf(filepath     , 400, "%s/%s.%d.fpga.tsc12a" , snapshot_imp_path_str, filename, index);
        snprintf(filepath_png , 400, "%s/%s.%d.fpga.png" , snapshot_imp_path_str, filename, index);
        snprintf(filepath_rgba, 400, "%s/%s.%d.fpga.rgba", snapshot_imp_path_str, filename, index);
    }
    else {
        snprintf(filepath     , 400, "%s/%s.%d.fpga.rgba", snapshot_imp_path_str, filename, index);
    }

//    printf("Taking snapshot to file %s\n", filepath);

    FILE *fp;
    fp=fopen(filepath, "w+");
    if (fp == 0) {
        printf("Error opening file %s\n", filepath);
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
    case NEMA_BGRA8888 :
    case NEMA_BGRX8888 :
    case NEMA_ABGR8888 :
    case NEMA_XBGR8888 :
                    for (pix = 0; pix < fb_size/4; ++pix) {
                        color = col2rgba(fb32[pix],fb_format);
                        fwrite(&color, 4, 1, fp);
                    }
                    break;
    case NEMA_RGB24    :
    case NEMA_BGR24    :
                    printf("fb_size: %d\n", fb_size);
                    for (pix = 0; pix < fb_size/4; pix+=3) {
                        // printf("pix: %d\n", pix);
                        color = col2rgba(fb32[pix+0],fb_format);
                        fwrite(&color, 4, 1, fp);

                        color = col2rgba( ((fb32[pix+1] & 0xffff) << 8) | ((fb32[pix+0] >> 24) & 0xff),fb_format);
                        fwrite(&color, 4, 1, fp);

                        color = col2rgba( ((fb32[pix+2] & 0xff)  << 16) | ((fb32[pix+1] >> 16) & 0xffff) ,fb_format);
                        fwrite(&color, 4, 1, fp);

                        color = col2rgba(((fb32[pix+2] >> 8) & 0xffffff),fb_format);
                        fwrite(&color, 4, 1, fp);
                    }
                    break;
    case NEMA_RGBA5650 :
    case NEMA_BGRA5650 :
    case NEMA_RGBA5551 :
    case NEMA_ARGB1555 :
    case NEMA_BGRA5551 :
    case NEMA_ABGR1555 :
    case NEMA_RGBA4444 :
    case NEMA_ARGB4444 :
    case NEMA_BGRA4444 :
    case NEMA_ABGR4444 :
    case NEMA_AL88     :
                    for (pix = 0; pix < fb_size/2; ++pix) {
                        color = col2rgba(fb16[pix],fb_format);
                        fwrite(&color, 4, 1, fp);
                    }
                    break;
    case NEMA_L8       :
    case NEMA_AL44     :
    case NEMA_RGBA2222 :
    case NEMA_ARGB2222 :
    case NEMA_BGRA2222 :
    case NEMA_ABGR2222 :
                    for (pix = 0; pix < fb_size; ++pix) {
                        color = col2rgba(fb8[pix],fb_format);
                        fwrite(&color, 4, 1, fp);
                    }
                    break;
    default          :  fwrite(fb32, fb_size, 1, fp); break;
    }

    fclose(fp);
}

void snapshot_format(int format, void *base_virt, int image_size) {
    fb_format   =  format;
    fb_basevirt = base_virt;
    fb_size     = image_size;
}

// Define Graphics Memory snapshot path - file path defined as argument (Assist.v)
//-----------------------------------------------------------------------------------------------------------------------
void snapshot_imp_path(char * filepath)
//-----------------------------------------------------------------------------------------------------------------------
{
    ts_strcpy(snapshot_imp_path_str, filepath);
}


// Define Graphics Memory snapshot path - file path defined as argument (Assist.v)
//-----------------------------------------------------------------------------------------------------------------------
void snapshot_ref_path(char * filepath)
//-----------------------------------------------------------------------------------------------------------------------
{
    ts_strcpy(snapshot_ref_path_str, filepath);
}

// Force image into Graphics Memory (Assist.v)
//-----------------------------------------------------------------------------------------------------------------------
void fast_image_load(char * filename, int resx, int resy, int offset, int mode_size)
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("%s: not supported\n", __func__);
}

// Force Data load into Graphics Memory (Assist.v)
//-----------------------------------------------------------------------------------------------------------------------
void fast_data_load(const char * filename, int size, int offset, int b32)
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("\n Loading file %s (b32 = %d)\n", filename, b32);

    FILE *fp;
    fp=fopen(filename, "r");
    if (fp == 0) {
        printf("Error opening file %s\n", filename);
        return;
    }

    size_t bytes_read = fread( (void *)(uintptr_t)offset, sizeof(char), size, fp);

    if (bytes_read != (size_t)size)
        printf("Error reading file %s with size %zu bytes_read\n", filename, bytes_read);
    fclose(fp);
}


// Clear (set all-white) Graphics Memory (Assist.v)
//-----------------------------------------------------------------------------------------------------------------------
void fast_clr_fb(int baseaddr, int resx, int resy, int color)
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("%s: not supported\n", __func__);
}

// Set VGA2BMP Index number
//-----------------------------------------------------------------------------------------------------------------------
void tb_VGA2BMPindex(int index)
{
    //printf("%s: not supported\n", __func__);
}

void tb_set_clk_frequency(unsigned int frequency){
    printf("%s: not supported\n", __func__);
}

void tb_set_clks(unsigned int clocks){
    printf("%s: not supported\n", __func__);
}

unsigned int tb_get_clk_frequency(){
    printf("%s: not supported\n", __func__);
    return -1;
}

// Enables AHB delay for reads on iMemCtrl1 AHB memory
void tb_set_ahb_delay(unsigned int delay){
    return;
}

unsigned int tb_get_ahb_delay(){
    printf("%s: not supported\n", __func__);
    return -1;
}

// Compare Images with Reference Images
//-----------------------------------------------------------------------------------------------------------------------
int cmp_image(char * filename, int ref_index, int imp_index)
//-----------------------------------------------------------------------------------------------------------------------
{
#ifndef OFFLINE_CHECK
    char cmd[400];
    if( fb_format == (int)NEMA_TSC4 ) {
        printf("Comparing files: %s/%s.%d.ref.tsc4 %s/%s.%d.fpga.tsc4\n", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
        snprintf(cmd, 400, "diff -q %s/%s.%d.ref.tsc4 %s/%s.%d.fpga.tsc4", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
    }
    else if( fb_format == (int)NEMA_TSC6 ) {
        printf("Comparing files: %s/%s.%d.ref.tsc6 %s/%s.%d.fpga.tsc6\n", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
        snprintf(cmd, 400, "diff -q %s/%s.%d.ref.tsc6 %s/%s.%d.fpga.tsc6", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
    }
    else if( fb_format == (int)NEMA_TSC6A ) {
        printf("Comparing files: %s/%s.%d.ref.tsc6a %s/%s.%d.fpga.tsc6a\n", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
        snprintf(cmd, 400, "diff -q %s/%s.%d.ref.tsc6a %s/%s.%d.fpga.tsc6a", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
    }
    else if( fb_format == (int)NEMA_TSC12 ) {
        printf("Comparing files: %s/%s.%d.ref.tsc12 %s/%s.%d.fpga.tsc12\n", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
        snprintf(cmd, 400, "diff -q %s/%s.%d.ref.tsc12 %s/%s.%d.fpga.tsc12", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
    }
    else if( fb_format == (int)NEMA_TSC12A ) {
        printf("Comparing files: %s/%s.%d.ref.tsc12a %s/%s.%d.fpga.tsc12a\n", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
        snprintf(cmd, 400, "diff -q %s/%s.%d.ref.tsc12a %s/%s.%d.fpga.tsc12a", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
    }
    else{
        printf("Comparing files: %s/%s.%d.ref.rgba %s/%s.%d.fpga.rgba\n", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
        snprintf(cmd, 400, "diff -q %s/%s.%d.ref.rgba %s/%s.%d.fpga.rgba", snapshot_ref_path_str, filename, ref_index, snapshot_imp_path_str, filename, imp_index);
    }

    int ret = system(cmd);
    if(ret == 0) {
        printf("\033[1;32m*** TEST PASS *** :)\033[0m  ");
        pass_tests++;
    } else {
        printf("\033[1;31m*** TEST FAILURE *** :(\033[0m  ");
    }

    total_tests++;
    return ret;
#else
    return 0;
#endif
}

// Compare Images with Reference Images
//-----------------------------------------------------------------------------------------------------------------------
int cmp_regress(char *filename, void *base_addr)
//-----------------------------------------------------------------------------------------------------------------------
{
    char cmd[400];

    printf("---------------------------------\n");
    // printf("%s:%d\n", __FILE__, __LINE__);
    printf("Compare %s\n", filename);

    char fpga_result_file[400];


    ts_strcpy(fpga_result_file, snapshot_imp_path_str);
    ts_strcat(fpga_result_file, filename);
    ts_strcat(fpga_result_file, ".fpga.txt");
    ///TODO:
    int *result_index_addr = base_addr;

    // printf("Loading  index value from  physical address %p\n", GRA_virt2phys(base_addr) );

    int result_index = *result_index_addr;
    printf(" Writing %d value(s)\n", result_index);
    FILE *f;
    f = fopen(fpga_result_file, "w");
    if(f != NULL){
        result_index_addr++;
        // printf("Loading  pointer to write value from  physical address %p\n", GRA_virt2phys(result_index_addr) );
        if(result_index == 0) {
            fprintf(f, "error there are no results to read \n");
        } else {
            for(int k = 0; k < result_index; k++) {
                // printf("writing value %08x to file from physical address %p\n", (unsigned int)*result_index_addr, GRA_virt2phys(result_index_addr) );
                fprintf(f, "%8d ** %8u ** %08x\n", (int)*result_index_addr, (unsigned int)*result_index_addr, (unsigned int)*result_index_addr);
                result_index_addr++;
            }
        }
        fclose(f);
    } else {
        printf("\n fopen() Error!!!\n");
        printf("check path privileges\n");
        return 1;
    }
    ///1. save to file
    ///2. compare file

    printf("Comparing files: %s%s.ref.txt %s%s.fpga.txt\n", snapshot_ref_path_str, filename, snapshot_imp_path_str, filename);
    snprintf(cmd, 400, "diff -q  %s%s.ref.txt %s%s.fpga.txt ", snapshot_ref_path_str, filename, snapshot_imp_path_str, filename);
    int ret = system(cmd);

    if(ret == 0) {
        printf("*** TEST PASS ***\n");
        pass_tests++;
    } else {
        printf("*** TEST FAILURE ***\n");
    }

    total_tests++;
    return ret;
}

// Compare written values with  Reference txt
//-----------------------------------------------------------------------------------------------------------------------
int cmp_insns(char *filename, void *base_addr, int num)
//-----------------------------------------------------------------------------------------------------------------------
{
    char cmd[400];

    printf("---------------------------------\n");
    // printf("%s:%d\n", __FILE__, __LINE__);
    printf("Compare %s\n", filename);

    char fpga_result_file[400];


    ts_strcpy(fpga_result_file, snapshot_imp_path_str);
    ts_strcat(fpga_result_file, filename);
    ts_strcat(fpga_result_file, ".fpga.txt");
    ///TODO:
    int *result_index_addr = base_addr;

    // printf("Loading  index value from  physical address %p\n", GRA_virt2phys(base_addr) );

    int result_index = num;
    printf(" Writing %d value(s)\n", result_index);
    FILE *f;
    f = fopen(fpga_result_file, "w");
    if(f != NULL){
        // printf("Loading  pointer to write value from  physical address %p\n", GRA_virt2phys(result_index_addr) );
        if(result_index == 0) {
            fprintf(f, "error there are no results to read \n");
        } else {
            for(int k = 0; k < result_index; k++) {
                fprintf(f, "%8d ** %8u ** %08x\n", (int)*result_index_addr, (unsigned int)*result_index_addr, (unsigned int)*result_index_addr);
                result_index_addr++;
            }
        }
        fclose(f);
    } else {
        printf("\n fopen() Error!!!\n");
        printf("check path privileges\n");
        return 1;
    }
    ///1. save to file
    ///2. compare file

    printf("Comparing files: %s%s.ref.txt %s%s.fpga.txt\n", snapshot_ref_path_str, filename, snapshot_imp_path_str, filename);
    snprintf(cmd, 400, "diff -q  %s%s.ref.txt %s%s.fpga.txt ", snapshot_ref_path_str, filename, snapshot_imp_path_str, filename);
    int ret = system(cmd);

    if(ret == 0) {
        printf("*** TEST PASS ***\n");
        pass_tests++;
    } else {
        printf("*** TEST FAILURE ***\n");
    }

    total_tests++;
    return ret;
}

// End the Simulation
//-----------------------------------------------------------------------------------------------------------------------
void theend()
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("\n\n------------------------------------------------------\n");
    printf("Total  tests: %d\n",total_tests);
    printf("Passed tests: %d\n",pass_tests);
    printf("Failed tests: %d\n",total_tests-pass_tests);
    printf("------------------------------------------------------\n\n");
}

// Show the current time and difference from last invocation
//-----------------------------------------------------------------------------------------------------------------------
void showtime()
//-----------------------------------------------------------------------------------------------------------------------
{
//    printf("%s: not supported\n", __func__);
}

// Opens a Filename for reading
//-----------------------------------------------------------------------------------------------------------------------
void read_openfile(char * text)
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("%s: not supported\n", __func__);
}

// Reads data from Opened filename
//-----------------------------------------------------------------------------------------------------------------------
int read_file(void)
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("%s: not supported\n", __func__);
    return 1;
}

// Close Filename for reading
//-----------------------------------------------------------------------------------------------------------------------
void read_closefile(void)
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("%s: not supported\n", __func__);
}

// Open a file for Writing
//-----------------------------------------------------------------------------------------------------------------------
void write_openfile(char * text)
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("%s: not supported\n", __func__);
}

// Write a byte on the previously opened file
//-----------------------------------------------------------------------------------------------------------------------
void write_file(int byte)
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("%s: not supported\n", __func__);
}

// Closes Opened File
//-----------------------------------------------------------------------------------------------------------------------
void write_closefile(void)
//-----------------------------------------------------------------------------------------------------------------------
{
    printf("%s: not supported\n", __func__);
}

// Delay Ticks
//--------------------------------------------------
void delay(int ticks) {
//--------------------------------------------------
    int i;
    // struct timespec tunit;
    // tunit.tv_sec  = 0;
    // tunit.tv_nsec = 1000;

    for ( i = 0 ; i < ticks ; i++) {
        // nanosleep(&tunit,NULL);
        usleep(1);
    }
}

// Get Test Number
//--------------------------------------------------
int tb_testno(void) {
    return 0;
}
//--------------------------------------------------

// Get Test Number
//--------------------------------------------------
int tb_testmode(void) {
    return 0;
}
//--------------------------------------------------


// Tb-Assist Read General Purpose Input value
//--------------------------------------------------
unsigned int tb_read_gpi(void) {

    return 0;
}
//--------------------------------------------------

// Tb-Assist Read General Purpose Output value
//--------------------------------------------------
unsigned int tb_read_gpo(void) {

    return 0;
}
//--------------------------------------------------

// Tb-Assist Read General Purpose Output value
//--------------------------------------------------
void tb_set_gpo(unsigned int gpo) {

    // PTR_TEXTPRINT->gp_output = gpo;
}

// Tb-Assist Read General Purpose Output value
//--------------------------------------------------
void tb_set_dsi_assist(unsigned int dsi_assist) {

    // PTR_TEXTPRINT->dsi_assist = dsi_assist;
}

// Set Panel ResX ResY
//-----------------------------------------------------------------------------------------------------------------------
void tb_panel_resolution(int resx, int resy)
{

}

// Set Panel Configuration
//-----------------------------------------------------------------------------------------------------------------------
void tb_panel_config(int cfg)
{

}



int test_check (int crc_ok, char* filename, unsigned int index,unsigned int tb_testno){

    if (crc_ok) {
        test_report( crc_ok , "\033[1;32m*** TEST %u PASS *** :)\033[0m \n",tb_testno);
    }else{
        test_report( crc_ok , "\033[1;31m*** TEST %u FAILURE *** :(\033[0m \n",tb_testno);
    }

    return crc_ok;

}

int
test_report(int result,const char *text,...) {

    va_list va;
    va_start(va, text);
    vprintf(text, va);
    va_end(va);

    return 0;
}

#endif // _TSI_FPGA_ASSIST_C_
