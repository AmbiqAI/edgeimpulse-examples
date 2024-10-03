// -----------------------------------------------------------------------------
// Copyright (c) 2022 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//  This file can be modified by OEMs as specified in the license agreement
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#ifndef __NEMA_KB_MAP_INC__
#define __NEMA_KB_MAP_INC__

const char not_shifted_map[] = {
    0  , //0 null
    27 , //1 esc
    '1', //2
    '2', //3
    '3', //4
    '4', //5
    '5', //6
    '6', //7
    '7', //8
    '8', //9
    '9', //10
    '0', //11
    '-', //12
    '=', //13
    8  , //14 backspace
    9  , //15 tab
    'q', //16
    'w', //17
    'e', //18
    'r', //19
    't', //20
    'y', //21
    'u', //22
    'i', //23
    'o', //24
    'p', //25
    '[', //26
    ']', //27
    13 , //28 enter - CR
    0  , //29 left ctrl
    'a', //30
    's', //31
    'd', //32
    'f', //33
    'g', //34
    'h', //35
    'j', //36
    'k', //37
    'l', //38
    ';', //39
    '\'', //40
    '`', //41
    0  , //42 left shift
    '\\', //43
    'z', //44
    'x', //45
    'c', //46
    'v', //47
    'b', //48
    'n', //49
    'm', //50
    ',', //51
    '.', //52
    '/', //53
    0  , //54 right shift
    '*', //55
    0  , //56 left alt
    ' ', //57
    0    //58 caps
};

char shifted_map[] = {
    0  , //0 null
    27 , //1 esc
    '!', //2
    '@', //3
    '#', //4
    '$', //5
    '%', //6
    '^', //7
    '&', //8
    '*', //9
    '(', //10
    ')', //11
    '_', //12
    '+', //13
    8  , //14 backspace
    9  , //15 tab
    'Q', //16
    'W', //17
    'E', //18
    'R', //19
    'T', //20
    'Y', //21
    'U', //22
    'I', //23
    'O', //24
    'P', //25
    '{', //26
    '}', //27
    13 , //28 enter - CR
    0  , //29 left ctrl
    'A', //30
    'S', //31
    'D', //32
    'F', //33
    'G', //34
    'H', //35
    'J', //36
    'K', //37
    'L', //38
    ':', //39
    '"', //40
    '~', //41
    0  , //42 left shift
    '|', //43
    'Z', //44
    'X', //45
    'C', //46
    'V', //47
    'B', //48
    'N', //49
    'M', //50
    '<', //51
    '>', //52
    '?', //53
    0  , //54 right shift
    '*', //55
    0  , //56 left alt
    ' ', //57
    0    //58 caps
};


char nema_kb_map(int code, int value) {
    static int shift = 0;
    static int caps = 0;

    // static const int RELEASED = 0;
    static const int PRESSED = 1;
    static const int HOLD = 2;

    if (code == 42 || code == 54) {
        //left/right shift
        if ((int)value != (int)HOLD) {
            shift = value == PRESSED;
        }
    } else /* if (value != RELEASED) */ {
        if (code == 58) {
            //caps lock
            caps = (caps == 0);
        } else {
            char c;
            if (shift != 0) {
                c = shifted_map[code];
            }
            else {
                c = not_shifted_map[code];
            }

            if (caps != 0) {
                if (c >= 'a' && c <= 'z') {
                    c += 'A'-'a';
                }
                else if (c >= 'A' && c <= 'Z') {
                    c += 'A'-'a';
                } else {

                }
            }

            return c;
        }
    }

    return 0;
}

#endif //__NEMA_KB_MAP_INC__
