# Time-stamp: <Last changed 2026-05-26 13:00:29 by magnolia>

UNAME_S := $(shell uname -s)

TECCDIR := ../..
BUILDDIR:= $(TECCDIR)/build
LIBTECC := tecc

INCLUDES := -I$(TECCDIR)/..
LIBDIR := $(TECCDIR)/lib$(TARGET)
OUTDIR := $(BUILDDIR)$(TARGET)

LIBS := -L$(LIBDIR) -l$(LIBTECC)

ifndef NO_PTHREAD
ifeq ($(UNAME_S),Linux)
	LIBS += -lpthread
endif
endif

# Source and object files.
SRC := $(wildcard *.c)
OBJ := $(patsubst %.c, $(OUTDIR)/%.o, $(SRC))

# # Auto‑include dependency files.
-include $(OBJ:.o=.d)
