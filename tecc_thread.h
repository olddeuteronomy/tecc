// Time-stamp: <Last changed 2026-04-18 16:59:12 by magnolia>
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

#ifndef TECC_THREAD_H
#define TECC_THREAD_H

#include <stdbool.h>
#include <threads.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagTecThread TecThread;
typedef TecThread* TecThreadPtr;

typedef struct tagTecThread {
    bool ok;
    int res;
    thrd_t t;
} TecThread;

#define TecThread_create(self, func, arg) do {\
        (self)->res = thrd_create(&(self)->t, (func), (arg));\
        (self)->ok = ((self)->res == thrd_success);\
    } while(0)

#define TecThread_join(self) do {\
        if ((self)->ok) { thrd_join((self)->t, &((self)->res)); (self)->ok = false; } \
    } while(0)

#define TecThread_ok(self) ((self)->ok)

#define TecThread_result(self) ((self)->res)

#ifdef __cplusplus
}
#endif

#endif // TECC_THREAD_H
