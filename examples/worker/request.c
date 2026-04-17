// Time-stamp: <Last changed 2026-04-17 12:05:23 by magnolia>

#include <stdio.h>

#include "tecc/tecc_worker.h"

typedef unsigned int GAUGE_ID;

// Declare messages.
TECC_DEF_MESSAGE(GaugeRequest)
    GAUGE_ID id;
    char units; // 'C' or 'F'
TECC_END_MESSAGE(GaugeRequest)

TECC_DEF_MESSAGE(GaugeReply)
    GAUGE_ID id;
    char units; // 'C' or 'F'
    double temperature;
TECC_END_MESSAGE(GaugeReply)

// This handler will be called from the worker internal thread.
static int on_gauge_request(GaugeRequestPtr request, GaugeReplyPtr reply, void* arg) {
    (void)arg;
    reply->id = request->id;
    reply->units = 'C';
    reply->temperature = 36.7;
    return 0;
}

// Analyze the result.
static void analyze(GaugeReplyPtr reply, int error) {
    printf("Gauge #%03u: temp=%3.1f%c error=%d\n",
           reply->id, reply->temperature, reply->units, error);
}

// Requests a temperature of a gauge.
static void query_gauge(GAUGE_ID id, TecDaemonPtr w) {
    // Prepare a request
    GaugeRequest request;
    TecMsg_init(GaugeRequest, &request);
    request.id = id;
    request.units = 'C';

    // Prepare a reply.
    GaugeReply reply;
    TecMsg_init(GaugeReply, &reply);

    // Query the gauge
    int error =TecDaemon_rpc(w, &request, &reply);

    // Analyze the result.
    analyze(&reply, error);
}


int main(void) {
    TecWorker w;
    // Initializes the worker with the smallest hash table size possible
    // because we have just one request registered.
    TecWorker_init(&w, 1);
    // Registers an RPC handler for the GaugeRequest message.
    TecWorker_register_rpc(&w, GaugeRequest, on_gauge_request);

    // RUN THE WORKER USING DAEMON INTERFACE.
    int error = TecDaemon_run(&w);
    if (error) {
        TecDaemon_done(&w);
        printf("\n*** Inited with code %d\n", error);
        return error;
    }
    // Waits until the worker is running.
    TecDaemon_wait_until_running(&w);

    // Query gauges.
    query_gauge(12, TecDaemon_ptr(&w));

    // Terminates the worker.
    error = TecDaemon_terminate(&w);
    // Waits until the worker has terminated.
    TecDaemon_wait_until_terminated(&w);

    // Clean up.
    TecDaemon_done(&w);
    printf("\n*** Exited with code %d\n", error);
    return error;
}
