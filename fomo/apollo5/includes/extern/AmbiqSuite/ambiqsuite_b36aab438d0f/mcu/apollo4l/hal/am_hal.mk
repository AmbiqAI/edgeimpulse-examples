#******************************************************************************
#
# am_hal.mk - Rules for building Ambiq support libraries.
#
# ${copyright}
#
# This is part of revision ${version} of the AmbiqSuite Development Package.
#
#******************************************************************************

#### BUILD BEGIN ####
# Internally, we'll use "buildproj" as the default target.
SCRIPTROOT?=$(SWROOT)/scripts/buildproj
BUILDPROJARGS?=

.PHONY: buildproj
buildproj: config.ini
	$(SCRIPTROOT)/buildproj.py $(BUILDPROJARGS)
	@$(MAKE) all

#### BUILD END ####
# Make "all" the default target.
all:

#### BUILD BEGIN ####
# The "realclean" option should restore the directory to a pristine state, just
# as it would be saved in the repository.
.PHONY: realclean
realclean:
	rm -rf $(SUBDIRS)
#### BUILD END ####

# All makefiles use this to find the top level directory.
# Set SWROOT to help us find other Makefiles
#### BUILD BEGIN ####
ifeq ($(findstring CYGWIN, $(shell uname)), CYGWIN)
    SWROOT?=$(cygpath -m $(shell git rev-parse --show-toplevel))
endif
#### BUILD END ####
SWROOT?=$(shell git rev-parse --show-toplevel)
include $(SWROOT)/makedefs/subdirs.mk
