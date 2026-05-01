// Time-stamp: <Last changed 2026-05-01 15:01:09 by magnolia>
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
#ifndef TECC_WORKER_POOL_H
#define TECC_WORKER_POOL_H

#include <stdbool.h>

#include "tecc/tecc_def.h" // IWYU pragma: keep
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_worker.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagTecWorkerPool TecWorkerPool;
typedef TecWorkerPool* TecWorkerPoolPtr;

typedef struct tagTecPoolNode TecPoolNode;
typedef TecPoolNode*  TecPoolNodePtr;

typedef struct tagTecPoolNode {
    TecWorker worker;
    TecBuffer buffer;
} TecPoolNode;

typedef struct tagTecWorkerPool {
    size_t nworkers;
    TecWorkerPtr workers;
    char* buffer;
} TecWorkerPool;

/*======================================================================
*
*                      TecWorkerPool API
*
 *====================================================================*/

TECC_API bool TecWorkerPool_init(TecWorkerPoolPtr self, size_t nworkers, size_t bufsize);
TECC_API void TecWorkerPool_done(TecWorkerPoolPtr self);
TECC_API bool TecWorkerPool_run(TecWorkerPoolPtr self);

#ifdef __cplusplus
}
#endif

#endif // TECC_WORKER_POOL_H
