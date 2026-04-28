// Time-stamp: <Last changed 2026-04-28 14:46:46 by magnolia>
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_memory.h"
#include "tecc/tecc_buffer.h"


// Global buffer memory error handler.
static TecBufferErrorFunc on_buffer_error = NULL;

TECC_IMPL void TecBuffer_set_error_handler(TecBufferErrorFunc handler) {
    on_buffer_error = handler;
}

#define ON_ERROR(b) if (on_buffer_error) on_buffer_error(b)

/*======================================================================
*
*                          Helpers
*
 *====================================================================*/

TECC_unused static void dump(const char* title, TecBufferPtr buf) {
    printf("%s: {data=%p blk_size=%lu cap=%lu size=%lu pos=%ld}\n",
           title,
           buf->data, buf->block_size, buf->capacity, buf->size, buf->pos);
}


static bool realloc_buffer(TecBufferPtr buf, size_t new_capacity) {
    if (new_capacity <= buf->capacity ) {
        return true;
    }
    char* new_data = TECC_CALLOC(1, new_capacity);
    if (new_data == NULL) {
        return false;
    }
    char* old_data = buf->data;
    if (old_data) {
        memcpy(new_data, old_data, buf->size);
        TECC_FREE(old_data);
    }
    buf->capacity = new_capacity;
    buf->data = new_data;
    return true;
}

static void ensure_capacity(TecBufferPtr buf, long pos, size_t len) {
    if (pos + len <= buf->capacity) {
        return;
    }
    size_t new_capacity = (buf->capacity ? buf->capacity : buf->block_size);
    if (buf->capacity) {
        new_capacity += ((pos + len) / buf->block_size) * buf->block_size;
    }
    while (new_capacity < buf->capacity ) {
        new_capacity += buf->block_size;
        // TODO: should we check for `new_capacity' overflows?
    }
    // Try to expand the buffer.
    if (!realloc_buffer(buf, new_capacity)) {
        ON_ERROR(buf);
    }
}

/*======================================================================
*
*                       TecBuffer API
*
 *====================================================================*/

TECC_IMPL TecBuffer TecBuffer_create_(size_t initial_size, size_t block_size) {
    TecBuffer b;
    TecBuffer_init(&b, initial_size, block_size);
    return b;
}

// If `block_size' == 0, uses TECC_BUFFER_BLOCK_SIZE.
TECC_IMPL void TecBuffer_init_(TecBufferPtr buf, size_t initial_size, size_t block_size) {
    buf->data = NULL;
    buf->pos = 0;
    buf->size = 0;
    buf->capacity = 0;
    buf->block_size = (block_size == 0 ? TECC_BUFFER_BLOCK_SIZE : block_size);
    if (initial_size) {
        ensure_capacity(buf, buf->pos, initial_size);
    }
}

// Deallocates `data' if it has been allocated previosly,
// resets all members to 0 except `block_size`.
TECC_IMPL void TecBuffer_done(TecBufferPtr buf) {
    if (buf->data) {
        TECC_FREE(buf->data);
    }
    buf->data = NULL;
    buf->pos = 0;
    buf->size = 0;
    buf->capacity = 0;
}

// Returns number of bytes written.
TECC_IMPL size_t TecBuffer_write(TecBufferPtr buf, const void *src, size_t len) {
    if (len == 0) {
        return 0;
    }
    long pos = buf->pos;
    ensure_capacity(buf, pos, len);
    memcpy(buf->data + pos, src, len);
    pos += len;
    if ((size_t)pos > buf->size) {
        buf->size = pos;
    }
    buf->pos = pos;
    return len;
}

// Returns the number of bytes read successfully, which may be less than `len'.
TECC_IMPL size_t TecBuffer_read(TecBufferPtr buf, void* dst, size_t len) {
    if (len == 0) {
        return 0;
    }
    if (buf->pos + len > buf->size) {
        len = buf->size - buf->pos;
    }
    memcpy(dst, buf->data + buf->pos, len);
    buf->pos += len;
    return len;
}

// Returns TECC_EOB (-1) if out of bound.
TECC_IMPL int TecBuffer_seek(TecBufferPtr buf, long offset, int origin) {
    long new_pos = 0;
    if (origin == kTecSeekSet) {
        new_pos = offset;
    }
    else if (origin == kTecSeekCur) {
        new_pos = buf->pos + offset;
    }
    else {
        // tecSeekEnd
        new_pos = buf->size + offset;
    }
    if (new_pos < 0 || (size_t)new_pos > buf->size) {
        return TECC_EOB;
    }
    buf->pos = new_pos;
    return 0;
}

// Appends the given string to the buffer (treated as null-terminated),
// ensuring the result remains properly null-terminated.
// Returns the pointer to the beginning of the buffer.
TECC_IMPL size_t TecBuffer_puts(TecBufferPtr buf, const char* str) {
    if (str == NULL) {
        return 0;
    }
    int len  = strlen(str);
    if (len == 0) {
        return 0;
    }
    long pos = buf->pos;
    // Include trailing null.
    ensure_capacity(buf, pos, len + 1);
    if (pos > 0 && buf->data[pos-1] == 0) {
        pos -=1;
        if (buf->size) {
            buf->size -= 1;
        }
    }
    buf->pos = pos;
    return TecBuffer_write(buf, str, len + 1);
}

/*======================================================================
*
*                          Debugging
*
 *====================================================================*/

#define ENQUOTE(s) "\"" #s "\""
#define COMMA ", "

static const char json_format[] =
    "{"
    ENQUOTE(data) ": 0x%08lx" COMMA
    ENQUOTE(block_size) ": %lu" COMMA
    ENQUOTE(capacity) ": %lu" COMMA
    ENQUOTE(size) ": %lu" COMMA
    ENQUOTE(pos) ": %ld"
    "}"
    ;

TECC_IMPL TecBuffer TecBuffer_json(TecBufferPtr buf, const char* name, size_t maxlen) {
    (void)maxlen;
    TecBuffer dst = TecBuffer_create(80, 80);
    char content[128];
    if (name) {
        sprintf(content, "\"%s\": ", name);
        TecBuffer_puts(&dst, content);
    }
    sprintf(content, json_format,
            (ptrdiff_t)buf->data,
            buf->block_size,
            buf->capacity,
            buf->size,
            buf->pos
        );
    TecBuffer_puts(&dst, content);
    return dst;
}


static char table[] = "0123456789ABCDEF";

static void to_hex_chars(int* hex, int ch) {
    if (0x20 < ch && ch < 0x7F) {
        hex[0] = ' ';
        hex[1] = ch;
    }
    else {
        hex[0] = table[ch >> 4];
        hex[1] = table[ch & 0x0F];
    }
}

static const size_t hex_buffer_block_size = 256;

// Returns hex representation of the buffer starting at a `start' position.
// if `len' is 0,
TECC_IMPL TecBuffer TecBuffer_as_hex(TecBufferPtr src, long start, size_t len) {
    // Empty destination buffer.
    TecBuffer dst = TecBuffer_create(0, hex_buffer_block_size);
    // Check source parameters.
    if (!TecBuffer_size(src)) {
        // Empty buffer
        return dst;
    }
    if (start < 0) {
        start = 0;
    }
    if (!len) {
        len = TecBuffer_size(src);
    }
    if ((size_t)start > TecBuffer_size(src)) {
        // Empty buffer
        return dst;
    }
    if (start + len > TecBuffer_size(src)) {
        len = TecBuffer_size(src) - start;
    }
    // Calculate initial capacity to hold the entire output string,
    // including trailing \0.
    size_t size = (len << 1) + 1;
    dst.block_size = ((size / hex_buffer_block_size) * hex_buffer_block_size) + hex_buffer_block_size;
    ensure_capacity(&dst, 0, size);
    int hex[2] = {0, 0};
    long pos = 0;
    char* data = src->data + start;;
    for (size_t i = 0; i < len; ++i) {
        to_hex_chars(hex, data[i]);
        dst.data[pos++] = hex[0];
        dst.data[pos++] = hex[1];
    }
    dst.size = pos;
    dst.pos = pos;
    // Trailing \0
    dst.data[pos] = 0;
    return dst;
}

static const size_t bytes_per_line = 32;
static const size_t hex_table_block_size = 256;

#define NEW_LINE             "\n"
#define TABLE_OFFSET         " offset |"
#define TABLE_OFFSET_FOOTER  "========|"

static const char row_prefix_format[] = "|" NEW_LINE "%08lu|";
static const char hex_table_header[] =
TABLE_OFFSET
"00  02  04  06  08  10  12  14  16  18  20  22  24  26  28  30  |"
NEW_LINE
TABLE_OFFSET_FOOTER
"++--++--++--++--++--++--++--++--++--++--++--++--++--++--++--++--"
;

TECC_IMPL TecBuffer TecBuffer_as_table(TecBufferPtr src, long start, size_t len) {
    TecBuffer tmp = TecBuffer_as_hex(src, start, len);
    // Calculate capacity of the destination buffer.
    size_t block_size = ((tmp.size / hex_table_block_size) * hex_table_block_size)
        + hex_table_block_size + 2;
    TecBuffer dst = TecBuffer_create(block_size, block_size);
    // Print the header.
    TecBuffer_puts(&dst, hex_table_header);
    if (start < 0) {
        start = 0;
    }
    // Check hex buffer.
    size_t size = tmp.size;
    size_t start_offset = (start / bytes_per_line) * bytes_per_line;
    if (!size || !tmp.data) {
        // Nothing to print
        char buf[80];
        sprintf(buf, row_prefix_format, start_offset);
        TecBuffer_puts(&dst, buf);
        return dst;
    }
    // Print table.
    char d0[] = "  ";
    const char *s = tmp.data;
    size_t offset = start_offset;
    for (size_t n = 0, nbyte = 0; n < size; ++nbyte) {
        if ((nbyte % bytes_per_line) == 0) {
            // New row.
            char buf[80];
            sprintf(buf, row_prefix_format, offset);
            offset += bytes_per_line;
            TecBuffer_puts(&dst, buf);
        }
        if (nbyte < (size_t)start) {
            TecBuffer_puts(&dst, d0);
        }
        else {
            char d[] = {s[n], s[n+1], 0};
            TecBuffer_puts(&dst, d);
            n += 2;
        }
    }
    TecBuffer_done(&tmp);
    return dst;
}
