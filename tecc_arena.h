// Time-stamp: <Last changed 2026-05-07 10:23:41 by magnolia>
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
/*======================================================================
*
*    TecArena - a fixed‑capacity arena that allocates objects of
*    fixed size from a preallocated block and automatically switches
*    to heap allocation when the arena is full.
*
 *====================================================================*/

#ifndef TECC_ARENA_H
#define TECC_ARENA_H

#include <stddef.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_signal.h"

#ifdef __cplusplus
extern "C" {
#endif


#define TECC_ARENA_ALLOCATED 0x01

// Any arena-ready object must include this header as its first member.
typedef struct tagTecArenaElem {
    unsigned flags;    // Allocation flags. 0 - allocated from heap.
} TecArenaElem;
typedef TecArenaElem* TecArenaElemPtr;

// Forward references.
typedef struct tagTecArenaFreeNode TecArenaFreeNode;
typedef TecArenaFreeNode* TecArenaFreeNodePtr;

// 88 bytes.
typedef struct tagTecArena {
    char* buf;                // Preallocated arena buffer.
    size_t elem_size;         // Size of object to be allocated.
    size_t capacity;          // Arena capacity.
    atomic_size_t allocated;  // Count of allocated object at arena.
    TecArenaFreeNodePtr free_list;
    TecMutex lock;
} TecArena;
typedef TecArena* TecArenaPtr;

TECC_API bool TecArena_init(TecArenaPtr arena, size_t nelems, size_t elem_size);
TECC_API void TecArena_done(TecArenaPtr arena);

TECC_API void* TecArena_allocate(TecArenaPtr arena);
TECC_API void TecArena_release(TecArenaPtr arena, TecArenaElemPtr elem);


#ifdef __cplusplus
}
#endif

#endif // TECC_ARENA_H
