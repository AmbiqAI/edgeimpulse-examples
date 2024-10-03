# Nema Font Converter

## Building on Linux
Nema Font Converter can be built on Linux using the *Makefile*. The application depends on the Freetype2 library which needs to be installed before attempting to build it.


## Building on Windows
On Windows, Nema Font Converter can be built using the *Makefile.windows*.

### Dependencies

Nema Font Converter depends on the *Freetype 2* library which can be downloaded from [here](https://sourceforge.net/projects/gnuwin32/files/freetype/2.3.5-1/freetype-2.3.5-1-bin.zip).

A suitable toolchain is also needed in order to build and link the application to its dependencies. Such a toochain is [MinGW32](https://sourceforge.net/projects/mingw/).

The application can then be built using the *Makefile.windows*. For example, if you have downloaded both *Freetype 2* and *MinGW32* inside the Nema Font Converter directory you can then open a powershell (in the same directory) and run the following command:

```sh
.\MinGW32\bin\mingw32-make.exe -f .\Makefile.windows
```

If *Freetype 2* and *MinGW32* are located in a different directory, you need to adjust the respective paths in the *Makefile.windows* and call the correct *mingw32-make.exe* executable.

This will create the executable *nema_font_convert.exe*. Attempting to run will initially fail because the executable needs the  *freetype26.dll* and *zlib.dll* (provided by the [Freetype team](https://gitlab.freedesktop.org/freetype/freetype/-/blob/master/docs/FTL.TXT) and [zlib team ( Jean-loup Gailly and  Mark Adler)](https://www.zlib.net/zlib_license.html) respecively). You can copy them inside the directory of  *nema_font_convert.exe* from *Windows_Dependencies* folder, or you can add them to your system PATH. After this step, Nema Font Converter should be ready.
