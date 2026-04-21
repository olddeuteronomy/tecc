// Time-stamp: <Last changed 2026-04-21 15:27:41 by magnolia>
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
#include "tecc/tecc_trace.h"
#include "tecc/tecc_service.h"


// Default implementation does nothing.
static void start(TecServicePtr self, TecSignalPtr sig_started, int* error) {
    (void)self;
    *error = 0;
    TecSignal_set(sig_started);
}

// Default implementation does nothing.
static void shutdown(TecServicePtr self, TecSignalPtr sig_stopped) {
    (void)self;
    TecSignal_set(sig_stopped);
}


// Calls a registered RPC handler.
static int dispatch(TecRequestPtr request, TecReplyPtr reply, void* args) {
    TecServicePtr self = TecService_ptr(args);
    TecMutex_lock(&self->mtx_guard);
    int error = TECC_ERR_HANDLER_NOT_FOUND;
    TecServiceFunc handler = (TecServiceFunc)(TecMap_get(&self->handlers, TecMsg_tag(request)));
    if (handler) {
        error =  handler(request, reply, self);
    }
    TecMutex_unlock(&self->mtx_guard);
    return error;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                        TecService API
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


TECC_IMPL void TecService_done_(TecServicePtr self) {
    TecMap_done(&self->handlers);
    TecMutex_destroy(&self->mtx_guard);
}


TECC_IMPL bool TecService_init_(TecServicePtr self, size_t hash_table_size) {
    self->owner = NULL;
    self->error = 0;
    self->start = start;
    self->shutdown = shutdown;
    self->dispatch = dispatch;
    self->done = TecService_done_;
    TecMap_init(&self->handlers, hash_table_size);
    return TecMutex_init(&self->mtx_guard);
}


TECC_IMPL void TecService_register_(TecServicePtr self, const char* func_name, TecServiceFunc handler) {
    TecMutex_lock(&self->mtx_guard);
    TecMap_set(&self->handlers, func_name, (void*)handler);
    TecMutex_unlock(&self->mtx_guard);
}


TECC_IMPL int TecService_rpc_(TecServicePtr self, TecRequestPtr request, TecReplyPtr reply) {
    TECC_TRACE_ENTER("Service_rpc()");
    int error = 0;
    if (self->owner) {
        // Make a call throught the daemon.
        TECC_TRACE("Dispatching through the daemon...\n");
        error = TecDaemon_rpc(self->owner, request, reply);
        /* error = self->owner->rpc(self->owner, request, reply); */
    }
    else {
        // Dispatch through itself.
        TECC_TRACE("Dispatching through the service...\n");
        error = self->dispatch(request, reply, self);
    }
    TECC_TRACE_EXIT();
    return error;
}
