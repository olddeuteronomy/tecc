// Time-stamp: <Last changed 2026-05-07 10:32:57 by magnolia>
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
#include <stdlib.h>
#include <string.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_memory.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_trace.h"
#include "tecc/tecc_arena.h"


typedef struct tagTecArenaFreeNode {
    TecArenaFreeNodePtr next;
} TecArenaFreeNode;
typedef TecArenaFreeNode* TecArenaFreeNodePtr;


TECC_IMPL bool TecArena_init(TecArenaPtr arena, size_t nelems, size_t elem_size) {
    arena->elem_size = elem_size;
    arena->capacity = nelems;
    arena->allocated = 0;
    bool ok = TecMutex_init(&arena->lock);
    if (nelems) {
        arena->buf = TECC_CALLOC(nelems, elem_size);
    }
    else {
        arena->buf = NULL;
    }
    arena->free_list = NULL;
    atomic_store(&arena->allocated, 0);
    return ok;
}


TECC_IMPL void* TecArena_allocate(TecArenaPtr arena) {
    TecMutex_lock(&arena->lock);
    TecArenaElemPtr ptr = NULL;
    // Try to allocate from arena.
    if (arena->free_list) {
        // Pop from free list.
        ptr = (TecArenaElemPtr)(arena->free_list);
        arena->free_list = arena->free_list->next;
    }
    else {
        // Allocate from arena.
        size_t used = atomic_load(&arena->allocated);
        if (used < arena->capacity) {
            ptr = (TecArenaElemPtr)(arena->buf + used * arena->elem_size);
        }
    }
    // If no arena memory, use heap.
    if (ptr) {
        atomic_fetch_add(&arena->allocated, 1);
        ptr->flags = TECC_ARENA_ALLOCATED;
    }
    else {
        ptr = TECC_CALLOC(1, arena->elem_size);
    }
    TecMutex_unlock(&arena->lock);
    return ptr;
}


TECC_IMPL void TecArena_release(TecArenaPtr arena, TecArenaElemPtr ptr) {
    if (!ptr) {
        return;
    }
    TecMutex_lock(&arena->lock);
    if (ptr->flags & TECC_ARENA_ALLOCATED) {
        // Push back to free list.
        TecArenaFreeNodePtr node = (TecArenaFreeNodePtr)ptr;
        node->next = arena->free_list;
        arena->free_list = node;
        size_t new_count = atomic_fetch_sub(&arena->allocated, 1) - 1;
        // Reset free list when empty.
        if (new_count == 0) {
            arena->free_list = NULL;
        }
    }
    else {
        // Release from heap.
        TECC_FREE(ptr);
    }
    TecMutex_unlock(&arena->lock);
}


TECC_IMPL void TecArena_done(TecArenaPtr arena) {
    if (arena->buf) {
        TECC_FREE(arena->buf);
        arena->buf = NULL;
    }
    TecMutex_destroy(&arena->lock);
}
