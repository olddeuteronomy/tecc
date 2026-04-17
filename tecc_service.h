// Time-stamp: <Last changed 2026-04-17 15:41:43 by magnolia>
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
#ifndef TECC_SERVICE_H
#define TECC_SERVICE_H

#include "tecc/tecc_def.h" // IWYU pragma: keep
#include "tecc/tecc_signal.h"
#include "tecc/tecc_rpc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* A generic, non-thread-safe service with `start`/`shutdown` semantics
*         and `request`-`reply` RPC-style message handling.
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct tagTecService TecService;
typedef TecService* TecServicePtr;

typedef struct tagTecService {
    // Starts the service and signals completion.
    // In long-running services, this function may not return
    // until `shutdown` is invoked from another thread.
    void (*start)(TecServicePtr self, TecSignalPtr sig_started, int* error);
    // Stops the service and signals termination.
    void (*shutdown)(TecServicePtr self, TecSignalPtr sig_stopped);
    // Process an RPC-style request.
    int (*process)(TecServicePtr self, TecRequestPtr request, TecReplyPtr reply);
    // Destructor. Default is NULL.
    void (*done)(TecServicePtr self);
} TecService;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                       Service API
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define TecService_ptr(self) ((TecServicePtr)(self))

TECC_API void TecService_init_(TecServicePtr self);
#define TecSerice_init(self) TecService_init(TecService_ptr(self))

#define TecService_start(self, sig_started, error)\
    TecService_ptr(self)->start(TecService_ptr(self), sig_started, error)

#define TecService_shutdown(self, sig_started)\
    TecService_ptr(self)->shutdown(TecService_ptr(self), sig_stopped)

#define TecService_process(self, request, reply)\
    TecService_ptr(self)->process(TecService_ptr(self), request, reply)

#define TecService_done_func(self) (TecService_ptr(self)->done)

#define TecService_done(self)\
    if(TecService_done_func(self)) TecService_done_func(self)(TecService_ptr(self))

// FOR CALLING FROM AN INHERITED OBJECT!
#define TecService_done_(self) ((void)(self))


#ifdef __cplusplus
}
#endif

#endif // TECC_SERVICE_H
