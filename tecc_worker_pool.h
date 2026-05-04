// Time-stamp: <Last changed 2026-05-04 16:05:35 by magnolia>
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
#ifndef TECC_WORKER_POOL_H
#define TECC_WORKER_POOL_H

#include <stdatomic.h>
#include <stdbool.h>

#include "tecc/tecc_def.h"    // IWYU pragma: keep
#include "tecc/tecc_buffer.h" // IWYU pragma: keep
#include "tecc/tecc_message.h"
#include "tecc/tecc_worker.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagTecWorkerPool TecWorkerPool;
typedef TecWorkerPool* TecWorkerPoolPtr;

// Worker pool = 56 bytes.
typedef struct tagTecWorkerPool {
    size_t num_workers;
    TecWorkerPtr workers;
    atomic_int next_worker_index; // Round-robin worker index selection.
    size_t buffer_size;           // One buffer per task.
    char* buffer_arena;           // NULL if `task_buffer_size` is 0.
    size_t payload_size;          // Size of payload per task.
    char* payload_arena;          // Preallocated payload arena if any.
} TecWorkerPool;

// Message for invoking a task in the pool's thread.
typedef void (*TecTaskInvokeFunc)(void* payload, TecBuffer buf, void* args);

TECC_DEF_MESSAGE(TecTask)
    char* buffer;       // Task buffer; may be NULL.
    size_t buffer_size;
    void* payload;      // Preallocated payload or NULL.
    void* args;         // Additional arguments or NULL.
    TecTaskInvokeFunc invoke;
TECC_END_MESSAGE(TecTask)


/*======================================================================
*
*                      TecWorkerPool API
*
 *====================================================================*/

// If `task_buffer_size` == 0, no arena is in use.
TECC_API bool TecWorkerPool_init(TecWorkerPoolPtr self, size_t num_workers,
                                 size_t task_buffer_size,
                                 size_t payload_size);

TECC_API bool TecWorkerPool_run(TecWorkerPoolPtr);

TECC_API void TecWorkerPool_dispatch_task(
    TecWorkerPoolPtr self, TecTaskPtr task, void* payload, void* args);

TECC_API void TecWorkerPool_done(TecWorkerPoolPtr);

#ifdef __cplusplus
}
#endif

#endif // TECC_WORKER_POOL_H
