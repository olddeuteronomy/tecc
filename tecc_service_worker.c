// Time-stamp: <Last changed 2026-05-12 12:13:11 by magnolia>
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

#include "tecc/tecc_def.h"
#include "tecc/tecc_daemon.h"
#include "tecc/tecc_service.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_threads.h"
#include "tecc/tecc_worker.h"
#include "tecc/tecc_service_worker.h"

/*======================================================================
*
*                    TecWorker overrides
*
 *====================================================================*/

static TECC_THREAD_FUNC_RETVAL service_start_func(void* arg) {
    TecServiceWorkerPtr w = TecServiceWorker_ptr(arg);
    TecServicePtr service = w->service;
    service->start(service, &w->sig_started, &w->error);
    TECC_THREAD_FUNC_RETURN(w->error);
}

static TECC_THREAD_FUNC_RETVAL service_shutdown_func(void* arg) {
    TecServiceWorkerPtr w = TecServiceWorker_ptr(arg);
    TecServicePtr service = w->service;
    service->shutdown(service, &w->sig_stopped);
    return 0;
}

// Creates and runs the service thread which calls `service->start()`.
static int on_init(void* arg) {
    TecServiceWorkerPtr self = TecServiceWorker_ptr(arg);
    // Create a thread that starts the service.
    TecThread_create(&self->service_thread, service_start_func, self);
    if (!TecThread_ok(&self->service_thread)) {
        return TecThread_result(&self->service_thread);
    }
    // Wait until the Service has started.
    TecSignal_wait(&self->sig_started);
    if (self->error) {
        // Stop service on error.
        TecThread_join(&self->service_thread);
        TecSignal_set(&self->sig_stopped);
    }
    return self->error;
}

// Finish the service.
static int on_exit(void* arg) {
    TecServiceWorkerPtr self = TecServiceWorker_ptr(arg);
    TecThread exit_thread;
    TecThread_init(&exit_thread);
    // Run a thread which stops the service.
    TecThread_create(&exit_thread, service_shutdown_func, self);
    if (TecThread_ok(&exit_thread)) {
        TecThread_join(&exit_thread);
    }
    // Wait until the service has stopped.
    TecSignal_wait(&self->sig_stopped);
    // Finish the service thread.
    TecThread_join(&self->service_thread);
    return self->error;
}

/*======================================================================
*
*                      TecServiceWorker API
*
 *====================================================================*/

TECC_IMPL void TecServiceWorker_done_(TecDaemonPtr d) {
    TecServiceWorkerPtr self = TecServiceWorker_ptr(d);
    TecSignal_done(&self->sig_stopped);
    TecSignal_done(&self->sig_started);
    TecThread_join(&self->service_thread);
    TecWorker_done_(d);
}


TECC_IMPL bool TecServiceWorker_init_(TecServiceWorkerPtr self, TecServicePtr service, size_t hash_table_size) {
    bool ok = TecWorker_init(&self->worker, hash_table_size);
    self->error = 0;
    self->service = service;
    TecThread_init(&self->service_thread);
    ok = ok && TecSignal_init(&self->sig_started);
    ok = ok && TecSignal_init(&self->sig_stopped);
    if (ok) {
        TecDaemonPtr d = TecDaemon_ptr(self);
        d->done = TecServiceWorker_done_;
        self->worker.on_init = on_init;
        self->worker.on_exit = on_exit;
        service->owner = d;
    }
    return ok;
}
