PLATFORM?=sw_linux_sdl
PROJECT?=libNemaGUI
PREFIX?=/data/TSi_usr

NEMAGFX_SDK_PATH?=../../../NemaGFX_SDK
NEMA_GUI_PATH=.


#source files list
include $(NEMA_GUI_PATH)/nemagui.mk


#Include paths
INCPATHS += $(NEMA_GUI_PATH)/include
INCPATHS += $(NEMA_GUI_PATH)/include/gitems

#NemaGFX include paths
#--------------------------------------------------------------------
INCLUDE_NEMAGFX=$(NEMAGFX_SDK_PATH)/include/tsi/NemaGFX
INCLUDE_NEMADC =$(NEMAGFX_SDK_PATH)/include/tsi/NemaDC
INCLUDE_COMMON =$(NEMAGFX_SDK_PATH)/include/tsi/common
BAREMETAL_PLATFORM=$(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)


COMPILER?=gcc
AR?=ar
CC=$(COMPILER)

.PHONY: Release
Release: CFLAGS += -O3
Release: LDFLAGS += -O3 -s
Release: all

.PHONY: Small
Small: CFLAGS += -Os
Small: LDFLAGS += -Os -s
Small: all

.PHONY: Profile
Profile: CFLAGS += -O3 -pg
Profile: LDFLAGS += -O3 -pg
Profile: all

.PHONY: Debug
Debug: CFLAGS += -g -O0
Debug: LDFLAGS += -g -O0
Debug: all

CFLAGS+=-fPIC -fno-strict-aliasing -MMD -std=c99 -Wall -fmerge-all-constants -Wsign-compare -D_POSIX_C_SOURCE=199309L

CFLAGS+=-DWALL_TIME_CLOCKS #Watchfaces and Digital clocks will be updated using the systems wall time

LDFLAGS+=-lpthread
LDFLAGS_SHARED=-shared $(LDFLAGS)

ODIR=build
LIBDIR=lib

# --- Include Directories ---
LIBRARY=
INCPATHS+=$(INCLUDE_NEMAGFX)
INCPATHS+=$(INCLUDE_NEMADC)
INCPATHS+=$(INCLUDE_COMMON)
INCPATHS+=$(BAREMETAL_PLATFORM)/common
INCPATHS+=$(PWD)

# LIBPATHS=$(NEMAGFX_SDK_PATH)/lib

# ------------ MAGIC BEGINS HERE -------------

# Automatic generation of some important lists
_C_OBJS   =$(C_SRCS:.c=.o)
C_OBJS = $(patsubst %,$(ODIR)/%,$(_C_OBJS))

INCFLAGS+=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS+=$(foreach TMP,$(LIBPATHS),-L$(TMP))

BINARY_SHARED=$(LIBDIR)/$(PROJECT).so
BINARY_STATIC=$(LIBDIR)/$(PROJECT).a
BINARY_ALL=$(BINARY_SHARED) $(BINARY_STATIC)

all: $(LIBDIR) $(C_SRCS) $(C_OBJS) $(BINARY_ALL)

#Compile
#-------------
$(ODIR)/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@

$(LIBDIR):
	mkdir -p $(LIBDIR)

#Link
#-------------
$(BINARY_STATIC):  $(C_OBJS)
	$(AR) rcs $(LDFLAGS_STATIC)  $(BINARY_STATIC) $(C_OBJS)

$(BINARY_SHARED):  $(C_OBJS)
	$(CC) $(LDFLAGS_SHARED) $(LIBFLAGS) $(C_OBJS)   -o $@ $(LDFLAGS)

-include $(OBJECTS:.o=.d)

install:
	test    -d $(PREFIX)
	install -d $(PREFIX)/lib
	install -d $(PREFIX)/include
	install -d $(PREFIX)/include/NemaGUI
	install -d $(PREFIX)/include/NemaGUI/gitems
	install -Dm 755 $(BINARY_STATIC) $(BINARY_SHARED) $(PREFIX)/lib
	find include -maxdepth 1 -type f  -exec install -Dm 664 "{}" "$(PREFIX)/include/NemaGUI/"  \;
	find include/gitems -type f -exec install -Dm 664 "{}" "$(PREFIX)/include/NemaGUI/gitems/"  \;

#Clean
#-------------
distclean: clean
		rm -f $(BINARY_STATIC) $(BINARY_SHARED)

clean:
		rm -f $(C_OBJS) $(C_OBJS:.o=.d) $(BINARY_STATIC) $(BINARY_SHARED)
