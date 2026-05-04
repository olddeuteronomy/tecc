// Time-stamp: <Last changed 2026-05-05 02:32:13 by magnolia>
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

#ifdef __cplusplus
extern "C" {
#endif

// Forward references.
typedef struct tagTecDaemon TecDaemon;
typedef TecDaemon* TecDaemonPtr;

/*======================================================================
*
*   A generic long-lived service with `start`/`shutdown` semantics.
*
 *====================================================================*/

typedef struct tagTecService TecService;
typedef TecService* TecServicePtr;

typedef struct tagTecService {
    // A pointer to the owning Daemon object (may be NULL).
    TecDaemonPtr owner;
    // Starts the service and signals completion.
    // In long-running services like TCP servers, this function may not return
    // until `shutdown` is invoked from another thread.
    void (*start)(TecServicePtr, TecSignalPtr, int*);
    // Stops the service and signals termination.
    void (*shutdown)(TecServicePtr, TecSignalPtr);
    // Destructor.
    void (*done)(TecServicePtr self);
} TecService;

/*======================================================================
*
*                       Service API
*
 *====================================================================*/

#define TecService_ptr(self) ((TecServicePtr)(self))

#define TecService_init(self) TecService_init_(TecService_ptr(self))

TECC_API bool TecService_init_(TecServicePtr);

#define TecService_start(self, started, error) do {\
    TecServicePtr svc = TecService_ptr(self);\
    svc->start(svc, (started), (error)); } while(0)

#define TecService_shutdown(self, stopped) do {\
    TecServicePtr svc = TecService_ptr(self);\
    svc->shutdown(svc, (stopped)); } while(0)

// Destructor.
#define TecService_done(self) do {\
    TecServicePtr s = TecService_ptr(self);\
    if (s->done) { s->done(s); } } while (0)

// FOR CALLING FROM AN INHERITED OBJECT ONLY!
TECC_API void TecService_done_(TecServicePtr);


#ifdef __cplusplus
}
#endif

#endif // TECC_SERVICE_H
