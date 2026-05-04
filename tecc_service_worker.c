// Time-stamp: <Last changed 2026-05-05 00:01:38 by magnolia>
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
#include "tecc/tecc_daemon.h"
#include "tecc/tecc_service.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_thread.h"
#include "tecc/tecc_worker.h"
#include "tecc/tecc_service_worker.h"


/*======================================================================
*
*                    TecWorker overrides
*
 *====================================================================*/

static int service_start_func(void* arg) {
    TecServiceWorkerPtr w = TecServiceWorker_ptr(arg);
    TecServicePtr service = w->service;
    int error = 0;
    service->start(service, &w->sig_started, &error);
    return error;
}

static int service_shutdown_func(void* arg) {
    TecServiceWorkerPtr w = TecServiceWorker_ptr(arg);
    TecServicePtr service = w->service;
    service->shutdown(service, &w->sig_stopped);
    return 0;
}

// Creates and runs the service thread which calls `service->start()`.
static int on_init(TecWorkerPtr w) {
    TecServiceWorkerPtr self = TecServiceWorker_ptr(w);
    TecThread_create(&self->service_thread, service_start_func, self);
    if (!self->service_thread.ok) {
        return self->service_thread.res;
    }
    // Wait until the Service has started.
    TecSignal_wait(&self->sig_started);
    return self->service_thread.res;
}

// Finish the service.
static int on_exit(TecWorkerPtr w) {
    TecServiceWorkerPtr self = TecServiceWorker_ptr(w);
    TecThread exit_thread;
    // Start a thread which stops the service.
    TecThread_create(&exit_thread, service_shutdown_func, self);
    if (!exit_thread.ok) {
        return exit_thread.res;
    }
    TecThread_join(&exit_thread);
    // Wait until the service has stopped.
    TecSignal_wait(&self->sig_stopped);
    // Finish the service thread.
    TecThread_join(&self->service_thread);
    return exit_thread.res;
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
    TecWorker_done_(d);
}


TECC_IMPL bool TecServiceWorker_init_(TecServiceWorkerPtr self, TecServicePtr service, size_t hash_table_size) {
    bool ok = TecWorker_init(&self->worker, hash_table_size);
    ok &= TecSignal_init(&self->sig_started);
    ok &= TecSignal_init(&self->sig_stopped);
    self->service = service;
    if (ok) {
        TecDaemonPtr d = TecDaemon_ptr(self);
        d->done = TecServiceWorker_done_;
        self->worker.on_init = on_init;
        self->worker.on_exit = on_exit;
        service->owner = d;
    }
    return ok;
}
