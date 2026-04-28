// Time-stamp: <Last changed 2026-04-28 03:52:20 by magnolia>
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
#ifndef TECC_BUFFER_H
#define TECC_BUFFER_H

#include <stddef.h>

#include "tecc/tecc_def.h"

// Default buffer block size.
#define TECC_BUFFER_BLOCK_SIZE 80

// End of buffer flag.
#define TECC_EOB (-1)

#ifdef __cplusplus
extern "C" {
#endif

// TecBuffer_seek's `origin` flags.
enum {
    kTecSeekCur,
    kTecSeekSet,
    kTecSeekEnd
};

/*======================================================================
*          An expandable byte buffer, mimicking file operations.
*
*       Provides a growable in-memory buffer with an API similar to
*           stdio FILE streams (read, write, seek, tell, etc.).
*       The internal storage is automatically expanded as needed.
 *====================================================================*/

typedef struct tagTecBuffer TecBuffer;
typedef TecBuffer* TecBufferPtr;

// 40 bytes.
typedef struct tagTecBuffer {
    char* data;
    long pos;
    size_t size;
    size_t capacity;
    size_t block_size;
} TecBuffer;

#define TecBuffer_ptr(ptr) ((TecBufferPtr)(ptr))

// Returns the initialized buffer. See `TecBuffer_init()`.
TECC_API TecBuffer TecBuffer_create_(size_t initial_size, size_t block_size);

// Variadic initializers.
#define TecBuffer_create0() TecBuffer_create_(0, 0)
#define TecBuffer_create1(initial_size) TecBuffer_create_((initial_size), 0)
#define TecBuffer_create2(initial_size, block_size) TecBuffer_create_((initial_size), (block_size))

#define TecBuffer_create(...) TECC_GET_MACRO_2(__VA_ARGS__, TecBuffer_create2, TecBuffer_create1, TecBuffer_create0)(__VA_ARGS__)


// Initialize the buffer inplace.
// `initial_size` can be 0, no memory will be allocated in this case.
// If `block_size` is 0, TECC_BUFFER_BLOCK_SIZE will be used.
TECC_API void TecBuffer_init_(TecBufferPtr buf, size_t initial_size, size_t block_size);

// Variadic initializers.
#define TecBuffer_init1(bufptr) TecBuffer_init_((bufptr), 0, 0)
#define TecBuffer_init2(bufptr, initial_size) TecBuffer_init_((bufptr), (initial_size), 0)
#define TecBuffer_init3(bufptr, initial_size, block_size) TecBuffer_init_((bufptr), (initial_size), (block_size))

#define TecBuffer_init(...) TECC_GET_MACRO_3(__VA_ARGS__, TecBuffer_init3, TecBuffer_init2, TecBuffer_init1)(__VA_ARGS__)

// Deallocates buffer's `data`, keeping `block_size` intact.
// The buffer can then be reused.
TECC_API void TecBuffer_done(TecBufferPtr buf);

// Writes `len` bytes to the buffer, starting at the current `pos`.
// Expands the buffer if required. On success, `pos` is advanced by `len`.
// Returns number of bytes written.
TECC_API size_t TecBuffer_write(TecBufferPtr buf, const void *src, size_t len);

// Read `len` bytes from the buffer, starting at the current `pos`.
// On success, `pos` is advanced by the number of bytes read successfully
// Returns the number of bytes read successfully, which may be less than `len`.
TECC_API size_t TecBuffer_read(TecBufferPtr buf, void* dst, size_t len);

// Returns the current position indicator (mimicking `ftell`).
#define TecBuffer_tell(buf) (TecBuffer_ptr(buf)->pos)

// Resets the current position indicator.
#define TecBuffer_rewind(buf) TecBuffer_ptr(buf)->pos = 0

// Returns the size of the buffer.
#define TecBuffer_size(buf) (TecBuffer_ptr(buf)->size)

// Get a read-only view of the internal buffer.
// Returns a const pointer to buffer data. May be NULL.
#define TecBuffer_data(buf) ((const char*)(TecBuffer_ptr(buf)->data))

// Resets the buffer position indicator. Returns TECC_EOB (-1) if out of bound.
TECC_API int TecBuffer_seek(TecBufferPtr buf, long offset, int origin);

// Appends a null-terminated string to the buffer (treated as null-terminated),
// ensuring the result remains properly null-terminated.
// Returns number of bytes written.
TECC_API size_t TecBuffer_puts(TecBufferPtr buf, const char* str);

/*======================================================================
*
*                         Debugging
*
 *====================================================================*/

// Returns buffer internal parameters as a JSON string.
TECC_API TecBuffer TecBuffer_json(TecBufferPtr buf, const char* name, size_t maxlen);

// Returns hexadecimal representation of `src` starting at a `start` position.
// Uses `len` if `len` != 0, otherwise uses `buf->size` - `start`.
TECC_API TecBuffer TecBuffer_as_hex(TecBufferPtr src, long start, size_t len);

// Returns a formatted hex dump of the `src` buffer as a table.
/*
 offset |00  02  04  06  08  10  12  14  16  18  20  22  24  26  28  30  |
========|++--++--++--++--++--++--++--++--++--++--++--++--++--++--++--++--|
00000000| H e l l o ,20 w o r l d !20 A p p e n d e d20 s t r i n g .20 T|
00000032| h e20 q u i c k20 b r o w n20 f o x20 j u m p s20 o v e r20 t h|
00000064| e20 l a z y20 d o g .00
*/
TECC_API TecBuffer TecBuffer_as_table(TecBufferPtr src, long start, size_t len);

// Buffer out-of-memory handler. Default is NULL.
typedef void (*TecBufferErrorFunc)(TecBufferPtr);
TECC_API void TecBuffer_set_error_handler(TecBufferErrorFunc handler);

#ifdef __cplusplus
}
#endif

#endif // TECC_BUFFER_H
