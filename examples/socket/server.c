// Time-stamp: <Last changed 2026-05-04 16:00:11 by magnolia>

#include <signal.h>
#include <stdio.h>

#include "tecc/tecc_buffer.h"
#include "tecc/tecc_service.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_socket.h"
#include "tecc/tecc_thread.h"
#include "tecc/tecc_trace.h"
#include "tecc/tecc_tcp_server.h"


static TecSignal sig_started = {0};
static TecSignal sig_stopped = {0};
static int error = 0;

// Start the service in the separate thread.
static int service_thread(void* args) {
    TECC_TRACE_ENTER("service_thread");
    TecServicePtr svc = TecService_ptr(args);
    svc->start(svc, &sig_started, &error);
    TECC_TRACE_EXIT();
    return error;
}

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


// Process client connection.
static void process_str(TecSocketPtr sock) {
    TECC_TRACE_ENTER("process_str");
    TECC_TRACE("Socket: FD=%d, data=%p, size=%zu.\n", sock->fd, sock->buf.data, sock->buf.size);
    TecBuffer data;
    TecBuffer_init(&data, 1024, 1024);
    int err = TecSocket_read(sock, &data, 0);
    if (!err) {
        puts(data.data);
    }
    TecBuffer_done(&data);
    TECC_TRACE_EXIT();
}


int main(int argc, char* argv[]) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");
    TecSignal_init(&sig_quit);

    // Set Ctrl-C handler that stops polling.
    signal(SIGINT, handle_sigint);

    TecSignal_init(&sig_started);
    TecSignal_init(&sig_stopped);

    TecSocketParams socket_params;
    TecSocketParams_init(&socket_params);
    socket_params.addr = kTecAnyAddr; // Accept connection from any IPv4 address.
    parse_args(argc, argv, &socket_params);

    TecTCPServerParams server_params;
    TecTCPServerParams_init(&server_params);
    server_params.worker_pool_size = 8;

    TecTCPServer srv;
    TecTCPServer_init(&srv, &server_params, &socket_params);
    // Process incoming connection.
    srv.process_client = process_str;

    // Start the server in the separate thread.
    TecThread th;
    TecThread_create(&th, service_thread, &srv);
    TecSignal_wait(&sig_started);

    if (!error) {
        // Wait until quit signalled...
        TecSignal_wait(&sig_quit);
        // ... then shutdown the server.
        TecServicePtr svc = TecService_ptr(&srv);
        svc->shutdown(svc, &sig_stopped);
        TecSignal_wait(&sig_stopped);
    }

    // Clean up.
    TecThread_join(&th);
    TecTCPServerParams_done(&server_params);
    TecSocketParams_done(&socket_params);
    TecTCPServer_done(&srv);
    TecSignal_done(&sig_stopped);
    TecSignal_done(&sig_started);
    TecSignal_done(&sig_quit);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return error;
}
