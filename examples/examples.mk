# Time-stamp: <Last changed 2026-03-29 11:52:08 by magnolia>

TECCDIR := ../..
BUILDDIR:= $(TECCDIR)/build
LIBTECC := tecc

INCLUDES := -I$(TECCDIR)/..
LIBDIR := $(TECCDIR)/lib$(TARGET)

OUTDIR := $(BUILDDIR)$(TARGET)

ifndef HEADER_ONLY
LIBS := -L$(LIBDIR) -l$(LIBTECC)
endif
