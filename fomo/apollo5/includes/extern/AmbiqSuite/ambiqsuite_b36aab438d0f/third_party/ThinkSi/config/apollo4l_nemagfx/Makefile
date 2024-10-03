#******************************************************************************
#
# Makefile - Rules for compiling
#
# ${copyright}
#
# This is part of revision ${version} of the AmbiqSuite Development Package.
#
#******************************************************************************

# All makefiles use this to find the top level directory.
#### BUILD BEGIN ####
ifeq ($(findstring CYGWIN, $(shell uname)), CYGWIN)
    SWROOT?=$(shell cygpath -m $(shell git rev-parse --show-toplevel))
endif
#### BUILD END ####
SWROOT?=$(shell git rev-parse --show-toplevel)
# Include rules for building the HAL.
include $(SWROOT)/makedefs/lib.mk

# Generate pin definitions for apollo3.
CHIP_GENERATION = 3
