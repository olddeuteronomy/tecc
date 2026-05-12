# Time-stamp: <Last changed 2026-05-13 02:45:22 by magnolia>

include ../../common.mk
include ../examples.mk

all: timepoint signal queue map buffer

timepoint: timepoint.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

signal: signal.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

queue: queue.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

map: map.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

buffer: buffer.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

$(OUTDIR):
	$(MKDIR_P) $(OUTDIR)

rebuild:
	$(MAKE) -C $(TECCDIR)
	$(MAKE) -C .
