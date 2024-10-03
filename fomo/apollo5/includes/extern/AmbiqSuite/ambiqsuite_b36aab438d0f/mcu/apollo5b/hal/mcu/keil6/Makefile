#******************************************************************************
#
# Makefile - Rules for building the libraries, examples and docs.
#
# ${copyright}
#
# This is part of revision ${version} of the AmbiqSuite Development Package.
#
#******************************************************************************
TARGET := libam_hal
COMPILERNAME := Keil6
PROJECT := libam_hal_Keil6
CONFIG := bin
AM_SoftwareRoot ?= ../../../../..

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
INCS = ../../../../../CMSIS/AmbiqMicro/Include
INCS+= ../../../../../CMSIS/ARM/Include
INCS+= ../../../../../utils
INCS+= ../../../../../mcu/apollo5b/hal
INCS+= ../../../../../mcu/apollo5b

all: directories $(CONFIG)/$(TARGET).lib

# Source Dependencies must be defined before they are used.
SRCS = ../../../../../mcu/apollo5b/hal/mcu/am_hal_syspll.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_tpiu.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_mspi.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_mcuctrl.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_utils.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_uart.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_debug.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_rtc.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_iom.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_sysctrl.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_sdhc.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_cachectrl.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_card.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_card_host.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_secure_ota.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_clkgen.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_mram.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_systick.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_cmdq.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_itm.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_mpu.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_interrupt.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_dsi.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_reset.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_bootrom_helper.c
SRCS += ../../../../../mcu/apollo5b/hal/mcu/am_hal_ios.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_dcu.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_infoc.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_audadc.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_wdt.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_access.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_pin.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_usb.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_pwrctrl.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_info.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_stimer.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_security.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_timer.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_clkmgr.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_i2s.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_global.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_gpdma.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_queue.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_pdm.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_gpio.c
SRCS += ../../../../../mcu/apollo5b/hal/am_hal_adc.c

$(CONFIG)/$(TARGET).lib: $(LIBS) $(INCS) $(SRCS)
	UV5.exe -b -t "libam_hal" libam_hal.uvprojx -j0 || [ $$? -eq 1 ]

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

