// Time-stamp: <Last changed 2026-04-27 15:49:56 by magnolia>
/*======================================================================
*
*  Tests TecSocket server.
*
 *====================================================================*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <signal.h>

/* #include "tecc/tecc_buffer.h" */
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_def.h"
#include "tecc/tecc_socket.h"
#include "tecc/tecc_trace.h"


TECC_unused static atomic_bool running = true;

void handle_sigint(int sig) {
    (void)sig;
    atomic_store(&running, false);
}

int run_server(TecSocketPtr sock) {
    int err = TecSocket_open(sock);
    if (!err) {
        TecSocket_set_options(sock);
        err = TecSocket_bind(sock);
    }
    if (!err) {
        err = TecSocket_listen(sock);
    }
    if (err) {
        return err;
    }

    // The simple polling incoming connections.
    while (atomic_load(&running)) {
        TecSocket cli = TecSocket_accept(sock);
        if (TecSocket_is_valid(&cli)) {
            TecSocket_read(&cli, &sock->buf, 0);
            puts(TecBuffer_data(&sock->buf));
            TecSocket_close(&cli);
            TecSocket_done(&cli);
        }
}

    // Using "pure" socket, we have to handle socket's buffer manually.
    /* TecBuffer_init(&sock->buf, sock->params->buffer_size); */
    // Send a string.
    /* TecSocket_write_str(sock, "Hello world!\n"); */
    /* TecBuffer_done(&sock->buf); */
    return err;
}


// Usage: ssocket [ADDR] [PORT]
void parse_args(int argc, char* argv[], TecSocketParamsPtr params) {
    if (argc > 1) {
        TecSocketParams_set_addr(params, argv[1]);
    }
    if (argc > 2) {
        params->port = atoi(argv[2]);
    }
}



int main(int argc, char* argv[]) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");

    // Set Ctrl-C handler that stops polling.
    signal(SIGINT, handle_sigint);

    TecSocketParams params;
    TecSocketParams_init(&params);
    TecSocket sock;
    TecSocket_init_server(&sock, &params);
    parse_args(argc, argv, &params);

    int err = run_server(&sock);

    TecSocketParams_done(&params);
    TecSocket_done(&sock);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return err;
}
