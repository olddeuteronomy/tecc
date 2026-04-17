// Time-stamp: <Last changed 2026-04-17 13:50:08 by magnolia>
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
#ifndef TECC_QUEUE_H
#define TECC_QUEUE_H

#include <stdbool.h>
#include <threads.h>

#include "tecc/tecc_def.h" // IWYU pragma: keep
// #include "tecc/tecc_message.h"
#include "tecc/tecc_signal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                      Thread-safe queue
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct tagTecQueueNode TecQueueNode;
typedef TecQueueNode* TecQueueNodePtr;

typedef struct tagTecQueue {
    TecQueueNodePtr head;
    TecQueueNodePtr tail;
    size_t size;
    TecMutex mtx;
    TecCV cv;
} TecQueue;
typedef TecQueue* TecQueuePtr;


// Initialize a queue.
TECC_API bool TecQueue_init(TecQueuePtr q);

// Destructor.
TECC_API void TecQueue_done(TecQueuePtr q);

// Enqueue an object.
TECC_API void TecQueue_push(TecQueuePtr q, void* obj);

// Dequeue an object.
TECC_API void* TecQueue_pop(TecQueuePtr q);


#ifdef __cplusplus
}
#endif

#endif // TECC_QUEUE_H
