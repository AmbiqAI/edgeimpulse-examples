NEMAGFX_MK=1

PREFIX?=/data/TSi_usr

INCLUDE_PATH=$(NEMAGFX_SDK_PATH)/include/tsi

# --- Include Directories ---
INCFLAGS += -I$(INCLUDE_PATH)/NemaGFX
INCFLAGS += -I$(INCLUDE_PATH)/NemaDC
INCFLAGS += -I$(INCLUDE_PATH)/common

INCFLAGS += -I$(NEMAGFX_SDK_PATH)/NemaGFX
INCFLAGS += -I$(NEMAGFX_SDK_PATH)/common/mem
INCFLAGS += -I$(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)/common

# --- NemaGFX Core API ---
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)/NemaGFX/nema_hal.c           #NemaGFX Abstraction Layer
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/NemaGFX/CL/nema_ringbuffer.c                       #Ringbuffer
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/NemaGFX/CL/nema_cmdlist.c                          #Command List
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/NemaGFX/nema_programHW.c                           #Program GPU's hardware registers
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/NemaGFX/nema_blender.c                             #Program Fragment Processing Core (load blending modes)
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/NemaGFX/nema_graphics.c                            #High level drawing functions (fill, blit, etc)

# --- NemaGFX Additional API ---
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/NemaGFX/nema_font.c                                #Draw fonts
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/NemaGFX/nema_shaderSpecific.c                      #A set of special drawing functions (blur, warp, etc)
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/common/utils/nema_easing.c                              #Easing functions
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/NemaGFX/nema_interpolators.c                       #Program GPU's interpolators
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/common/utils/nema_transitions.c                         #Screen transitions
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/NemaGFX/nema_rasterizer.c                          #

# --- NemaGFX Memory Management ---
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/common/mem/tsi_malloc.c                            #Memory management

#ifndef NEMADC_MK
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/common/mem/nema_devmem.c                           #Map physical memory space to virtual
#endif

# --- Platform Dependent Utilities ---
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/platforms/$(PLATFORM)/common/nema_utils.c          #Utilities functions (get_time, load_file etc.)

# --- NemaGFX Utils ---
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/common/utils/nema_math.c                           #Basic math library (sin, cos, abs etc)
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/common/utils/nema_matrix3x3.c                      #Use 3x3 matrices for 2.5D texture mapping
C_SRCS  +=   $(NEMAGFX_SDK_PATH)/common/utils/nema_matrix4x4.c                      #Use 4x4 matrices for 3D vertex transformations
