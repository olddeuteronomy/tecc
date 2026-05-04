// Time-stamp: <Last changed 2026-05-04 16:03:53 by magnolia>
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

#include <stdatomic.h>
#include <stdio.h>

#include "tecc/tecc_buffer.h"
#include "tecc/tecc_def.h"   // IWYU pragma: keep
#include "tecc/tecc_trace.h" // IWYU pragma: keep
#include "tecc/tecc_memory.h"
#include "tecc/tecc_message.h"
#include "tecc/tecc_daemon.h"
#include "tecc/tecc_worker.h"
#include "tecc/tecc_worker_pool.h"


#define get_worker(self, n) (&self->workers[n])


static void on_task(TecMsgPtr msg, void* args) {
    (void)args;
    TecTaskPtr task = (TecTaskPtr)msg;
    if (task->invoke) {
        TecBuffer buf = {0};
        buf.data = task->buffer;
        buf.size = task->buffer_size;
        task->invoke(task->payload, buf, task->args);
    }
    TecMsg_free(msg);
}


// We override the Worker's dispatcher with this one.
static void dispatch(TecMsgPtr msg, void* args) {
    on_task(msg, args);
}


// If `task_buffer_size` == 0, no arena is in use.
TECC_IMPL bool TecWorkerPool_init(TecWorkerPoolPtr self, size_t num_workers,
                                  size_t buffer_size,
                                  size_t payload_size) {
    TECC_TRACE_ENTER("WorkerPool::init()");
    self->num_workers = num_workers;
    self->buffer_size = buffer_size;
    self->payload_size = payload_size;
    atomic_init(&self->next_worker_index, 0);
    // Preallocate buffer arena.
    self->buffer_arena = NULL;
    if (buffer_size) {
        self->buffer_arena = TECC_CALLOC(num_workers, buffer_size);
    }
    // Preallocate payload arena.
    self->payload_arena = NULL;
    if (payload_size) {
        self->payload_arena = TECC_CALLOC(num_workers, payload_size);
    }
    // Allocate workers.
    self->workers = TECC_CALLOC(num_workers, sizeof(TecWorker));
    bool ok = true;
    for (size_t n = 0; n < num_workers; ++n) {
        TecWorkerPtr w = get_worker(self, n);
        ok = ok && TecWorker_init(w, 1);
        if (ok) {
            // Substitute Worker's dispatcher.
            w->dispatch = dispatch;
        }
    }
    if (ok) {
        TECC_TRACE("Inited with nworkers=%zu, buffer_arena=%zu, payload_arena=%zu.\n",
                   self->num_workers,
                   self->buffer_size * self->num_workers,
                   self->payload_size * self->num_workers
            );
    }
    else {
        TECC_TRACE("!!! Initialization failed.\n");
    }
    TECC_TRACE_EXIT();
    return ok;
}


TECC_IMPL bool TecWorkerPool_run(TecWorkerPoolPtr self) {
    TECC_TRACE_ENTER("WorkerPool::run()");
    bool ok = true;
    for (size_t n = 0; n < self->num_workers; ++n) {
        TecWorkerPtr w = get_worker(self, n);
        bool b = (TecDaemon_run(w) == 0);
        if (b) {
            TecDaemon_wait_until_running(w);
        }
        ok = ok && b;
    }
    TECC_TRACE_EXIT();
    return ok;
}


TECC_IMPL void TecWorkerPool_done(TecWorkerPoolPtr self) {
    TECC_TRACE_ENTER("WorkerPool::done()");
    for (size_t n = 0; n < self->num_workers; ++n) {
        TecWorkerPtr w = get_worker(self, n);
        TecDaemon_terminate(w);
        TecDaemon_wait_until_terminated(w);
        TecWorker_done(w);
    }
    TECC_FREE(self->workers);
    if (self->payload_arena) {
        TECC_FREE(self->payload_arena);
    }
    if (self->buffer_arena) {
        TECC_FREE(self->buffer_arena);
    }
    TECC_TRACE_EXIT();
}


TECC_API void TecWorkerPool_dispatch_task(
    TecWorkerPoolPtr self, TecTaskPtr task, void* payload, void* args) {
    TECC_TRACE_ENTER("WorkerPool::dispatch_task()");
    // Gets the next Worker index in round-robin fashion.
    size_t ndx = atomic_fetch_add_explicit(
        &self->next_worker_index, 1, memory_order_relaxed)
        % self->num_workers;
    // Assign the buffer for the task.
    task->buffer_size = (self->buffer_arena) ? self->buffer_size : 0;
    task->buffer = (self->buffer_arena) ?
        self->buffer_arena + ndx * self->buffer_size : NULL;
    // Copy payload.
    task->payload = (self->payload_arena) ?
        self->payload_arena + ndx * self->payload_size : NULL;
    if (payload && task->payload) {
        memcpy(task->payload, payload, self->payload_size);
    }
    // Arguments
    task->args = args;
    TECC_TRACE("IDX=%zu buf_size=%zu, payload_size=%zu.\n",
               ndx, task->buffer_size, self->payload_size);
    // Send the task for execution.
    TecWorkerPtr worker = get_worker(self, ndx);
    TecDaemon_send(worker, task);
    TECC_TRACE_EXIT();
}
