# Time-stamp: <Last changed 2026-04-23 01:32:21 by magnolia>

include ../../common.mk
include ../examples.mk

all: client | $(OUTDIR)

client: client.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

$(OUTDIR):
	$(MKDIR_P) $(OUTDIR)

rebuild:
	$(MAKE) -C $(TECCDIR)
	$(MAKE) -C .
