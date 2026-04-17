// Time-stamp: <Last changed 2026-04-07 16:30:28 by magnolia>

#include <stdio.h>
#include <assert.h>

#include "tecc/tecc_map.h"


static void dump(TecMapPtr map) {
    TecMapIter iter;
    TecMapKeyValuePtr p = TecMapIter_begin(&iter, map);
    printf("size=%lu\n{", map->size);
    while (p) {
        printf("\n  [%lu] key=%s value=%p", iter.pos, p->key, p->value);
        p = TecMapIter_next(&iter);
    }
    printf("\n}\n\n");
}

int main(void) {
    TecMap map;
    TecMap_init(&map, 17);

    puts("TecMap_set: h1");
    TecMap_set(&map, "h1", (void*)0x111111);
    void* p = TecMap_get(&map, "h1");
    assert(p);
    dump(&map);

    puts("TecMap_set: h2");
    TecMap_set(&map, "h2", (void*)0x222222);
    p = TecMap_get(&map, "h2");
    assert(p);
    dump(&map);

    puts("TecMap_set: h3");
    TecMap_set(&map, "h3", (void*)0x333333);
    p = TecMap_get(&map, "h3");
    assert(p);
    dump(&map);

    puts("TecMap_set: h2 - change value");
    TecMap_set(&map, "h2", (void*)0xFFFFFF);
    p = TecMap_get(&map, "h2");
    assert(p);
    dump(&map);

    puts("TecMap_remove: not existing");
    p = TecMap_remove(&map, "not existing");
    assert(!p);
    dump(&map);

    puts("TecMap_remove: h2");
    p = TecMap_remove(&map, "h2");
    assert(p);
    dump(&map);

    puts("TecMap_remove: h1");
    p = TecMap_remove(&map, "h1");
    assert(p);
    dump(&map);

    puts("TecMap_done");
    TecMap_done(&map);
    dump(&map);

    return 0;
}
