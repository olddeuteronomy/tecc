# Time-stamp: <Last changed 2026-04-13 11:06:10 by magnolia>

include ../../common.mk
include ../examples.mk

all: timepoint signal queue map buffer | $(OUTDIR)

timepoint: timepoint.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

signal: signal.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

queue: queue.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

map: map.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

buffer: buffer.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

$(OUTDIR):
	$(MKDIR_P) $(OUTDIR)

rebuild:
	$(MAKE) -C $(TECCDIR)
	$(MAKE) -C .
