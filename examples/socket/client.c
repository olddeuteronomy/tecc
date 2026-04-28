// Time-stamp: <Last changed 2026-04-28 13:03:57 by magnolia>

#include "tecc/tecc_client.h"
#include "tecc/tecc_service.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_socket.h"
#include "tecc/tecc_trace.h"


int run(TecServicePtr cli) {
    TecSignal sig_started;
    TecSignal_init(&sig_started);
    TecSignal sig_stopped;
    TecSignal_init(&sig_stopped);

    int err = 0;
    cli->start(cli, &sig_started, &err);
    TecSignal_wait(&sig_started);

    if (!err) {
        cli->shutdown(cli, &sig_stopped);
        TecSignal_wait(&sig_stopped);
    }

    TecSignal_done(&sig_stopped);
    TecSignal_done(&sig_started);
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

    TecSocketParams socket_params;
    TecSocketParams_init(&socket_params);
    parse_args(argc, argv, &socket_params);

    TecClientParams client_params;
    TecClientParams_init(&client_params);

    TecClient cli;
    TecClient_init(&cli, &client_params, &socket_params);

    int err = run(TecService_ptr(&cli));

    TecClientParams_done(&client_params);
    TecSocketParams_done(&socket_params);
    TecClient_done(&cli);
    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return err;
}
