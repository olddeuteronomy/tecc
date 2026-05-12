# Time-stamp: <Last changed 2026-05-13 02:22:42 by magnolia>

TECCDIR := ../..
BUILDDIR:= $(TECCDIR)/build
LIBTECC := tecc

INCLUDES := -I$(TECCDIR)/..
LIBDIR := $(TECCDIR)/lib$(TARGET)

OUTDIR := $(BUILDDIR)$(TARGET)

LIBS := -L$(LIBDIR) -l$(LIBTECC)

# Source and object files
SRC := $(wildcard *.c)
OBJ := $(patsubst %.c, $(OUTDIR)/%.o, $(SRC))

# Auto‑include dependency files
-include $(OBJ:.o=.d)
