// Time-stamp: <Last changed 2026-04-17 15:39:17 by magnolia>
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
#include "tecc/tecc_signal.h"
#include "tecc/tecc_service.h"


// Does nothing.
static void start(TecServicePtr self, TecSignalPtr sig_started, int* error) {
    (void)self;
    *error = 0;
    TecSignal_set(sig_started);
}

// Does nothing.
static void shutdown(TecServicePtr self, TecSignalPtr sig_stopped) {
    (void)self;
    TecSignal_set(sig_stopped);
}

// Does nothing.
static int process(TecServicePtr self, TecRequestPtr request, TecReplyPtr reply) {
    (void)self;
    (void)request;
    (void)reply;
    return -3;
}


TECC_IMPL void TecService_init_(TecServicePtr self) {
    self->start = start;
    self->shutdown = shutdown;
    self->process = process;
    self->done = NULL;
}
