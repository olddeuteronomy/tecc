# Time-stamp: <Last changed 2026-09-03 14:57:19 by magnolia>

include common.mk

########################################################################
#                   Making the library
########################################################################

# LIBDIR := $(LIBDIR)/$(TARGET)
# LIBPATH := $(LIBDIR)/$(LIBNAME)$(LIBSUFFIX)
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
	$(MKDIR_P) $(LIBDIR)

# Cleanup
clean:
	rm -rf $(LIBDIR)

# Convenience target
rebuild: clean all
