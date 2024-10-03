#ifndef _GUI_H
#define _GUI_H

#define GUI_USE_ARGB 1

#if (GUI_USE_ARGB)
  #define GUI_BLUE          0xFF0000FF
  #define GUI_GREEN         0xFF00FF00
  #define GUI_RED           0xFFFF0000
  #define GUI_CYAN          0xFF00FFFF
  #define GUI_MAGENTA       0xFFFF00FF
  #define GUI_YELLOW        0xFFFFFF00
  #define GUI_LIGHTBLUE     0xFF8080FF
  #define GUI_LIGHTGREEN    0xFF80FF80
  #define GUI_LIGHTRED      0xFFFF8080
  #define GUI_LIGHTCYAN     0xFF80FFFF
  #define GUI_LIGHTMAGENTA  0xFFFF80FF
  #define GUI_LIGHTYELLOW   0xFFFFFF80
  #define GUI_DARKBLUE      0xFF000080
  #define GUI_DARKGREEN     0xFF008000
  #define GUI_DARKRED       0xFF800000
  #define GUI_DARKCYAN      0xFF008080
  #define GUI_DARKMAGENTA   0xFF800080
  #define GUI_DARKYELLOW    0xFF808000
  #define GUI_WHITE         0xFFFFFFFF
  #define GUI_LIGHTGRAY     0xFFD3D3D3
  #define GUI_GRAY          0xFF808080
  #define GUI_DARKGRAY      0xFF404040
  #define GUI_BLACK         0xFF000000
  #define GUI_BROWN         0xFFA52A2A
  #define GUI_ORANGE        0xFFFFA500
  #define GUI_TRANSPARENT   0x00000000

  #define GUI_GRAY_3F       0xFF3F3F3F
  #define GUI_GRAY_50       0xFF505050
  #define GUI_GRAY_55       0xFF555555
  #define GUI_GRAY_60       0xFF606060
  #define GUI_GRAY_7C       0xFF7C7C7C
  #define GUI_GRAY_9A       0xFF9A9A9A
  #define GUI_GRAY_AA       0xFFAAAAAA
  #define GUI_GRAY_C0       0xFFC0C0C0
  #define GUI_GRAY_C8       0xFFC8C8C8
  #define GUI_GRAY_D0       0xFFD0D0D0
  #define GUI_GRAY_E7       0xFFE7E7E7
  #define GUI_BLUE_98       0xFF000098
#else
  #define GUI_BLUE          0x00FF0000
  #define GUI_GREEN         0x0000FF00
  #define GUI_RED           0x000000FF
  #define GUI_CYAN          0x00FFFF00
  #define GUI_MAGENTA       0x00FF00FF
  #define GUI_YELLOW        0x0000FFFF
  #define GUI_LIGHTBLUE     0x00FF8080
  #define GUI_LIGHTGREEN    0x0080FF80
  #define GUI_LIGHTRED      0x008080FF
  #define GUI_LIGHTCYAN     0x00FFFF80
  #define GUI_LIGHTMAGENTA  0x00FF80FF
  #define GUI_LIGHTYELLOW   0x0080FFFF
  #define GUI_DARKBLUE      0x00800000
  #define GUI_DARKGREEN     0x00008000
  #define GUI_DARKRED       0x00000080
  #define GUI_DARKCYAN      0x00808000
  #define GUI_DARKMAGENTA   0x00800080
  #define GUI_DARKYELLOW    0x00008080
  #define GUI_WHITE         0x00FFFFFF
  #define GUI_LIGHTGRAY     0x00D3D3D3
  #define GUI_GRAY          0x00808080
  #define GUI_DARKGRAY      0x00404040
  #define GUI_BLACK         0x00000000
  #define GUI_BROWN         0x002A2AA5
  #define GUI_ORANGE        0x0000A5FF
  #define GUI_TRANSPARENT   0xFF000000

  #define GUI_GRAY_3F       0x003F3F3F
  #define GUI_GRAY_50       0x00505050
  #define GUI_GRAY_55       0x00555555
  #define GUI_GRAY_60       0x00606060
  #define GUI_GRAY_7C       0x007C7C7C
  #define GUI_GRAY_9A       0x009A9A9A
  #define GUI_GRAY_AA       0x00AAAAAA
  #define GUI_GRAY_C0       0x00C0C0C0
  #define GUI_GRAY_C8       0x00C8C8C8
  #define GUI_GRAY_D0       0x00D0D0D0
  #define GUI_GRAY_E7       0x00E7E7E7
  #define GUI_BLUE_98       0x00980000
#endif


#endif
