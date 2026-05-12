// Time-stamp: <Last changed 2026-05-12 13:14:48 by magnolia>
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
#ifndef TECC_MAP_H
#define TECC_MAP_H

/*======================================================================
*
*        A lightweight string‑keyed hash map that stores
*        const char* keys and void* values.
*
 *====================================================================*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "tecc/tecc_def.h"

#define TECC_MAP_DEFAULT_SIZE 117

#ifdef __cplusplus
extern "C" {
#endif

// Hash function.
TECC_API uint64_t tec_fnv_1a_64(const char* s);

/*======================================================================
*
*                      Simple hash table
*
 *====================================================================*/

// Forward references.
typedef struct tagTecMapNode TecMapNode;
typedef TecMapNode* TecMapNodePtr ;

typedef struct tagTecMap TecMap;
typedef TecMap* TecMapPtr;

// Hash table, 24 bytes.
typedef struct tagTecMap {
    TecMapNode** nodes;
    size_t size;
    size_t capacity;
} TecMap;
typedef TecMap* TecMapPtr;

// Uses TECC_MAP_DEFAULT_SIZE if `size` is 0.
// (Use a prime number for `size`).
TECC_API bool TecMap_init(TecMapPtr map, size_t size);

// Destructor. Clears the map, frees memory.
TECC_API void TecMap_done(TecMapPtr map);

// Clears the map. Deletes all nodes, zeroes `nodes` array.
TECC_API void TecMap_clear(TecMapPtr map);

// Inserts a value. Returns the old value if present otherwise NULL.
TECC_API void* TecMap_set(TecMapPtr map, const char* key, void* value);

// Finds a value. Returns NULL if not found.
TECC_API void* TecMap_get(TecMapPtr map, const char* key);

// Removes a key. Returns the old value if present otherwise NULL.
TECC_API void* TecMap_remove(TecMapPtr map, const char* key);

/*======================================================================
*
*                          Iterator
*
 *====================================================================*/

typedef struct tagTecMapKeyValue {
    const char* key;
    void* value;
} TecMapKeyValue;
typedef TecMapKeyValue* TecMapKeyValuePtr;

typedef struct tagTecMapIter {
    TecMapPtr map;
    size_t pos;
    TecMapNodePtr cur;
} TecMapIter;
typedef TecMapIter* TecMapIterPtr;

/*
  TecMap map;
  // ...
  TecMapIter iter;
  TecMapKeyValuePtr p = TecMapIter_init(&iter, &map);
  while (p) {
        // Use `p`
        p = TecMapIter_next(&iter);
        // ...
  }
*/
// DO NOT MODIFY RETURNED VALUE!
TECC_API TecMapKeyValuePtr TecMapIter_begin(TecMapIterPtr iter, TecMapPtr map);
TECC_API TecMapKeyValuePtr TecMapIter_next(TecMapIterPtr iter);
TECC_API TecMapKeyValuePtr TecMapIter_rewind(TecMapIterPtr iter);

#ifdef __cplusplus
}
#endif

#endif // TECC_MAP_H
