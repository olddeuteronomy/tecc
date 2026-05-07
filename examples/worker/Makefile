# Time-stamp: <Last changed 2026-05-06 16:54:10 by magnolia>

include ../../common.mk
include ../examples.mk

all: worker request wpool thrpool | $(OUTDIR)

worker: worker.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

request: request.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

wpool: wpool.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

thrpool: thrpool.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

$(OUTDIR):
	$(MKDIR_P) $(OUTDIR)

rebuild:
	$(MAKE) -C $(TECCDIR)
	$(MAKE) -C .
