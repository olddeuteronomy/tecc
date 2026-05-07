/*======================================================================
*
* A simple round‑robin thread pool. Tasks consist of a function,
* payload, and buffer, all allocated from the pool’s internal arena.
* Worker threads are selected in round‑robin order, providing
* predictable scheduling with minimal synchronization and
* no per‑task heap allocation.
*
 *====================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <memory.h>
#include <threads.h>

#include "tecc/tecc_def.h"    // IWYU pragma: keep
#include "tecc/tecc_memory.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_trace.h"  // IWYU pragma: keep
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_thread.h"
#include "tecc/tecc_queue.h"
#include "tecc/tecc_arena.h"


#ifdef __cplusplus
extern "C" {
#endif

// Forward references.
typedef struct tagTecTaskNode TecTaskNode;
typedef TecTaskNode* TecTaskNodePtr;

typedef struct tagTecThrPool TecThrPool;
typedef TecThrPool* TecThrPoolPtr;

// A function executed by a thread from the pool.
typedef void (*TecTaskFunc)(void* payload, TecBuffer buf, void* args);

// Thread pool = 64 bytes.
typedef struct tagTecThrPool {
    size_t nthreads;              // Number of threads.
    TecTaskNodePtr nodes;         // Internal.
    atomic_int next_thread_index; // Round-robin thread index selection.
    size_t buffer_size;           // One buffer per task, may be 0.
    char* buffer_arena;           // NULL if `buffer_size` is 0.
    size_t payload_size;          // Size of payload per task, may be 0.
    char* payload_arena;          // Preallocated payload arena if any.
    TecTaskFunc task_func;        // A function executed by a thread from the pool.
} TecThrPool;
typedef TecThrPool* TecThrPoolPtr;

/*======================================================================
*
*                   Task for execution in the pool
*
 *====================================================================*/

// A task to be enqueued to the pool for execution.
typedef struct tagTecTask {
    TecArenaElem hdr;        // Can be allocated at the arena.
    char* buffer;            // Task buffer; may be NULL.
    size_t buffer_size;      // Size of per-thread buffer.
    void* payload;           // Preallocated payload or NULL.
    void* args;              // Additional arguments or NULL.
    TecTaskFunc task_func;   // To be executed by a thread from the pool.
} TecTask;
typedef TecTask* TecTaskPtr;


#ifdef __cplusplus
}
#endif

/*======================================================================
*
*                        Task node
*
 *====================================================================*/

// 184 bytes.
typedef struct tagTecTaskNode {
    TecThread thr;
    TecQueue q;
    TecArena arena;
    TecSignal sig_terminated;
} TecTaskNode;
typedef TecTaskNode* TecTaskNodePtr;


static int task_func(void* args) {
    TecTaskNodePtr node = (TecTaskNodePtr)args;
    TecTaskPtr task = NULL;
    do {
        TecTaskPtr task = TecQueue_pop(&node->q);
        if (task) {
            if (task->task_func) {
                // Call task function.
                TecBuffer buf = {0};
                buf.data = task->buffer;
                buf.size = task->buffer_size;
                task->task_func(task->payload, buf, task->args);
            }
            TecArena_release(&node->arena, (TecArenaElemPtr)task);
        }
    } while(task);
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
    TecQueue_push(&self->q, NULL);
    TecSignal_wait(&self->sig_terminated);
    TecThread_join(&self->thr);
    TecQueue_done(&self->q);
    TecArena_done(&self->arena);
    TecSignal_done(&self->sig_terminated);
}

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
    // A function executed by a thread from the pool.
    self->task_func = NULL;
    // Preallocate buffer arena.
    self->buffer_arena = NULL;
    if (buffer_size) {
        self->buffer_arena = TECC_CALLOC(nthreads, buffer_size);
    }
    // Preallocate payload arena.
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
    TECC_TRACE_EXIT();
}


TECC_IMPL bool TecThrPool_run(TecThrPoolPtr self) {
    TECC_TRACE_ENTER("ThrPool::run()");
    bool ok = true;
    for (size_t n = 0; n < self->nthreads; ++n) {
        TecTaskNodePtr node = get_node(self, n);
        ok = TecTaskNode_run(node);
        if (!ok) {
            // FATAL.
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


TECC_IMPL void TecThrPool_enqueue(TecThrPoolPtr self, void* payload, void* args) {
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
    // Arguments
    task->args = args;
    TECC_TRACE("Task IDX=%zu buf_size=%zu, payload_size=%zu, allocated at %s.\n",
               ndx, task->buffer_size, self->payload_size,
               (task->hdr.flags & TECC_ARENA_ALLOCATED) ? "arena" : "heap"
        );
    // Send the task for execution.
    TecTaskNode_enqueue_task(node, task);
    TECC_TRACE_EXIT();
}


#ifdef __cplusplus
}
#endif

/*======================================================================
*
*                            TEST
*
 *====================================================================*/

TECC_unused void test_arena(void) {
    TECC_TRACE_ENTER("test_arena()");
    TecArena arena;
    const size_t N = 16;
    TecArenaElemPtr elems[N];
    TecArena_init(&arena, 8, 8);
    for (size_t i = 0; i < N; ++i) {
        TecArenaElemPtr elem = TecArena_allocate(&arena);
        TECC_TRACE("Allocated at %s.\n",
                   (elem->flags & TECC_ARENA_ALLOCATED) ? "arena" : "heap");
        elems[i] = elem;
    }
    for (size_t i = 0; i < N; ++i) {
        TecArena_release(&arena, elems[i]);
    }
    TecArena_done(&arena);
    TECC_TRACE_EXIT();
}


TECC_unused void test_pool(void) {
    TecThrPool pool;
    TecThrPool_init(&pool, 2, 1024, 0, 2);
    TecThrPool_run(&pool);
    for (size_t i = 0; i < 64; ++i) {
        TecThrPool_enqueue(&pool, NULL, NULL);
    }
    TecThrPool_done(&pool);
}


int main(void) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");

    /* test_arena(); */
    test_pool();

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return 0;
}

