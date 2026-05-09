// Time-stamp: <Last changed 2026-05-09 14:25:24 by magnolia>
/*======================================================================
*
* An example of using RPC‑style request/reply via the daemon interface
*
 *====================================================================*/

#include <stdio.h>

#include "tecc/tecc_worker.h"
#include "tecc/tecc_trace.h"


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


// This RPC handler is being called from the worker internal thread.
static void on_gauge_request(TecRPCPtr rpc, void* args) {
    TECC_TRACE_ENTER("on_gauge_request()");
    (void)args;
    GaugeRequestPtr request = (GaugeRequestPtr)rpc->request;
    GaugeReplyPtr reply = (GaugeReplyPtr)rpc->reply;
    reply->id = request->id;
    reply->units = 'C';
    reply->temperature = 36.7;
    TECC_TRACE_EXIT();
}

// Analyze the result.
static void analyze(GaugeReplyPtr reply, int error) {
    printf("Gauge #%03u: temp=%3.1f%c error=%d\n",
           reply->id, reply->temperature, reply->units, error);
}

// Requests a temperature of a gauge using the Daemon interface.
static void query_gauge(GAUGE_ID id, TecDaemonPtr d) {
    TECC_TRACE_ENTER("query_gauge()");
    // Prepare a request
    GaugeRequest request;
    TecMsg_init(GaugeRequest, &request);
    request.id = id;
    request.units = 'C';

    // Prepare a reply.
    GaugeReply reply;
    TecMsg_init(GaugeReply, &reply);
    reply.id = 0;
    reply.temperature = 0.0;
    reply.units = '?';

    // Query the gauge
    int error = TecDaemon_rpc(d, &request, &reply);

    // Analyze the result.
    analyze(&reply, error);
    TECC_TRACE_EXIT();
}

// RUN THE SERVICE WORKER USING THE DAEMON INTERFACE.
static int run(TecDaemonPtr d) {
    TECC_TRACE_ENTER("run()");
    int error = TecDaemon_run(d);
    if (error) {
        printf("\n*** Inited with code %d\n", error);
        TECC_TRACE_EXIT();
        return error;
    }
    // Waits until the service worker is running.
    TecDaemon_wait_until_running(d);

    // Query gauges.
    query_gauge(12, d);

    TECC_TRACE_EXIT();
    return error;
}

int main(void) {
    TECC_TRACE_INIT();

    // Initialize the worker with the smallest hash table size possible
    // because we have just one request registered.
    TecWorker w;
    TecWorker_init(&w, 1);
    TecWorker_register(&w, GaugeRequest, on_gauge_request);

    // RUN THE SERVICE WORKER USING THE DAEMON INTERFACE.
    int error = run(TecDaemon_ptr(&w));

    // Terminates the worker.
    error = TecDaemon_terminate(&w);
    // Waits until the worker has terminated.
    TecDaemon_wait_until_terminated(&w);

    // Clean up.
    TecDaemon_done(&w);
    printf("\n*** Exited with code %d\n", error);
    TECC_TRACE_DONE();
    return error;
}
