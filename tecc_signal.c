// Time-stamp: <Last changed 2026-04-17 13:50:06 by magnolia>
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

#include "tecc/tecc_def.h"
#include "tecc/tecc_time.h"
#include "tecc/tecc_signal.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                         Helpers
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

TECC_IMPL bool TecMutex_init(TecMutexPtr _mtx) {
    _mtx->ok = (mtx_init(&(_mtx)->m, mtx_plain) == thrd_success);
    return _mtx->ok;
}

TECC_IMPL bool TecCV_init(TecCVPtr _cv) {
    _cv->ok = (cnd_init(&(_cv)->c) == thrd_success);
    return _cv->ok;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                       Signal
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Initialize Signal. Returns true on success, false on failure.
TECC_IMPL bool TecSignal_init(TecSignal* sig)
{
    atomic_init(&sig->value, 0);
    TecMutex_init(&sig->mtx);
    if (!TecMutex_ok(&sig->mtx)) {
        return false;
    }
    TecCV_init(&sig->cnd);
    if (!TecCV_ok(&sig->cnd)) {
        return false;
    }
    return true;
}


// Destructor. Behavior undefined if threads are still waiting.
TECC_IMPL void TecSignal_done(TecSignal* sig)
{
    TecCV_destroy(&sig->cnd);
    TecMutex_destroy(&sig->mtx);
}


// Set the atomic value to 1 and notify all waiters.
TECC_IMPL void TecSignal_set(TecSignal* sig)
{
    atomic_store_explicit(&sig->value, 1, memory_order_release);
    // Wake waiters. Lock to synchronize with waiters that may be about to wait.
    if (TecMutex_lock(&sig->mtx) == thrd_success) {
        TecCV_broadcast(&sig->cnd);
        TecMutex_unlock(&sig->mtx);
    }
    else {
        // If lock fails, still attempt a broadcast without lock as fallback.
        TecCV_broadcast(&sig->cnd);
    }
}


// Wait until Signal is set. Blocks indefinitely.
TECC_IMPL void TecSignal_wait(TecSignal* sig)
{
    // Fast-path check without locking to avoid unnecessary mutex ops.
    if (atomic_load_explicit(&sig->value, memory_order_acquire) == 1) {
        return;
    }
    // Lock and wait on condition variable until predicate satisfied.
    TecMutex_lock(&sig->mtx);
    while (atomic_load_explicit(&sig->value, memory_order_acquire) != 1) {
        TecCV_wait(&sig->cnd, &sig->mtx);
    }
    TecMutex_unlock(&sig->mtx);
}


// Wait until Signal is set or timeout (in nanoseconds) elapses.
// Returns false on timeout or error.
TECC_IMPL bool TecSignal_wait_for(TecSignal * sig, TecTimePoint timeout)
{
    /* Fast-path check */
    if (atomic_load_explicit(&sig->value, memory_order_acquire) == 1) {
        return true;
    }

    /* Compute absolute deadline */
    TecTimePoint deadline = tec_tp_now();
    if (deadline == 0) {
        return false;
    }
    deadline += timeout;
    /* Convert to time spec */
    struct timespec ts;
    tec_tp_to_ts(deadline, &ts);

    if (TecMutex_lock(&sig->mtx) != thrd_success) {
        return false;
    }

    bool ok = false;
    while (atomic_load_explicit(&sig->value, memory_order_acquire) != 1) {
        int cndres = TecCV_timedwait(&sig->cnd, &sig->mtx, &ts);
        if (atomic_load_explicit(&sig->value, memory_order_acquire) == 1) {
            ok = true;
            break;
        }
        if (cndres == thrd_timedout) {
            ok = false;
            break;
        }
    }

    TecMutex_unlock(&sig->mtx);
    return ok;
}
