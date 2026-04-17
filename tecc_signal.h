// Time-stamp: <Last changed 2026-04-17 13:50:08 by magnolia>
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
#ifndef TECC_SIGNAL_H
#define TECC_SIGNAL_H

#include <stdatomic.h>
#include <threads.h>
#include <stdbool.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_time.h"

#ifdef __cplusplus
extern "C" {
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                             Mutex
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct tagTecMutex TecMutex;
typedef TecMutex* TecMutexPtr;

typedef struct tagTecMutex {
    bool ok;
    mtx_t m;
} TecMutex;
typedef TecMutex* TecMutexPtr;

TECC_API bool TecMutex_init(TecMutexPtr _mtx);
#define TecMutex_destroy(_mtx) if ((_mtx)->ok) mtx_destroy(&(_mtx)->m)
#define TecMutex_ok(_mtx) ((_mtx)->ok)
#define TecMutex_lock(_mtx) mtx_lock(&(_mtx)->m)
#define TecMutex_unlock(_mtx) mtx_unlock(&(_mtx)->m)

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                     Conditional variable
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct tagTecCV TecCV;
typedef TecCV* TecCVPtr;

typedef struct tagTecCV {
    bool ok;
    cnd_t c;
} TecCV;
typedef TecCV* TecCVPtr;

bool TECC_API TecCV_init(TecCVPtr _cv);
#define TecCV_destroy(_cv) if ((_cv)->ok) cnd_destroy(&(_cv)->c)
#define TecCV_ok(_cv) ((_cv)->ok)
#define TecCV_broadcast(_cv) cnd_broadcast(&(_cv)->c)
#define TecCV_signal(_cv) cnd_signal(&(_cv)->c)
#define TecCV_wait(_cv, _mtx) cnd_wait(&(_cv)->c, &(_mtx)->m)
#define TecCV_timedwait(_cv, _mtx, _ts) cnd_timedwait(&(_cv)->c, &(_mtx)->m, _ts)

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                      The Signal object
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct tagTecSignal TecSignal;
typedef TecSignal* TecSignalPtr;

typedef struct tagTecSignal {
    atomic_int value;
    TecMutex mtx;
    TecCV cnd;
} TecSignal;
typedef TecSignal* TecSignalPtr;

// Initializes the signal. Returns `true` on success.
TECC_API bool TecSignal_init(TecSignalPtr sig);

// Destructor. Behavior undefined if threads are still waiting.
TECC_API void TecSignal_done(TecSignalPtr sig);

// Sets the atomic value to 1 and notifies all waiters.
TECC_API void TecSignal_set(TecSignalPtr sig);

// Wait until Signal is set to 1. Blocks indefinitely.
TECC_API void TecSignal_wait(TecSignalPtr sig);

// Wait until the signal is set to 1 or the timeout (in nanoseconds) has elapsed.
// Returns `false` on timeout or error.
TECC_API bool TecSignal_wait_for(TecSignalPtr sig, TecTimePoint timeout);


#ifdef __cplusplus
}
#endif

#endif // TECC_SIGNAL_H
