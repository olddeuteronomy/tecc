# Time-stamp: <Last changed 2026-04-20 03:08:37 by magnolia>

include common.mk

# Library name
LIBNAME := libtecc

# Directories
LIBDIR := lib

########################################################################
#                   Making the library
########################################################################

LIBDIR := $(LIBDIR)$(TARGET)
LIBPATH := $(LIBDIR)/$(LIBNAME)$(LIBSUFFIX)
LIBINCLUDES := -I..

# Source and object files
LIBSRC := $(wildcard *.c)
LIBOBJ := $(patsubst %.c, $(LIBDIR)/%.o, $(LIBSRC))

# Make the library
all: $(LIBPATH)

# Build the static library
$(LIBPATH): $(LIBOBJ) | $(LIBDIR)
	@echo "Archiving $@"
	ar rcs $@ $(LIBOBJ)

# Compile .c to lib/.o
$(LIBDIR)/%.o: %.c | $(LIBDIR)
	$(CC) $(CFLAGS) $(LIBINCLUDES) -c $< -o $@

# Auto‑include dependency files
-include $(LIBOBJ:.o=.d)

# Create directories if missing
$(LIBDIR):
	mkdir -p $(LIBDIR)

# Cleanup
clean:
	rm -rf $(LIBDIR)

# Convenience target
rebuild: clean all
