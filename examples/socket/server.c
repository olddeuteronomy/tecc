// Time-stamp: <Last changed 2026-04-29 18:29:39 by magnolia>

/* #include <signal.h> */
#include <stdio.h>

#include "tecc/tecc_service.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_thread.h"
#include "tecc/tecc_trace.h"
#include "tecc/tecc_tcp_server.h"


/* static TecServicePtr service = NULL; */
static TecSignalPtr started = NULL;
static TecSignalPtr stopped = NULL;
static int* error = NULL;

int service_thread(void* args) {
    TECC_TRACE_ENTER("service_thread");
    TecServicePtr svc = TecService_ptr(args);
    svc->start(svc, started, error);
    TECC_TRACE_EXIT();
    return *error;
}

/* void handle_sigint(int sig) { */
/*     (void)sig; */
/*     TecThread th; */
/*     TecThread_create(&th, shutdown_thread, service); */
/*     TecThread_join(&th); */
/* } */

/* int run(TecServicePtr srv) { */

/*     int err = 0; */
/*     srv->start(srv, &sig_started, &err); */
/*     TecSignal_wait(&sig_started); */

/*     /\* if (!err) { *\/ */
/*     /\*     srv->shutdown(srv, &sig_stopped); *\/ */
/*     /\*     TecSignal_wait(&sig_stopped); *\/ */
/*     /\* } *\/ */

/*     return err; */
/* } */


// Usage: server [ADDR] [PORT]
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

    // Set Ctrl-C handler that stops polling.
    /* signal(SIGINT, handle_sigint); */

    TecSignal sig_started;
    TecSignal_init(&sig_started);
    started = &sig_started;
    TecSignal sig_stopped;
    TecSignal_init(&sig_stopped);
    stopped = &sig_stopped;

    TecSocketParams socket_params;
    TecSocketParams_init(&socket_params);
    parse_args(argc, argv, &socket_params);

    TecTCPServerParams server_params;
    TecTCPServerParams_init(&server_params);

    TecTCPServer srv;
    TecTCPServer_init(&srv, &server_params, &socket_params);

    int err = 0;
    error = &err;
    // Start the server on separate thread.
    TecThread th;
    TecThread_create(&th, service_thread, &srv);
    TecSignal_wait(&sig_started);
    TECC_TRACE("Server started with err=%d.\n");

    if (!err) {
        // OK.
        getchar();
        TecServicePtr svc = TecService_ptr(&srv);
        svc->shutdown(svc, &sig_stopped);
        TecSignal_wait(&sig_stopped);
    }
    TecThread_join(&th);

    TecTCPServerParams_done(&server_params);
    TecSocketParams_done(&socket_params);
    TecTCPServer_done(&srv);
    TecSignal_done(&sig_stopped);
    TecSignal_done(&sig_started);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return err;
}
