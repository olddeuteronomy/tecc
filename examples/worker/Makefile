# Time-stamp: <Last changed 2026-04-13 16:39:56 by magnolia>

include ../../common.mk
include ../examples.mk

all: worker request | $(OUTDIR)

worker: worker.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

request: request.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

$(OUTDIR):
	$(MKDIR_P) $(OUTDIR)

rebuild:
	$(MAKE) -C $(TECCDIR)
	$(MAKE) -C .
