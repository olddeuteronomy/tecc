# Time-stamp: <Last changed 2026-05-13 02:28:44 by magnolia>

include ../../common.mk
include ../examples.mk

all: worker request thrpool | $(OUTDIR)

worker: worker.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

request: request.c  | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

thrpool: thrpool.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

$(OUTDIR):
	$(MKDIR_P) $(OUTDIR)

rebuild:
	$(MAKE) -C $(TECCDIR)
	$(MAKE) -C .
