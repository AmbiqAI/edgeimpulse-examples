#******************************************************************************
#
# Makefile - Rules for compiling
#
# ${copyright}
#
# This is part of revision ${version} of the AmbiqSuite Development Package.
#
#******************************************************************************

# Include rules specific to this board
-include ../board-defs.mk

# All makefiles use this to find the top level directory.
#### BUILD BEGIN ####
ifeq ($(findstring CYGWIN, $(shell uname)), CYGWIN)
    SWROOT?=$(shell cygpath -m $(shell git rev-parse --show-toplevel))
endif
#### BUILD END ####
SWROOT?=$(shell git rev-parse --show-toplevel)

# Include rules for building the BSP.
include $(SWROOT)/makedefs/am_bsp.mk
