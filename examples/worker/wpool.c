
#include <stdio.h>

#include "tecc/tecc_trace.h"
#include "tecc/tecc_worker_pool.h"

int run(TecWorkerPoolPtr wp) {
    return TecWorkerPool_run(wp);
}

int main(void) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");
    TecWorkerPool wp;
    bool ok = TecWorkerPool_init(&wp, 16, 1024);
    if (ok) {
        run(&wp);
    }

    getchar();
    TecWorkerPool_done(&wp);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return 0;
}
