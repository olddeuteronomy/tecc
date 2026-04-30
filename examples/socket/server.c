// Time-stamp: <Last changed 2026-04-30 12:09:43 by magnolia>

#include <signal.h>

#include "tecc/tecc_service.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_thread.h"
#include "tecc/tecc_trace.h"
#include "tecc/tecc_tcp_server.h"


static TecSignal sig_started = {0};
static TecSignal sig_stopped = {0};
static int error = 0;

// Start the service in the separate thread.
int service_thread(void* args) {
    TECC_TRACE_ENTER("service_thread");
    TecServicePtr svc = TecService_ptr(args);
    svc->start(svc, &sig_started, &error);
    TECC_TRACE_EXIT();
    return error;
}

static TecSignal sig_quit;

// Handle Ctrl-C.
void handle_sigint(int sig) {
    (void)sig;
    TecSignal_set(&sig_quit);
}


// Usage: server [ADDR] [PORT]
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

    TecTCPServer srv;
    TecTCPServer_init(&srv, &server_params, &socket_params);

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
