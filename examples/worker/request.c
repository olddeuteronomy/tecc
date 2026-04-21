// Time-stamp: <Last changed 2026-04-21 02:42:44 by magnolia>

#include <stdio.h>
#include <threads.h>

#include "tecc/tecc_daemon.h"
#include "tecc/tecc_service_worker.h"
#include "tecc/tecc_service.h"
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


// This handler will be called from the worker internal thread.
static int on_gauge_request(TecServicePtr svc, GaugeRequestPtr request, GaugeReplyPtr reply) {
    (void)svc;
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
    int error =TecDaemon_rpc(w, &request, &reply);

    // Analyze the result.
    analyze(&reply, error);
    TECC_TRACE_EXIT();
}

// RUN THE SERVICE WORKER USING THE DAEMON INTERFACE.
static int run(TecDaemonPtr w) {
    TECC_TRACE_ENTER("run()");
    TECC_TRACE("Running the daemon ...\n");
    int error = TecDaemon_run(w);
    if (error) {
        printf("\n*** Inited with code %d\n", error);
        TECC_TRACE_EXIT();
        return error;
    }
    // Waits until the service worker is running.
    TecDaemon_wait_until_running(w);

    // Query gauges.
    query_gauge(12, TecDaemon_ptr(w));

    TECC_TRACE_EXIT();
    return error;
}

int main(void) {
    TECC_TRACE_INIT();
    // Initialize the service.
    TecService svc;
    TecService_init(&svc, 1);
    TecService_register(&svc, GaugeRequest, on_gauge_request);

    TecServiceWorker w;
    // Initialize the service worker with the smallest hash table size possible
    // because we have just one request registered.
    TecServiceWorker_init(&w, &svc, 1);

    // RUN THE SERVICE WORKER USING THE DAEMON INTERFACE.
    int error = run(TecDaemon_ptr(&w));

    // Terminates the worker.
    error = TecDaemon_terminate(&w);
    // Waits until the worker has terminated.
    TecDaemon_wait_until_terminated(&w);

    // Clean up.
    TecDaemon_done(&w);
    TecService_done(&svc);
    printf("\n*** Exited with code %d\n", error);
    TECC_TRACE_DONE();
    return error;
}
