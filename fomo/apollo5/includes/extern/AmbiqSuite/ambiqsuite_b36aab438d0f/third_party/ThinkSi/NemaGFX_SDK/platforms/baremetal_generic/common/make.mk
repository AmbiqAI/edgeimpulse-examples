PROJECT?=$(strip $(notdir $(CURDIR)))

PLATFORM?=baremetal_generic
NEMAGFX_SDK_PATH?=../../

INCLUDE_NEMAGFX=$(NEMAGFX_SDK_PATH)/include/tsi/NemaGFX
INCLUDE_NEMADC =$(NEMAGFX_SDK_PATH)/include/tsi/NemaDC
INCLUDE_COMMON =$(NEMAGFX_SDK_PATH)/include/tsi/common
BAREMETAL_PLATFORM=$(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)

COMPILER?=gcc
CC=$(COMPILER)

.PHONY: Release
Release: CFLAGS += -O3
Release: LDFLAGS += -O3
Release: all

.PHONY: Profile
Profile: CFLAGS += -O3 -pg
Profile: LDFLAGS += -O3 -pg
Profile: all

.PHONY: Coverage
Coverage: CFLAGS += -O0 -g -fprofile-arcs -ftest-coverage
Coverage: LDFLAGS += -O0 -g -fprofile-arcs -ftest-coverage
Coverage: all

.PHONY: Debug
Debug: CFLAGS += -g -O0 -pedantic -Wall  -Wsign-compare -pedantic-errors -Werror=pedantic -Wextra
Debug: LDFLAGS += -g -O0 -pedantic
Debug: all

LIBRARY=
INCPATHS+=$(INCLUDE_NEMAGFX)
INCPATHS+=$(INCLUDE_NEMADC)
INCPATHS+=$(INCLUDE_COMMON)
INCPATHS+=$(BAREMETAL_PLATFORM)/common
INCPATHS+=$(PWD)

LIBPATHS=$(NEMAGFX_SDK_PATH)/lib
LDFLAGS+=-lNemaGFX
#LDFLAGS+=-lNemaDC # link to NemaDC lirary if needed

# ------------ MAGIC BEGINS HERE -------------

# Automatic generation of some important lists
OBJECTS=$(C_SRCS:.c=.o)

INCFLAGS+=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS+=$(foreach TMP,$(LIBPATHS),-L$(TMP))

# Dynamic linking is probably not supported on baremetal systems, thus the BINARY_SHARED needs to be excluded
# BINARY_SHARED=$(PROJECT).dynamic
BINARY_STATIC=$(PROJECT).static

all: $(C_SRCS) $(BINARY_STATIC)

%.o : %.c
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@

# $(BINARY_SHARED):  $(OBJECTS)
# 	$(CC) $(LIBFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(BINARY_STATIC):  $(OBJECTS)
	$(CC) -static $(LIBFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

-include $(OBJECTS:.o=.d)

distclean: clean
		rm -f $(BINARY_STATIC)

clean:
		rm -f $(OBJECTS) $(OBJECTS:.o=.d) $(BINARY_STATIC) $(BINARY_SHARED)
