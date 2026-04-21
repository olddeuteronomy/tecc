
#include <stdio.h>

#include "tecc/tecc_worker.h"
#include "tecc/tecc_trace.h"

// Declare messages.
TECC_DEF_MESSAGE(MsgPoint)
    int x;
    int y;
TECC_END_MESSAGE(MsgPoint)

TECC_DEF_MESSAGE(MsgStr80)
    char s[80];
TECC_END_MESSAGE(MsgStr80)

// NOTE: Callbacks are working in the daemon's internal thread.
static void on_point(TecMsgPtr msg, void* worker) {
    TECC_TRACE_ENTER("on_point");
    (void)worker;
    MsgPoint* p = (MsgPoint*)msg;
    printf("%s: {x=%d, y=%d}\n", TecMsg_tag(msg), p->x, p->y);
    TECC_TRACE_EXIT();
}

static void on_str80(TecMsgPtr msg, void* worker) {
    TECC_TRACE_ENTER("on_str80");
    (void)worker;
    MsgStr80* s = (MsgStr80*)msg;
    printf("%s: \"%s\"\n", TecMsg_tag(msg), s->s);
    TECC_TRACE_EXIT();
}

// Emulates `on_init`/`on_exit` errors.
TECC_unused static int on_init(TecWorkerPtr w) {
    (void)w;
    return 1000;
}

TECC_unused static int on_exit(TecWorkerPtr w) {
    (void)w;
    return 1234;
}


int main(void) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");
    TecWorker w;
    // Initialized with the smallest possible map capacity to reduce memory footprint.
    TecWorker_init(&w, 3);
    // Register message callbacks.
    TecWorker_register(&w, MsgPoint, on_point);
    TecWorker_register(&w, MsgStr80, on_str80);

    // Uncomment to emulate initialization/exiting errors.
    /* TecWorker_set_on_init(&w, on_init); */
    TecWorker_set_on_exit(&w, on_exit);

    // RUNS THE WORKER USING DAEMON INTERFACE.
    int error = TecDaemon_run(&w);
    if (error) {
        TecDaemon_done(&w);
        printf("\n*** Inited with code %d ***\n", error);
        TECC_TRACE_EXIT();
        TECC_TRACE_DONE();
        return error;
    }
    // Wait until the worker is running.
    TecDaemon_wait_until_running(&w);
    //
    // Send various messages.
    //
    TECC_MESSAGE(MsgPoint, p);
    p->x = 1;
    p->y = 42;
    TecDaemon_send(&w, p);

    TECC_MESSAGE(MsgStr80, str);
    strcpy(str->s, "Hello world!");
    TecDaemon_send(&w, str);
    //
    // Terminate the worker.
    error = TecDaemon_terminate(&w);
    // Wait until the worker has terminated.
    TecDaemon_wait_until_terminated(&w);
    // Clean up.
    TecDaemon_done(&w);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    printf("\n*** Exited with code %d ***\n", error);
    return error;
}
