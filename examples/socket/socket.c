// Time-stamp: <Last changed 2026-04-29 10:51:56 by magnolia>
/*======================================================================
*
*   A minimum TCP client using TecSocket API.
*
*   Sends a string to the appropriate TCP server
*   (e.g. `ncat -lk[6] localhost 4321` can be used as a server).
*
 *====================================================================*/
#include <stdlib.h>

#include "tecc/tecc_socket.h"
#include "tecc/tecc_trace.h"


static char data[] =
    "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.  "
    "Donec hendrerit tempor tellus.  Donec pretium posuere tellus.  "
    "Proin quam nisl, tincidunt et, mattis eget, convallis nec, purus.  "
    "Cum sociis natoque penatibus et magnis dis parturient montes, "
    "nascetur ridiculus mus.  Nulla posuere.  Donec vitae dolor.  "
    "Nullam tristique diam non turpis.  Cras placerat accumsan nulla.  "
    "Nullam rutrum.  Nam vestibulum accumsan nisl.\n";


int run(TecSocketPtr sock) {
    int err = TecSocket_open(sock);
    if (!err) {
        TecSocket_set_options(sock);
        err = TecSocket_connect(sock);
    }
    if (err) {
        TecSocket_close(sock);
        return err;
    }
    // Sending a static string, we don't use socket's internal buffer.
    err = TecSocket_write_str(sock, data);
    return err;
}


// Usage: socket [ADDR] [PORT]
void parse_args(int argc, char* argv[], TecSocketParamsPtr params) {
    if (argc > 1) {
        params->addr = argv[1];
    }
    if (argc> 2) {
        params->port = atoi(argv[2]);
    }
}

int main(int argc, char* argv[]) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");

    // Initialization.
    TecSocketParams params;
    TecSocketParams_init(&params);
    parse_args(argc, argv, &params);
    TecSocket sock;
    TecSocket_init(&sock, &params);

    // Running.
    int err = run(&sock);

    // Clean up.
    TecSocketParams_done(&params);
    TecSocket_done(&sock);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return err;
}
