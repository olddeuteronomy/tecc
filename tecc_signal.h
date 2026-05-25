// Time-stamp: <Last changed 2026-05-25 12:24:56 by magnolia>
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
#ifndef TECC_SIGNAL_H
#define TECC_SIGNAL_H

/*======================================================================
*
*     A thread‑safe signal object that blocks callers
*     until its internal `atomic_int` becomes 1.
*
 *====================================================================*/

#include <stdbool.h>
#include <stdatomic.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_time.h"
#include "tecc/tecc_threads.h"

#ifdef __cplusplus
extern "C" {
#endif

// 112 bytes.
typedef struct tagTecSignal {
    atomic_int value;
    TecMutex mtx;
    TecCV cnd;
} TecSignal;
typedef TecSignal* TecSignalPtr;

// Initializes the signal.
TECC_API bool TecSignal_init(TecSignalPtr);

// Destructor. Behavior undefined if threads are still waiting.
TECC_API void TecSignal_done(TecSignalPtr);

// Sets the atomic value to 1 and notifies all waiters.
TECC_API int TecSignal_set(TecSignalPtr);

// Waits until Signal is set to 1. Blocks indefinitely.
TECC_API int TecSignal_wait(TecSignalPtr);

// Waits until the signal is set to 1 or the timeout (in nanoseconds) has elapsed.
TECC_API int TecSignal_wait_for(TecSignalPtr, TecTimePoint);


#ifdef __cplusplus
}
#endif

#endif // TECC_SIGNAL_H
