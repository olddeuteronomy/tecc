// Time-stamp: <Last changed 2026-05-13 02:30:13 by magnolia>

#include "tecc/tecc_def.h"    // IWYU pragma: keep
#include "tecc/tecc_trace.h"  // IWYU pragma: keep
#include "tecc/tecc_thread_pool.h"


/*======================================================================
*
*                            TEST
*
 *====================================================================*/

#define NTHREADS 8
#define BUFFER_SIZE 1024
#define PAYLOAD_SIZE 0
#define TASK_ARENA 32

TECC_unused void test_pool(void) {
    TecThrPool pool;
    TecThrPool_init(&pool,
                    NTHREADS,
                    BUFFER_SIZE,
                    PAYLOAD_SIZE,
                    TASK_ARENA);
    TecThrPool_run(&pool);
    for (size_t i = 0; i < 64; ++i) {
        TecThrPool_enqueue(&pool, NULL, NULL, NULL);
    }
    TecThrPool_done(&pool);
}


int main(void) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");

    test_pool();

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return 0;
}
