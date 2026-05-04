// Time-stamp: <Last changed 2026-05-05 00:07:36 by magnolia>
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

#ifndef TECC_SERVICE_WORKER_H
#define TECC_SERVICE_WORKER_H

#include "tecc/tecc_def.h" // IWYU pragma: keep
#include "tecc/tecc_thread.h"
#include "tecc/tecc_worker.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward references.
typedef struct tagTecService TecService;
typedef TecService* TecServicePtr;

/*======================================================================
*
*          ServiceWorker combines Service and Worker for
*             running Service in a dedicated thread.
*
 *====================================================================*/

typedef struct tagTecServiceWorker TecServiceWorker;
typedef TecServiceWorker* TecServiceWorkerPtr;

// Inherited from TecWorker (824 bytes).
typedef struct tagTecServiceWorker {
    TecWorker worker;
    TecServicePtr service;
    TecSignal sig_started;
    TecSignal sig_stopped;
    TecThreadFunc service_func;
    TecThread service_thread;
} TecServiceWorker;

/*======================================================================
*
*                   TecServiceWorker API
*
 *====================================================================*/

#define TecServiceWorker_ptr(ptr) ((TecServiceWorkerPtr)(ptr))

#define TecServiceWorker_init(self, service, hash_table_size)\
    TecServiceWorker_init_(TecServiceWorker_ptr(self), TecService_ptr(service), (hash_table_size))

TECC_API bool TecServiceWorker_init_(TecServiceWorkerPtr self, TecServicePtr service, size_t hash_table_size);

// FOR CALLING FROM AN INHERITED OBJECT ONLY!
TECC_API void TecServiceWorker_done_(TecDaemonPtr);


#ifdef __cplusplus
}
#endif

#endif // TECC_SERVICE_WORKER_H
