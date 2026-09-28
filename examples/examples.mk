# Time-stamp: <Last changed 2026-09-03 15:07:02 by magnolia>
# `tecc/common.mk' should be included _before_ this file.

TECCDIR := ../..

INCLUDES := -I$(TECCDIR)/..
OUTDIR := $(BINDIR)

LIBS := -L$(LIBDIR) -l$(PROJECT_NAME)

ifndef NO_PTHREAD
	LIBS += -lpthread
endif

# Source and object files.
SRC := $(wildcard *.c)
OBJ := $(patsubst %.c, $(OUTDIR)/%.o, $(SRC))

# # Auto‑include dependency files.
-include $(OBJ:.o=.d)
