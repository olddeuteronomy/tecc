# Time-stamp: <Last changed 2026-04-21 01:15:39 by magnolia>

ifndef CC_STD
CC_STD := c17
endif

########################################################################
#                     Compiler flags
########################################################################
DEPS := -MMD -MP
CFLAGS := -std=$(CC_STD) -Wall -Wextra -Werror $(DEPS)

ifdef TRACE_ON
CFLAGS += -DTECC_TRACE_ON=1
endif

########################################################################
#                Platform depending settings
########################################################################
ifeq ($(detected_OS),Windows)
MKDIR_P := MKDIR
LIBSUFFIX := .lib
else
MKDIR_P := mkdir -p
LIBSUFFIX := .a
endif

########################################################################
#        Target paths depending on build configuration
########################################################################
TARGET :=

ifdef REL
CFLAGS += -O2
TARGET := $(TARGET)/release
else
# Debug by default
CFLAGS += -O0 -g
TARGET := $(TARGET)/debug
endif

ifdef CLANG
CC := clang
CFLAGS += -fcolor-diagnostics
TARGET := $(TARGET)/clang
else
# `gcc' by default
CC := gcc
CFLAGS += -fdiagnostics-color=always
TARGET := $(TARGET)/gcc
endif
