PROJECT?=$(strip $(notdir $(CURDIR)))


PLATFORM?=lattice_mico32_no_OS
NEMAGFX_SDK_PATH?=../../

red:=$(shell tput setaf 1)
green:=$(shell tput setaf 2)
yellow:=$(shell tput setaf 3)
blue:=$(shell tput setaf 4)
reset:=$(shell tput sgr0)

$(info $(yellow) NEMAGFX_PLATFORM = $(green)${PLATFORM} $(reset))

INCLUDE_NEMAGFX=$(NEMAGFX_SDK_PATH)/include/tsi/NemaGFX
INCLUDE_NEMADC =$(NEMAGFX_SDK_PATH)/include/tsi/NemaDC
INCLUDE_COMMON =$(NEMAGFX_SDK_PATH)/include/tsi/common
BAREMETAL_PLATFORM=$(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)

PLATFORM_PATH=$(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)

GCC_LM32_VERSION?=9.3.0
BASE_LM32?=/tools/software-pc/gcc-$(GCC_LM32_VERSION)-lm32
export LD_LIBRARY_PATH:=${LD_LIBRARY_PATH}:$(BASE_LM32)/lib

COMPILER=$(BASE_LM32)/bin/lm32-elf-
AS=$(COMPILER)as
CC=$(COMPILER)gcc
LD=$(COMPILER)ld
OBJCOPY=$(COMPILER)objcopy
OBJDUMP=$(COMPILER)objdump

ifneq (,$(findstring nemapvg,$(PLATFORM)))
CFLAGS+= -DNEMAPVG
endif

$(info $(yellow) COMPILER = $(green)${COMPILER} $(reset))

ifdef NEMAGFX_SDK
include $(PLATFORM_PATH)/NemaGFX/nemagfx.mk
S_SRCS      +=$(PLATFORM_PATH)/NemaGFX/boot.s
endif

ifdef USE_NEMAVG
	include $(PLATFORM_PATH)/NemaVG/nemavg.mk
endif

ifdef NEMADC_API
	include $(PLATFORM_PATH)/NemaDC/nemadc.mk
	S_SRCS      +=$(PLATFORM_PATH)/NemaDC/boot.s
endif

INCFLAGS    +=-I$(BASE_LM32)/lib/gcc/lm32-elf/$(GCC_LM32_VERSION)/include
INCFLAGS    +=-I$(BASE_LM32)/lm32-elf/include

LIBFLAGS    +=-L$(BASE_LM32)/lib
LIBFLAGS    +=-L$(BASE_LM32)/lib/gcc/lm32-elf/$(GCC_LM32_VERSION)/mmultiply-enabled/mbarrel-shift-enabled
LIBFLAGS    +=-L$(BASE_LM32)/lm32-elf/lib/mmultiply-enabled/mbarrel-shift-enabled
LIBFLAGS    +=-L$(BASE_LM32)/lm32-elf/lib
LIBFLAGS    +=-L$(BASE_LM32)/lib/gcc/lm32-elf/$(GCC_LM32_VERSION)/


INCPATHS+=$(INCLUDE_NEMAGFX)
INCPATHS+=$(INCLUDE_NEMADC)
INCPATHS+=$(INCLUDE_COMMON)
INCPATHS+=$(BAREMETAL_PLATFORM)/common
INCPATHS+=$(PWD)


INCFLAGS+=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS+=$(foreach TMP,$(LIBPATHS),-L$(TMP))

LDFLAGS=-nostdlib -nodefaultlibs -T$(PLATFORM_PATH)/common/platform.ld
CFLAGS?=-O0
CFLAGS+=-c -Wall -Wno-unused-function
CFLAGS+=-std=c99 -fno-strict-aliasing -nostdinc
CFLAGS+=$(USER_CFLAGS)

#lm32 specific
CFLAGS+=-mbarrel-shift-enabled  -muser-enabled -mmultiply-enabled -mdivide-enabled -msign-extend-enabled

# Automatic generation of some important lists
OBJECTS_S    =$(S_SRCS:.s=.o)
OBJECTS      =$(C_SRCS:.c=.o)
OBJECTS_ALL  =$(OBJECTS_S) $(OBJECTS)

all: $(SOURCES) $(SOURCES_S) $(PROJECT)

%.o : %.c
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@

$(PROJECT):  $(OBJECTS_ALL)
	$(LD) -Map image.map -N $(LDFLAGS) $(LIBFLAGS) $(OBJECTS_ALL) -lm -lc -lgcc -lnosys -o test.obj
	$(OBJCOPY) -Rcomment -O binary test.obj $(PROJECT)
	echo "#############################"
	echo $(PROJECT) " OK"
	echo "#############################"
	$(OBJDUMP) -d test.obj >test.dis

-include $(OBJECTS_ALL:.o=.d)

distclean: clean
		rm -f $(PROJECT)

clean:
		rm -f $(OBJECTS_ALL) $(OBJECTS_ALL:.o=.d) test.obj test.dis
