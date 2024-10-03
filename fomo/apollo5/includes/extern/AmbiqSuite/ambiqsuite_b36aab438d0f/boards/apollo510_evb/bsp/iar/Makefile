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
COMPILERNAME := iar
PROJECT := libam_bsp_iar
CONFIG := bin
AM_SoftwareRoot ?= ../../..

SHELL:=/bin/bash
#### Required Executables ####
K := $(shell type -p IarBuild.exe)
RM = $(shell which rm 2>/dev/null)

ifeq ($(K),)
all clean:
	$(info Tools w/$(COMPILERNAME) not installed.)
	$(RM) -rf bin
else

LIBS = ${libraries}
INCS = ${incs}

all: directories $(CONFIG)/$(TARGET).a

# Source Dependencies must be defined before they are used.
SRCS = .././am_bsp_pins.c
SRCS += .././am_bsp.c

$(CONFIG)/$(TARGET).a: $(LIBS) $(INCS) $(SRCS)
	IarBuild.exe libam_bsp.ewp -make Debug -log info

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
	IarBuild.exe libam_bsp.ewp -clean Debug -log all

endif
.PHONY: all clean directories

