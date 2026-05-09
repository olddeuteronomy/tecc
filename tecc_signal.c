// Time-stamp: <Last changed 2026-05-09 13:13:08 by magnolia>
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
#include <errno.h>
#include <stdbool.h>
#include <threads.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_threads.h"
#include "tecc/tecc_signal.h"


TECC_IMPL bool TecSignal_init(TecSignalPtr sig) {
    atomic_init(&sig->value, 0);
    TecMutex_init(&sig->mtx);
    if (!TecMutex_ok(&sig->mtx)) {
        return false;
    }
    TecCV_init(&sig->cnd);
    if (!TecCV_ok(&sig->cnd)) {
        TecMutex_destroy(&sig->mtx);
        return false;
    }
    return true;
}


TECC_IMPL void TecSignal_done(TecSignal* sig) {
    TecCV_destroy(&sig->cnd);
    TecMutex_destroy(&sig->mtx);
}


TECC_IMPL int TecSignal_set(TecSignal* sig) {
    atomic_store_explicit(&sig->value, 1, memory_order_release);
    // Wake waiters.
    int err = TecMutex_lock(&sig->mtx);
    if (!err) {
        err = TecCV_broadcast(&sig->cnd);
        TecMutex_unlock(&sig->mtx);
    }
    else {
        // If lock fails, still attempt a broadcast.
        err = TecCV_broadcast(&sig->cnd);
    }
    return err;
}


TECC_IMPL int TecSignal_wait(TecSignal* sig) {
    if (atomic_load_explicit(&sig->value, memory_order_acquire) == 1) {
        // Already signalled.
        return 0;
    }
    // Lock and wait.
    int err = TecMutex_lock(&sig->mtx);
    if (err) {
        return err;
    }
    while (atomic_load_explicit(&sig->value, memory_order_acquire) != 1) {
        err = TecCV_wait(&sig->cnd, &sig->mtx);
        if (err) {
            break;
        }
    }
    TecMutex_unlock(&sig->mtx);
    return err;
}


TECC_IMPL int TecSignal_wait_for(TecSignal * sig, TecTimePoint timeout) {
    if (atomic_load_explicit(&sig->value, memory_order_acquire) == 1) {
        return 0;
    }
    // Computes absolute deadline in nanosec.
    TecTimePoint deadline = tec_tp_now();
    if (deadline == 0) {
        return EINVAL;
    }
    deadline += timeout;
    // Converts to time spec
    struct timespec ts;
    tec_tp_to_ts(deadline, &ts);
    // Locks and waits.
    int err = TecMutex_lock(&sig->mtx);
    if (err) {
        TecMutex_unlock(&sig->mtx);
        return err;
    }
    while (atomic_load_explicit(&sig->value, memory_order_acquire) != 1) {
        int cnd_err = TecCV_timedwait(&sig->cnd, &sig->mtx, &ts);
        /* if (atomic_load_explicit(&sig->value, memory_order_acquire) == 1) { */
        /*     break; */
        /* } */
        if (cnd_err != 0) {
            err = cnd_err;
            break;
        }
    }
    TecMutex_unlock(&sig->mtx);
    return err;
}
