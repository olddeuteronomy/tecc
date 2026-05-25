// Time-stamp: <Last changed 2026-05-25 14:35:04 by magnolia>
/*======================================================================
*
*                  A single-threaded TCP server.
*
 *====================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "tecc/tecc_def.h"    // IWYU pragma: keep
#include "tecc/tecc_trace.h"  // IWYU pragma: keep
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_tcp_server.h"
#include "tecc/tecc_service_worker.h"


static TecSignal sig_quit;

// Handle Ctrl-C.
static void handle_sigint(int sig) {
    (void)sig;
    TecSignal_set(&sig_quit);
}


// Usage: server [ADDR] [PORT]
static void parse_args(int argc, char* argv[], TecSocketParamsPtr params) {
    if (argc > 1) {
        params->addr = argv[1];
    }
    if (argc > 2) {
        params->port = atoi(argv[2]);
    }
}

// Using the single-threaded server we can reuse the same input buffer.
static TecBuffer data;

// Process client connection.
static int process_str(TecSocketPtr sock, void* arg) {
    (void)arg;
    TECC_TRACE_ENTER("process_str");
    TECC_TRACE("Socket: FD=%d, data=%p, bufsize=%zu.\n", sock->fd, sock->buf.data, sock->buf.size);
    TecBuffer_rewind(&data);
    int err = TecSocket_read(sock, &data);
    if (!err) {
        puts(data.data);
    }
    TECC_TRACE_EXIT();
    return err;
}


int main(int argc, char* argv[]) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");

    // Set Ctrl-C handler that stops polling.
    TecSignal_init(&sig_quit);
    signal(SIGINT, handle_sigint);

    // Allocate incoming data buffer.
    TecBuffer_init(&data, 1024, 1024);

    // Initialize socket parameters.
    TecSocketParams socket_params;
    TecSocketParams_init(&socket_params);
    socket_params.addr = kTecAnyAddr; // Accept connection from any IPv4 address.
    parse_args(argc, argv, &socket_params);

    // Initialize the server.
    TecTCPServer server;
    TecTCPServer_init(&server, &socket_params);
    // Incoming connection processor.
    TecTCPServer_set_client_proc(&server, process_str);

    // Initialize ServiceWorker.
    TecServiceWorker service_worker;
    TecServiceWorker_init(&service_worker, &server, 1);

    // Start up the server via ServiceWorker using Daemon API.
    int error = TecDaemon_run(&service_worker);
    if (!error) {
        // Wait until quit signalled...
        TecSignal_wait(&sig_quit);
        // ... then terminate the service worker.
        TecDaemon_terminate(&service_worker);
        TecDaemon_wait_until_terminated(&service_worker);
    }

    // Clean up.
    TecSocketParams_done(&socket_params);
    TecTCPServer_done(&server);
    TecDaemon_done(&service_worker);
    TecSignal_done(&sig_quit);
    TecBuffer_done(&data);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return error;
}
