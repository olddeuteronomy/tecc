
#include <stdio.h>

#include "tecc/tecc_buffer.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                             TEST
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

TECC_unused static void dump_buffer(TecBufferPtr buf) {
    TecBuffer json = TecBuffer_json(buf, "TecBuffer", 0);
    TecBuffer hex = TecBuffer_as_hex(buf, 0, 0);
    // JSON
    printf("\n\%s\n", TecBuffer_data(&json));
    // Buffer content
    const char* s = TecBuffer_data(buf);
    printf("\"%s\"\n", s ? s : "");
    // Hex
    const char* h = TecBuffer_data(&hex);
    printf("[%s]\n", h ? h : "");
    // Clean up
    TecBuffer_done(&hex);
    TecBuffer_done(&json);
}

TECC_unused static void dump_table(TecBufferPtr buf, long start, size_t len) {
    printf("\n=== start=%ld len=%lu ===\n", start, len);
    TecBuffer tbl = TecBuffer_as_table(buf, start, len);
    puts(TecBuffer_data(&tbl));
    TecBuffer_done(&tbl);
}

static void test(TecBufferPtr buf) {
    dump_buffer(buf);
    dump_table(buf, 0, 0);

    TecBuffer_puts(buf, "Hello, world!");
    dump_buffer(buf);

    TecBuffer_puts(buf, " Appended string.");
    dump_buffer(buf);

    TecBuffer_puts(buf, " The quick brown fox jumps over the lazy dog.");
    dump_buffer(buf);
    dump_table(buf, 0, 0);
    dump_table(buf, 6, 8);
    dump_table(buf, 31, 100);
    dump_table(buf, 90, 100);
}

int main(void) {
    TecBuffer buf;
    TecBuffer_init(&buf); // Creates empty buffer with default `block_size`
    test(&buf);
    TecBuffer_done(&buf);
    return 0;
}
