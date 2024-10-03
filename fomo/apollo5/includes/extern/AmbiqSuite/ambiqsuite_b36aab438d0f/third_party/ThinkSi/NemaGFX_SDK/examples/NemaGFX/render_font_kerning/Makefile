PLATFORM?=zc70x_linux

NEMAGFX_SDK_PATH?=../../../

C_SRCS +=main.c

CFLAGS+=-D_POSIX_C_SOURCE=199309L -DSTANDALONE

################
# User Defines #
################

CFLAGS+=-DRESX=800
CFLAGS+=-DRESY=600

################


include $(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)/common/make.mk

