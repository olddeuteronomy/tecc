// Time-stamp: <Last changed 2026-04-17 13:50:03 by magnolia>
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
#include <stdbool.h>
#include <stdlib.h>

#include "tecc/tecc_def.h" // IWYU pragma: keep
#include "tecc/tecc_memory.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_queue.h"


typedef struct tagTecQueueNode {
    void* obj;
    TecQueueNodePtr next;
} TecQueueNode;


TECC_IMPL bool TecQueue_init(TecQueuePtr q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    TecMutex_init(&q->mtx);
    TecCV_init(&q->cv);
    return true;
}


TECC_IMPL void TecQueue_done(TecQueuePtr q) {
    TecMutex_lock(&q->mtx);
    TecQueueNodePtr n = q->head;
    while (n) {
        TecQueueNodePtr next = n->next;
        TECC_FREE(n);
        n = next;
    }
    TecMutex_unlock(&q->mtx);
    TecMutex_destroy(&q->mtx);
    TecCV_destroy(&q->cv);
}


TECC_IMPL void TecQueue_push(TecQueuePtr q, void* obj) {
    TecMutex_lock(&q->mtx);
    TecQueueNodePtr node = TECC_MALLOC(sizeof(TecQueueNode));
    node->obj = obj;
    node->next = NULL;
    if (q->head == NULL) {
        q->head = node;
        q->tail = node;
    }
    else {
        q->tail->next = node;
        q->tail = node;
    }
    q->size += 1;
    TecCV_signal(&q->cv);
    TecMutex_unlock(&q->mtx);
}


TECC_IMPL void* TecQueue_pop(TecQueuePtr q) {
    TecMutex_lock(&q->mtx);
    while (q->size == 0) {
        TecCV_wait(&q->cv, &q->mtx);
    }
    TecQueueNode* head = q->head;
    void* obj = head->obj;
    q->head = head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    TECC_FREE(head);
    q->size -= 1;
    TecMutex_unlock(&q->mtx);
    return obj;
}
