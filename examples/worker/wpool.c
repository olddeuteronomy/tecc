
#include <stdio.h>

#include "tecc/tecc_buffer.h"
#include "tecc/tecc_def.h"    // IWYU pragma: keep
#include "tecc/tecc_trace.h"  // IWYU pragma: keep
#include "tecc/tecc_message.h"
#include "tecc/tecc_worker_pool.h"


static void process(void* payload, TecBuffer buf) {
    (void)buf;
    (void)payload;
    TECC_TRACE_ENTER("process");
    TECC_TRACE_EXIT();
}

void test(TecWorkerPoolPtr wp) {
    TECC_MESSAGE(TecTask, t);
    t->invoke = process;
    TecWorkerPool_dispatch_task(wp, t, NULL);
}

int main(void) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");
    TecWorkerPool wp;
    bool ok = TecWorkerPool_init(&wp, 8, 1024, 0);
    if (ok) {
        TecWorkerPool_run(&wp);
        for (size_t n = 0; n < 16; ++n)
            test(&wp);
    }

    /* getchar(); */
    TecWorkerPool_done(&wp);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return 0;
}
