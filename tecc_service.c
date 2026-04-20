// Time-stamp: <Last changed 2026-04-20 12:54:03 by magnolia>
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
static int dispatch(TecServicePtr self, TecRequestPtr request, TecReplyPtr reply) {
    TecMutex_lock(&self->mtx_guard);
    TecServiceFunc handler = (TecServiceFunc)(TecMap_get(&self->handlers, TecMsg_tag(request)));
    int error = TECC_ERR_HANDLER_NOT_FOUND;
    if (handler) {
        error =  handler(self, request, reply);
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
