#******************************************************************************
#
# Makefile - Rules for building the libraries, examples and docs.
#
# ${copyright}
#
# This is part of revision ${version} of the AmbiqSuite Development Package.
#
#******************************************************************************
TARGET := libam_bsp
COMPILERNAME := gcc
PROJECT := libam_bsp_gcc
CONFIG := bin

VERBOSE ?= 0

SHELL:=/bin/bash

# Enable printing explicit commands with 'make VERBOSE=1'
ifneq ($(VERBOSE),1)
Q:=@
endif

#### Setup ####

TOOLCHAIN ?= arm-none-eabi
PART = apollo5b
CPU = cortex-m55
FPU = auto
# Default to FPU hardware calling convention.  However, some customers and/or
# applications may need the software calling convention.
#FABI = softfp
FABI = hard

#### Required Executables ####
CC = $(TOOLCHAIN)-gcc
GCC = $(TOOLCHAIN)-gcc
CPP = $(TOOLCHAIN)-cpp
LD = $(TOOLCHAIN)-ld
CP = $(TOOLCHAIN)-objcopy
OD = $(TOOLCHAIN)-objdump
RD = $(TOOLCHAIN)-readelf
AR = $(TOOLCHAIN)-ar
SIZE = $(TOOLCHAIN)-size
RM = $(shell which rm 2>/dev/null)

EXECUTABLES = CC LD CP OD AR RD SIZE GCC
K := $(foreach exec,$(EXECUTABLES),\
        $(if $(shell which $($(exec)) 2>/dev/null),,\
        $(info $(exec) not found on PATH ($($(exec))).)$(exec)))
$(if $(strip $(value K)),$(info Required Program(s) $(strip $(value K)) not found))

ifneq ($(strip $(value K)),)
all clean:
	$(info Tools $(TOOLCHAIN)-$(COMPILERNAME) not installed.)
	$(RM) -rf bin
else

DEFINES = -DAM_PACKAGE_BGA
DEFINES+= -DAM_PART_APOLLO5B
DEFINES+= -DNEMA_PLATFORM=apollo5b_nemagfx
DEFINES+= -Dgcc

INCLUDES = -I../../../../CMSIS/ARM/Include
INCLUDES+= -I../../../../CMSIS/AmbiqMicro/Include
INCLUDES+= -I../../../../devices
INCLUDES+= -I../../../../mcu/apollo5b
INCLUDES+= -I../../../../mcu/apollo5b/hal
INCLUDES+= -I../../../../mcu/apollo5b/hal/mcu
INCLUDES+= -I../../../../third_party/ThinkSi/NemaGFX_SDK/NemaDC
INCLUDES+= -I../../../../third_party/ThinkSi/NemaGFX_SDK/NemaGFX
INCLUDES+= -I../../../../third_party/ThinkSi/NemaGFX_SDK/common/mem
INCLUDES+= -I../../../../third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaDC
INCLUDES+= -I../../../../third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaGFX
INCLUDES+= -I../../../../third_party/ThinkSi/NemaGFX_SDK/include/tsi/common
INCLUDES+= -I../../../../third_party/ThinkSi/config/apollo5b_nemagfx
INCLUDES+= -I../../../../utils

VPATH = ..

SRC = am_bsp.c
SRC += am_bsp_pins.c

CSRC = $(filter %.c,$(SRC))
ASRC = $(filter %.s,$(SRC))

OBJS = $(CSRC:%.c=$(CONFIG)/%.o)
OBJS+= $(ASRC:%.s=$(CONFIG)/%.o)

DEPS = $(CSRC:%.c=$(CONFIG)/%.d)
DEPS+= $(ASRC:%.s=$(CONFIG)/%.d)

CFLAGS = -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
CFLAGS+= -ffunction-sections -fdata-sections
CFLAGS+= -MMD -MP -std=c99 -Wall -g
# Libraries O3 for production, examples O0 for debug.
CFLAGS+= -O3
#### INTERNAL BEGIN ####
CFLAGS+= -g -ggdb
#### INTERNAL END ####
CFLAGS+= $(DEFINES)
CFLAGS+= $(INCLUDES)
CFLAGS+= 

#
# Options to generate assembly listing with the source code.
# -g:   Produce debugging information
# -Wa,: Pass option to the assembler
# -a:   Turn on high-level assembly and symbols listing (must preceed the following options)
#  d:   Omit debugging directives
#  n:   Omit forms processing
#  h:   Include high-level source
#  l:   Include assembly
#  s:   Include symbol table
# Uncomment the next line to generate listing files.
#EXTRA_CFLAGS+= -Wa,-adhlns="$(CONFIG)/$*.lst"

# Additional user specified CFLAGS
CFLAGS+=$(EXTRA_CFLAGS)

ODFLAGS = -S

#### Rules ####
all: directories $(CONFIG)/$(TARGET).a

directories: $(CONFIG)

$(CONFIG):
	@mkdir -p $@

$(CONFIG)/%.o: %.c $(CONFIG)/%.d $(INCS)
	@echo " Compiling $(COMPILERNAME) $<"
	$(Q) $(CC) -c $(CFLAGS) $< -o $@

$(CONFIG)/%.o: %.s $(CONFIG)/%.d $(INCS)
	@echo " Assembling $(COMPILERNAME) $<"
	$(Q) $(CC) -c $(CFLAGS) $< -o $@

$(CONFIG)/$(TARGET).a: $(OBJS)
	@echo " Library $(COMPILERNAME) $@"
	$(Q) $(AR) rsvc $@ $(OBJS)

clean:
	@echo "Cleaning..."
	$(Q) $(RM) -f $(OBJS) $(DEPS) \
	    $(CONFIG)/$(TARGET).a

$(CONFIG)/%.d: ;

# Automatically include any generated dependencies
-include $(DEPS)
endif
.PHONY: all clean directories

