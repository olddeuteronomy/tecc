// Time-stamp: <Last changed 2026-05-12 12:13:11 by magnolia>
/*----------------------------------------------------------------------
------------------------------------------------------------------------
Copyright (c) 2026 The Emacs Cat (https://github.com/olddeuteronomy/tecc).

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
#include <stdlib.h>
#include <memory.h>

#include "tecc/tecc_def.h"    // IWYU pragma: keep
#include "tecc/tecc_trace.h"  // IWYU pragma: keep
#include "tecc/tecc_memory.h"
#include "tecc/tecc_threads.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_queue.h"
#include "tecc/tecc_arena.h"
#include "tecc/tecc_thread_pool.h"

/*======================================================================
*
*                        Task node API
*
 *====================================================================*/

// 336 bytes.
typedef struct tagTecTaskNode {
    TecThread thr;
    TecQueue q;
    TecArena arena;
    TecSignal sig_terminated;
} TecTaskNode;
typedef TecTaskNode* TecTaskNodePtr;


static TECC_THREAD_FUNC_RETVAL task_func(void* args) {
    TecTaskNodePtr node = (TecTaskNodePtr)args;
    while (true) {
        TecTaskPtr task = TecQueue_pop(&node->q);
        if (task == NULL) {
            break;
        }
        if (task->task_func) {
            // Call task function.
            TecBuffer buf = {0};
            buf.data = task->buffer;
            buf.size = task->buffer_size;
            buf.capacity = task->buffer_size;
            task->task_func(task->payload, buf, task->args);
        }
        TecArena_release(&node->arena, (TecArenaElemPtr)task);
    };
    TecSignal_set(&node->sig_terminated);
    return 0;
}

static void TecTaskNode_init(TecTaskNodePtr node, size_t arena_nelems) {
    TecThread_init(&node->thr);
    TecQueue_init(&node->q);
    TecArena_init(&node->arena, arena_nelems, sizeof(TecTask));
    TecSignal_init(&node->sig_terminated);
}

static bool TecTaskNode_run(TecTaskNodePtr node) {
    TecThread_create(&node->thr, task_func, node);
    return TecThread_ok(&node->thr);
}

static TecTaskPtr TecTaskNode_allocate_task(TecTaskNodePtr node) {
    return TecArena_allocate(&node->arena);
}

static void TecTaskNode_enqueue_task(TecTaskNodePtr node, TecTaskPtr task) {
    TecQueue_push(&node->q, task);
}

static void TecTaskNode_done(TecTaskNodePtr self) {
    if (TecThread_ok(&self->thr)) {
        TecQueue_push(&self->q, NULL);
        TecSignal_wait(&self->sig_terminated);
        TecThread_join(&self->thr);
    }
    TecQueue_done(&self->q);
    TecArena_done(&self->arena);
    TecSignal_done(&self->sig_terminated);
}

/*======================================================================
*
*                       TecThrPool API
*
 *====================================================================*/

#define get_node(self, n) (&self->nodes[n])

TECC_IMPL void TecThrPool_init(TecThrPoolPtr self,
                               size_t nthreads,
                               size_t buffer_size,
                               size_t payload_size,
                               size_t task_arena_nelems) {
    TECC_TRACE_ENTER("ThrPool::init()");
    self->nthreads = nthreads;
    self->buffer_size = buffer_size;
    self->payload_size = payload_size;
    atomic_init(&self->next_thread_index, 0);
    // Preallocated buffer arena.
    self->buffer_arena = NULL;
    if (buffer_size) {
        self->buffer_arena = TECC_CALLOC(nthreads, buffer_size);
    }
    // Preallocated payload arena.
    self->payload_arena = NULL;
    if (payload_size) {
        self->payload_arena = TECC_CALLOC(nthreads, payload_size);
    }
    // Allocate and initialize thread nodes.
    self->nodes = TECC_CALLOC(nthreads, sizeof(TecTaskNode));
    for (size_t n = 0; n < nthreads; ++n) {
        TecTaskNodePtr node = get_node(self, n);
        TecTaskNode_init(node, task_arena_nelems);
    }
    TECC_TRACE("nthreads=%zu, buffer_size=%zu, payload_size=%zu, task_arena=%zu.\n",
        self->nthreads, self->buffer_size, self->payload_size, task_arena_nelems);
    TECC_TRACE_EXIT();
}


TECC_IMPL bool TecThrPool_run(TecThrPoolPtr self) {
    TECC_TRACE_ENTER("ThrPool::run()");
    bool ok = true;
    for (size_t n = 0; n < self->nthreads; ++n) {
        TecTaskNodePtr node = get_node(self, n);
        ok = TecTaskNode_run(node);
        if (!ok) {
            TECC_TRACE("FATAL: cannot run thread pool!\n");
            break;
        }
    }
    TECC_TRACE_EXIT();
    return ok;
}


TECC_IMPL void TecThrPool_done(TecThrPoolPtr self) {
    TECC_TRACE_ENTER("ThrPool::done()");
    for (size_t n = 0; n < self->nthreads; ++n) {
        TecTaskNodePtr node = get_node(self, n);
        TecTaskNode_done(node);
    }
    TECC_FREE(self->nodes);
    if (self->payload_arena) {
        TECC_FREE(self->payload_arena);
    }
    if (self->buffer_arena) {
        TECC_FREE(self->buffer_arena);
    }
    TECC_TRACE_EXIT();
}


TECC_IMPL void TecThrPool_enqueue(TecThrPoolPtr self,
                                  TecTaskFunc task_func,
                                  void* payload, void* args) {
    TECC_TRACE_ENTER("ThrPool::enqueue()");
    // Gets the next thread index in round-robin fashion.
    size_t ndx = atomic_fetch_add_explicit(
        &self->next_thread_index, 1, memory_order_relaxed)
        % self->nthreads;
    // Allocate a task.
    TecTaskNodePtr node = get_node(self, ndx);
    TecTaskPtr task = TecTaskNode_allocate_task(node);
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
    // Extra arguments and task function.
    task->task_func = task_func;
    task->args = args;
    TECC_TRACE("Task IDX=%zu buf_size=%zu, payload_size=%zu, allocated at %s.\n",
               ndx, task->buffer_size, self->payload_size,
               (task->hdr.flags & TECC_ARENA_ALLOCATED) ? "arena" : "heap"
        );
    // Enqueue the task for execution.
    TecTaskNode_enqueue_task(node, task);
    TECC_TRACE_EXIT();
}
