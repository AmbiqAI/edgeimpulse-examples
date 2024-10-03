PLATFORM?=zc70x_linux

NEMAGFX_SDK_PATH?=../../..

C_SRCS +=main.c

CFLAGS+=-D_POSIX_C_SOURCE=199309L -DSTANDALONE

################
# User Defines #
################

CFLAGS+=-DRESX=800
CFLAGS+=-DRESY=600

CFLAGS+=-DUSE_1BIT_FONT=1
CFLAGS+=-DUSE_2BIT_FONT=1
CFLAGS+=-DUSE_4BIT_FONT=1
CFLAGS+=-DUSE_8BIT_FONT=1

################


include $(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)/common/make.mk

