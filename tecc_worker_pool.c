// Time-stamp: <Last changed 2026-05-01 15:06:51 by magnolia>
/*----------------------------------------------------------------------
------------------------------------------------------------------------
Copyright (c) 2020-2026 The Emacs Cat (https://github.com/olddeuteronomy/tecc).

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
------------------------------------------------------------------------
----------------------------------------------------------------------*/

#include "tecc/tecc_def.h" // IWYU pragma: keep
#include "tecc/tecc_memory.h"
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_daemon.h"
#include "tecc/tecc_trace.h"
#include "tecc/tecc_worker.h"
#include "tecc/tecc_worker_pool.h"


#define get_worker(self, n) (&self->workers[n])


TECC_IMPL bool TecWorkerPool_init(TecWorkerPoolPtr self, size_t nworkers, size_t bufsize) {
    TECC_TRACE_ENTER("WorkerPool::init()");
    self->buffer = NULL;
    self->nworkers = nworkers;
    self->workers = TECC_MALLOC(nworkers * sizeof(TecWorker));
    bool ok = true;
    for (size_t n = 0; n < nworkers; ++n) {
        ok = ok && TecWorker_init(get_worker(self, n), 1);
    }
    if (ok) {
        self->buffer = TECC_CALLOC(nworkers, bufsize);
    }
    TECC_TRACE_EXIT();
    return ok;
}


TECC_IMPL bool TecWorkerPool_run(TecWorkerPoolPtr self) {
    TECC_TRACE_ENTER("WorkerPool::run()");
    bool ok = true;
    for (size_t n = 0; n < self->nworkers; ++n) {
        bool b = (TecDaemon_run(get_worker(self, n)) == 0);
        if (b) {
            TecDaemon_wait_until_running(get_worker(self, n));
        }
        ok = ok && b;
    }
    TECC_TRACE_EXIT();
    return ok;
}


TECC_IMPL void TecWorkerPool_done(TecWorkerPoolPtr self) {
    TECC_TRACE_ENTER("WorkerPool::done()");
    for (size_t n = 0; n < self->nworkers; ++n) {
        TecWorkerPtr w = get_worker(self, n);
        TecDaemon_terminate(w);
        TecDaemon_wait_until_terminated(w);
        TecWorker_done(w);
    }
    TECC_FREE(self->buffer);
    TECC_FREE(self->workers);
    TECC_TRACE_EXIT();
}
