// Time-stamp: <Last changed 2026-04-17 16:28:42 by magnolia>
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
#include "tecc/tecc_worker.h"
#include "tecc/tecc_service.h"

#ifdef __cplusplus
extern "C" {
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*        ServiceWorker combines Service and Worker for
*     synchronous thread-safe request processing in a dedicated thread.
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


typedef struct tagTecServiceWorker TecServiceWorker;
typedef TecServiceWorker TecServiceWorkerPtr;

typedef struct tagTecServiceWorker {

} TecServiceWorker;


#ifdef __cplusplus
}
#endif

#endif // TECC_SERVICE_WORKER_H
