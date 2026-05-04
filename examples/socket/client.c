// Time-stamp: <Last changed 2026-05-05 02:58:32 by magnolia>

#include <stdlib.h>

#include "tecc/tecc_trace.h" // IWYU pragma: keep
#include "tecc/tecc_tcp_client.h"


static char test_str[] =
    "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.  "
    "Donec hendrerit tempor tellus.  Donec pretium posuere tellus.  "
    "Proin quam nisl, tincidunt et, mattis eget, convallis nec, purus.  "
    "Cum sociis natoque penatibus et magnis dis parturient montes, "
    "nascetur ridiculus mus.  Nulla posuere.  Donec vitae dolor.  "
    "Nullam tristique diam non turpis.  Cras placerat accumsan nulla.  "
    "Nullam rutrum.  Nam vestibulum accumsan nisl.\n";


int test(TecTCPClientPtr cli) {
    int error = cli->send_str(cli, test_str);
    return error;
}


// Usage: client [ADDR] [PORT]
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

    TecSignal sig_started;
    TecSignal_init(&sig_started);
    TecSignal sig_stopped;
    TecSignal_init(&sig_stopped);

    TecSocketParams socket_params;
    TecSocketParams_init(&socket_params);
    parse_args(argc, argv, &socket_params);

    TecTCPClient cli;
    TecTCPClient_init(&cli, &socket_params);

    int error = 0;
    TecService_start(&cli, &sig_started, &error);
    TecSignal_wait(&sig_started);

    if (!error) {
        error = test(&cli);
        TecService_shutdown(&cli, &sig_stopped);
        TecSignal_wait(&sig_stopped);
    }

    // Cleanup.
    TecSignal_done(&sig_stopped);
    TecSignal_done(&sig_started);
    TecSocketParams_done(&socket_params);
    TecTCPClient_done(&cli);
    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return error;
}
