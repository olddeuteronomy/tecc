// Time-stamp: <Last changed 2026-05-12 15:50:03 by magnolia>
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
#ifndef TECC_THREAD_POOL_H
#define TECC_THREAD_POOL_H

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
#include <stdatomic.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_arena.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward references.
typedef struct tagTecTaskNode TecTaskNode;
typedef TecTaskNode* TecTaskNodePtr;

// A function executed by a thread from the pool.
typedef void (*TecTaskFunc)(void* payload, TecBuffer buf, void* args);

// Thread pool = 56 bytes.
typedef struct tagTecThrPool {
    size_t nthreads;              // Number of threads.
    TecTaskNodePtr nodes;         // Internal.
    atomic_int next_thread_index; // Round-robin thread index selection.
    size_t buffer_size;           // One buffer per task, may be 0.
    char* buffer_arena;           // Preallocated buffer arena; NULL if `buffer_size` is 0.
    size_t payload_size;          // Size of payload per task, may be 0.
    char* payload_arena;          // Preallocated payload arena; NULL if `payload_size` is 0.
} TecThrPool;
typedef TecThrPool* TecThrPoolPtr;

// A task to be enqueued to the pool for execution. 48 bytes.
typedef struct tagTecTask {
    TecArenaElem hdr;        // Indicates that this object may be allocated at the arena.
    char* buffer;            // Task buffer assigned by ThrPool; may be NULL.
    size_t buffer_size;      // Size of per-thread buffer.
    void* payload;           // Preallocated payload assigned by ThrPool; may be NULL.
    void* args;              // Additional arguments or NULL.
    TecTaskFunc task_func;   // To be executed by a thread from the pool.
} TecTask;
typedef TecTask* TecTaskPtr;

/*======================================================================
*
 *                        TecThrPool API
*
 *====================================================================*/

TECC_API void TecThrPool_init(TecThrPoolPtr self,
                              size_t nthreads,
                              size_t buffer_size,
                              size_t payload_size,
                              size_t task_arena_nelems);

TECC_API void TecThrPool_done(TecThrPoolPtr);

//
TECC_API bool TecThrPool_run(TecThrPoolPtr);

// Creates a task and enqueues it to the pool for execution.
TECC_API void TecThrPool_enqueue(TecThrPoolPtr self,
                                 TecTaskFunc task_func,
                                 void* payload, void* args);

#ifdef __cplusplus
}
#endif

#endif // TECC_THREAD_POOL_H
