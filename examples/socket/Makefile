# Time-stamp: <Last changed 2026-05-13 12:04:40 by magnolia>

include ../../common.mk
include ../examples.mk


all: mt_server server client socket ssocket

client: client.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

server: server.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

mt_server: mt_server.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

socket: socket.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@

ssocket: ssocket.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBS) -o $(OUTDIR)/$@


$(OUTDIR):
	$(MKDIR_P) $(OUTDIR)

rebuild:
	$(MAKE) -C $(TECCDIR)
	$(MAKE) -C .
