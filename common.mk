# Time-stamp: <Last changed 2026-09-03 14:54:51 by magnolia>

PROJECT_NAME := tecc
PROJECT_ROOT := $(HOME)
LIBNAME := lib$(PROJECT_NAME)

########################################################################
#                Platform depending settings
########################################################################

# Get lower-case kernel name
detected_OS := $(shell uname -s | tr A-Z a-z)
ifeq ($(detected_OS),smolbsd)
	detected_OS = "netbsd"
endif

MKDIR_P := mkdir -p
LIBSUFFIX := .a

########################################################################
#                     Compiler flags
########################################################################

ifndef CC_STD
CC_STD := c17
endif

DEPS := -MMD -MP
CFLAGS := -std=$(CC_STD) -Wall -Wextra -Werror $(DEPS)

ifdef TRACE_ON
CFLAGS += -DTECC_TRACE_ON=1
endif

ifdef NO_PTHREAD
CFLAGS += -DTECC_NO_PTHREAD=1
endif


########################################################################
#        Target paths depending on build configuration
########################################################################
TARGET := $(detected_OS)

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
else
# `gcc' by default
CC := gcc
CFLAGS += -fdiagnostics-color=always
endif

LIBDIR := $(PROJECT_ROOT)/lib/$(PROJECT_NAME)/$(TARGET)
LIBPATH := $(LIBDIR)/$(LIBNAME)$(LIBSUFFIX)
BINDIR := $(PROJECT_ROOT)/bin/$(PROJECT_NAME)/$(TARGET)
