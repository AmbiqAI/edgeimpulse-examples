SRC_BASE = fft_radix2
DATA_BASE = 2K 32K 4K data3_big data4_mid data5_small

ifdef PRECISION
KERNEL_DEFINES+=$(CDEFN)$(PRECISION)
KERNEL_OBJ_FOLDER=$(PRECISION)/
endif

ifndef SELECT_PRESET_NAME
SRC_FILES = $(SRC_BASE) $(DATA_BASE)
else

DATA_SINGLE=$(SELECT_PRESET_NAME)
ifdef SELECT_PRESET
BO = $(DIR_BENCH)/fp
SRC_FILES = $(addprefix $(OBJ_HEADER), $(SRC_BASE) $(DATA_SINGLE) )
KERNEL_OBJS_PATH=$(addprefix $(BO)/fft_radix2/$(KERNEL_OBJ_FOLDER), $(SRC_FILES)) $(BO)/preset/$(KERNEL_OBJ_FOLDER)fake_$(SELECT_PRESET_ID)
KERNEL_OBJS=$(addsuffix $(OEXT),$(KERNEL_OBJS_PATH))
KERNEL_DEFINES+=-DSELECT_PRESET=1 
else
SRC_FILES = $(SRC_BASE) $(DATA_BASE)
endif

endif