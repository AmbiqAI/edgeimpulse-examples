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
COMPILERNAME := Keil6
PROJECT := libam_bsp_Keil6
CONFIG := bin
AM_SoftwareRoot ?= ../../../..

SHELL:=/bin/bash
#### Required Executables ####
K := $(shell type -p UV5.exe)
RM := $(shell which rm 2>/dev/null)

ifeq ($(K),)
all clean:
	$(info Tools w/$(COMPILERNAME) not installed.)
	$(RM) -rf bin
else

LIBS = 
INCS = ../../../../third_party/ThinkSi/NemaGFX_SDK/include/tsi/common
INCS+= ../../../../third_party/ThinkSi/NemaGFX_SDK/NemaGFX
INCS+= ../../../../mcu/apollo5b/hal
INCS+= ../../../../third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaGFX
INCS+= ../../../../third_party/ThinkSi/config/apollo5b_nemagfx
INCS+= ../../../../CMSIS/ARM/Include
INCS+= ../../../../CMSIS/AmbiqMicro/Include
INCS+= ../../../../devices
INCS+= ../../../../mcu/apollo5b
INCS+= ../../../../third_party/ThinkSi/NemaGFX_SDK/common/mem
INCS+= ../../../../utils
INCS+= ../../../../mcu/apollo5b/hal/mcu
INCS+= ../../../../third_party/ThinkSi/NemaGFX_SDK/NemaDC
INCS+= ../../../../third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaDC

all: directories $(CONFIG)/$(TARGET).lib

# Source Dependencies must be defined before they are used.
SRCS = .././am_bsp_pins.c
SRCS += .././am_bsp.c

$(CONFIG)/$(TARGET).lib: $(LIBS) $(INCS) $(SRCS)
	UV5.exe -b -t "libam_bsp" libam_bsp.uvprojx -j0 || [ $$? -eq 1 ]

directories: $(CONFIG)

$(CONFIG):
	@mkdir -p $@

# BSP's need this.
#### BUILD BEGIN ####

${bspincbuild}

${halincbuild}

#### BUILD END ####

clean:
	@echo Cleaning... ;\
	$(RM) -rf $(CONFIG)

endif
.PHONY: all clean directories

