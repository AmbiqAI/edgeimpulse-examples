PLATFORM?=lattice_mico32_no_OS
SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

NEMAGFX_SDK_PATH?=../../

C_SRCS +=$(SELF_DIR)/platforms/$(PLATFORM)/utils.c
C_SRCS +=$(SELF_DIR)/main.c
C_SRCS +=$(SELF_DIR)/bench_fill_tri.c
C_SRCS +=$(SELF_DIR)/bench_fill_rect.c
C_SRCS +=$(SELF_DIR)/bench_fill_quad.c
C_SRCS +=$(SELF_DIR)/bench_draw_string.c
C_SRCS +=$(SELF_DIR)/bench_draw_line.c
C_SRCS +=$(SELF_DIR)/bench_draw_rect.c
C_SRCS +=$(SELF_DIR)/bench_blit.c
C_SRCS +=$(SELF_DIR)/bench_stretch_blit.c
C_SRCS +=$(SELF_DIR)/bench_stretch_blit_rotate.c
C_SRCS +=$(SELF_DIR)/bench_textured_tri.c
C_SRCS +=$(SELF_DIR)/bench_textured_quad.c

INCPATHS +=$(SELF_DIR)
