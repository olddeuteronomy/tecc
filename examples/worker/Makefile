# Time-stamp: <Last changed 2026-05-13 13:19:58 by magnolia>

include ../../common.mk
include ../examples.mk

all: worker request thrpool

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
