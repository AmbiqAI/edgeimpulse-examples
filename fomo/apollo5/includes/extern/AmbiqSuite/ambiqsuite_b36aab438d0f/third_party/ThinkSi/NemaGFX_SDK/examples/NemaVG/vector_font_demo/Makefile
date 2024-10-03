PLATFORM?=zc70x_linux

NEMAGFX_SDK_PATH?=../../../

CFLAGS+=-DSTANDALONE

C_SRCS +=DejaVuSerif_ttf_kern.c
# C_SRCS +=DejaVuSans_ttf_kern.c
# C_SRCS +=Whisper_Regular_ttf.c
# C_SRCS +=Caveat_VariableFont_wght_ttf.c
# C_SRCS +=DancingScript_ttf.c

C_SRCS +=main.c

USE_NEMAVG=1
include $(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)/common/make.mk
