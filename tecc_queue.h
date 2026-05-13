// Time-stamp: <Last changed 2026-05-13 12:21:08 by mac>
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
#ifndef TECC_QUEUE_H
#define TECC_QUEUE_H

/*======================================================================
*
*  A thread‑safe FIFO queue that stores opaque `void*` objects, providing
*  concurrent enqueue (push) and blocking dequeue (pop) operations.
*
 *====================================================================*/

#include <stdbool.h>

#include "tecc/tecc_def.h" // IWYU pragma: keep
#include "tecc/tecc_threads.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagTecQueueNode TecQueueNode;
typedef TecQueueNode* TecQueueNodePtr;

// 120 bytes.
typedef struct tagTecQueue {
    TecQueueNodePtr head;
    TecQueueNodePtr tail;
    TecMutex mtx;
    TecCV cv;
} TecQueue;
typedef TecQueue* TecQueuePtr;

// Initializes the queue.
TECC_API bool TecQueue_init(TecQueuePtr q);

// Destructor.
TECC_API void TecQueue_done(TecQueuePtr q);

// Enqueues an object.
TECC_API void TecQueue_push(TecQueuePtr q, void* obj);

// Dequeues an object; waits until one is available.
TECC_API void* TecQueue_pop(TecQueuePtr q);

#ifdef __cplusplus
}
#endif

#endif // TECC_QUEUE_H
