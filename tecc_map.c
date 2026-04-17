// Time-stamp: <Last changed 2026-04-17 13:50:02 by magnolia>
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_memory.h"
#include "tecc/tecc_map.h"

#define hash_str(s) tec_fnv_1a_64(s)

typedef struct tagTecMapNode {
    TecMapKeyValue pair;
    TecMapNode* prev;
    TecMapNode* next;
} TecMapNode;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                          Helpers
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Hash function: FNV-1a
TECC_IMPL uint64_t tec_fnv_1a_64(const char* s) {
    uint64_t h = 14695981039346656037ULL;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 1099511628211ULL;
    }
    return h;
}


static TecMapNode* new_node(const char* key, void* value, TecMapNode* prev, TecMapNode* next) {
    TecMapNode* n = TECC_MALLOC(sizeof(TecMapNode));
    n->pair.key = key;
    n->pair.value = value;
    n->prev = prev;
    n->next = next;
    return n;
}

// Returns the next node in the bucket.
static TecMapNode* delete_node(TecMapNode* n) {
    TecMapNode* next = n->next;
    TECC_FREE(n);
    return next;
}

static void delete_bucket(TecMapNode* n) {
    while (n) {
        n = delete_node(n);
    }
}

static TecMapNode* find_node(TecMapPtr map, const char* key, size_t ndx) {
    TecMapNode* node = map->nodes[ndx];
    if (!node) {
        return NULL;
    }
    else if (strcmp(node->pair.key, key) == 0) {
        return node;
    }
    else {
        // Try bucket.
        TecMapNode* b = node->next;
        while (b) {
            if (strcmp(b->pair.key, key) == 0) {
                return b;
            }
            b = b->next;
        }
    }
    return NULL;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                         Hash table
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

TECC_IMPL bool TecMap_init(TecMapPtr map, size_t size) {
    if (size == 0) {
        size = TECC_DEFAULT_MAP_SIZE;
    }
    map->nodes = TECC_CALLOC(size, sizeof(TecMapNode*));
    if (map->nodes == NULL) {
        return false;
    }
    map->size = 0;
    map->capacity = size;
    return true;
}

TECC_IMPL void TecMap_clear(TecMapPtr map) {
    for (size_t i = 0; i < map->capacity; ++i) {
        TecMapNode* node = map->nodes[i];
        if (node) {
            delete_bucket(node);
            map->nodes[i] = NULL;
        }
    }
    map->size = 0;
}

TECC_IMPL void TecMap_done(TecMapPtr map) {
    TecMap_clear(map);
    TECC_FREE(map->nodes);
    map->nodes = NULL;
    map->capacity = 0;
}


TECC_IMPL void* TecMap_set(TecMapPtr map, const char* key, void* value) {
    size_t ndx = hash_str(key) % map->capacity;
    TecMapNode *found = find_node(map, key, ndx);
    void* oldvalue = NULL;
    if (found) {
        // Change existing value.
        oldvalue = found->pair.value;
        found->pair.value = value;
    }
    else {
        // Add a new root node.
        TecMapNode* root = map->nodes[ndx];
        TecMapNode* newroot = new_node(key, value, NULL, root);
        if (root) {
            root->prev = newroot;
        }
        map->nodes[ndx] = newroot;
        map->size += 1;
    }
    return oldvalue;
}


TECC_IMPL void* TecMap_get(TecMapPtr map, const char* key) {
    size_t ndx = hash_str(key) % map->capacity;
    TecMapNode* node = find_node(map, key, ndx);
    if (node) {
        return node->pair.value;
    }
    return NULL;
}


// Returns previous value.
TECC_IMPL void* TecMap_remove(TecMapPtr map, const char* key) {
    size_t ndx = hash_str(key) % map->capacity;
    TecMapNode* node = find_node(map, key, ndx);
    void* oldvalue = NULL;
    if (!node) {
        // Not found.
        return NULL;
    }
    else if (!node->prev) {
        // Root node.
        oldvalue = node->pair.value;
        TecMapNode* newroot = node->next;
        if (newroot) {
            newroot->prev = NULL;
        }
        map->nodes[ndx] = newroot;
    }
    else {
        // Bucket node.
        oldvalue = node->pair.value;
        node->prev->next = node->next;
        if (node->next) {
            node->next->prev = node->prev;
        }
    }
    delete_node(node);
    map->size -= 1;
    return oldvalue;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                          Iterator
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

TECC_IMPL TecMapKeyValuePtr TecMapIter_begin(TecMapIterPtr iter, TecMapPtr map) {
    iter->map = map;
    iter->pos = 0;
    iter->cur = NULL;
    for (size_t pos = 0; pos < map->capacity; ++pos) {
        TecMapNode* cur = map->nodes[pos];
        if (cur) {
            iter->pos = pos;
            iter->cur = cur;
            return &cur->pair;
        }
    }
    return NULL;
}

TECC_IMPL TecMapKeyValuePtr TecMapIter_next(TecMapIterPtr iter) {
    if (iter->cur == NULL) {
        return NULL;
    }
    else if (iter->cur->next) {
        // Check current bucket.
        iter->cur = iter->cur->next;
        return &iter->cur->pair;
    }
    else {
        // Find next bucket.
        for (size_t pos = iter->pos + 1; pos < iter->map->capacity; ++pos) {
            TecMapNode* node = iter->map->nodes[pos];
            if (node) {
                iter->cur = node;
                iter->pos = pos;
                return &node->pair;
            }
        }
    }
    return NULL;
}

TECC_IMPL TecMapKeyValuePtr TecMapIter_rewind(TecMapIterPtr iter) {
    if (iter->map == NULL) {
        return NULL;
    }
    else {
        return TecMapIter_begin(iter, iter->map);
    }
}
