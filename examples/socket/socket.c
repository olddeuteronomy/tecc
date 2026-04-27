// Time-stamp: <Last changed 2026-04-27 12:00:02 by magnolia>
/*======================================================================
*
*  Tests TecSocket client by sending a string to the appropriate
   TCP server, e.g. `ncat -lk localhost 4321`.
*
 *====================================================================*/
#include <stdlib.h>

#include "tecc/tecc_buffer.h"
#include "tecc/tecc_socket.h"
#include "tecc/tecc_trace.h"


int run(TecSocketPtr sock) {
    int err = TecSocket_open(sock);
    if (!err) {
        err = TecSocket_connect(sock);
    }
    if (err) {
        return err;
    }
    // Using "pure" socket, we have to handle socket's buffer manually.
    /* TecBuffer_init(&sock->buf, sock->params->buffer_size); */
    // Send a string.
    TecSocket_write_str(sock, "Hello world!\n");
    /* TecBuffer_done(&sock->buf); */
    return err;
}


// Usage: socket [ADDR] [PORT]
void parse_args(int argc, char* argv[], TecSocketParamsPtr params) {
    if (argc > 1) {
        TecSocketParams_set_addr(params, argv[1]);
    }
    if (argc> 2) {
        params->port = atoi(argv[2]);
    }
}

int main(int argc, char* argv[]) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");

    TecSocketParams params;
    TecSocketParams_init(&params);
    parse_args(argc, argv, &params);
    TecSocket sock;
    TecSocket_init(&sock, &params);

    int err = run(&sock);

    TecSocketParams_done(&params);
    TecSocket_done(&sock);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return err;
}
