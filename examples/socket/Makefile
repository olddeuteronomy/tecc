# Time-stamp: <Last changed 2026-04-25 11:35:25 by magnolia>

include ../../common.mk
include ../examples.mk

all: client socket | $(OUTDIR)

client: client.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@

socket: socket.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LIBS) -o $(OUTDIR)/$@


$(OUTDIR):
	$(MKDIR_P) $(OUTDIR)

rebuild:
	$(MAKE) -C $(TECCDIR)
	$(MAKE) -C .
