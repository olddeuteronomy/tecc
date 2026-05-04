// Time-stamp: <Last changed 2026-05-05 02:39:05 by magnolia>
/*======================================================================
*
*                      Testing TecSocket server.
*
 *====================================================================*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <signal.h>

#include "tecc/tecc_buffer.h"
#include "tecc/tecc_socket.h"
#include "tecc/tecc_trace.h"


static atomic_bool running = true;

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

    // Using "pure" socket, we have to handle listening socket's buffer manually.
    size_t bufsize = sock->params->buffer_size;
    TecBuffer_init(&sock->buf, bufsize, bufsize);

    // Buffer for incoming data.
    TecBuffer str;
    TecBuffer_init(&str, 80);

    // The simple polling incoming connections.
    while (atomic_load(&running)) {
        TecSocket cli = TecSocket_accept(sock);
        if (TecSocket_is_valid(&cli)) {
            TecBuffer_rewind(&str); // Reuse the incoming buffer.
            TecSocket_read(&cli, &str);
            puts(TecBuffer_data(&str));
            TecSocket_close(&cli);
            TecSocket_done(&cli);
        }
    }

    TecBuffer_done(&str);
    TecBuffer_done(&sock->buf);
    return err;
}


// Usage: ssocket [ADDR] [PORT]
void parse_args(int argc, char* argv[], TecSocketParamsPtr params) {
    if (argc > 1) {
        params->addr = argv[1];
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
    parse_args(argc, argv, &params);
    /* params.opt_reuse_addr = 1; */
    TecSocket sock;
    TecSocket_init_server(&sock, &params);

    int err = run_server(&sock);

    TecSocketParams_done(&params);
    TecSocket_done(&sock);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return err;
}
