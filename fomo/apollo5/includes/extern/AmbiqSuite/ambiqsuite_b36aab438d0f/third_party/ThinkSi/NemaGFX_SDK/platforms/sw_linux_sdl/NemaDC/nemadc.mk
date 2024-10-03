#CFLAGS+=
LDFLAGS+=-lSDL2

INCLUDE_PATH=$(NEMAGFX_SDK_PATH)/include/tsi

# --- NemaDC API ---
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)/NemaDC/nema_dc_hal.c
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)/NemaDC/nema_dc.c

C_SRCS  +=   $(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)/common/nema_event.c          #

INCFLAGS += -I$(INCLUDE_PATH)/NemaDC
INCFLAGS += -I$(INCLUDE_PATH)/common
INCFLAGS += -I$(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)/common
