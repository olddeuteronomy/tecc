// Time-stamp: <Last changed 2026-05-07 03:02:35 by magnolia>
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
#include <stdlib.h>
#include <string.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_memory.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_trace.h"
#include "tecc/tecc_arena.h"


TECC_IMPL void TecArena_init(TecArenaPtr arena, size_t nelems, size_t elem_size) {
    arena->elem_size = elem_size;
    arena->nelems = nelems;
    arena->allocated = 0;
    TecMutex_init(&arena->guard);
    if (nelems) {
        arena->buf = TECC_CALLOC(nelems, elem_size);
        arena->pos = (ptrdiff_t)arena->buf;
    }
    else {
        arena->buf = NULL;
        arena->pos = 0;
    }
}


static size_t count__ = 0;

TECC_IMPL void* TecArena_allocate(TecArenaPtr arena) {
    TECC_TRACE_ENTER("Arena::allocate()");
    TecMutex_lock(&arena->guard);
    TecArenaElemPtr elem = NULL;
    count__ += 1;
    if (arena->allocated < arena->nelems) {
        // Allocate at the arena.
        TECC_TRACE("At arena: ID=%zu allocated=%zu, nelems=%zu.\n",
                   count__, arena->allocated, arena->nelems);
        if (arena->allocated == 0) {
            // Reset the arena.
            arena->pos = (ptrdiff_t)arena->buf;
        }
        // Use the task arena.
        elem = (TecArenaElemPtr)arena->pos;
        /* memset(elem, 0, arena->elem_size); */
        elem->flags = TECC_ARENA_ALLOCATED;
        arena->pos += arena->elem_size;
        arena->allocated += 1;
    }
    else {
        // Allocate at heap.
        TECC_TRACE("At heap: ID=%zu.\n", count__);
        elem = TECC_CALLOC(1, arena->elem_size);
    }
    elem->id = count__;
    TecMutex_unlock(&arena->guard);
    TECC_TRACE_EXIT();
    return elem;
}

TECC_IMPL void TecArena_release(TecArenaPtr arena, TecArenaElemPtr elem) {
    TECC_TRACE_ENTER("Arena::release()");
    TecMutex_lock(&arena->guard);
    if (elem->flags & TECC_ARENA_ALLOCATED) {
        TECC_TRACE("At arena: ID=%zu, allocated=%zu, nelems=%zu.\n",
                   elem->id,  arena->allocated, arena->nelems);
        if (--arena->allocated == 0) {
            // Reset the arena.
            TECC_TRACE("Reset: allocated=%zu, nelems=%zu.\n", arena->allocated, arena->nelems);
            arena->pos = (ptrdiff_t)arena->buf;
        }
    }
    else {
        TECC_TRACE("At heap: ID=%zu.\n", elem->id);
        TECC_FREE(elem);
    }
    TecMutex_unlock(&arena->guard);
    TECC_TRACE_EXIT();
}

TECC_IMPL void TecArena_done(TecArenaPtr arena) {
    if (arena->buf) {
        TECC_FREE(arena->buf);
        arena->buf = NULL;
    }
    arena->nelems = 0;
    arena->pos = 0;
    TecMutex_destroy(&arena->guard);
}
