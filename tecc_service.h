// Time-stamp: <Last changed 2026-04-21 15:34:15 by magnolia>
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
#include "tecc/tecc_map.h"
#include "tecc/tecc_message.h"
#include "tecc/tecc_rpc.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward references.
typedef struct tagTecDaemon TecDaemon;
typedef TecDaemon* TecDaemonPtr;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*   A generic, thread-safe service with `start`/`shutdown` semantics
*         and `request`-`reply` RPC-style message handling.
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct tagTecService TecService;
typedef TecService* TecServicePtr;

typedef int (*TecServiceFunc)(TecRequestPtr, TecReplyPtr, void*);

typedef struct tagTecService {
    // A pointer to the owning Daemon object (may be NULL).
    TecDaemonPtr owner;
    // Status; 0 if OK.
    int error;
    // Starts the service and signals completion.
    // In long-running services like TCP servers, this function may not return
    // until `shutdown` is invoked from another thread.
    void (*start)(TecServicePtr, TecSignalPtr, int*);
    // Stops the service and signals termination.
    void (*shutdown)(TecServicePtr, TecSignalPtr);
    // RPC-style request handlers.
    TecMap handlers;
    // Process an RPC request.
    TecServiceFunc dispatch;
    // Guard
    TecMutex mtx_guard;
    // Destructor.
    void (*done)(TecServicePtr self);
} TecService;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                       Service API
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define TecService_ptr(self) ((TecServicePtr)(self))

#define TecService_init(self, hash_table_size)\
    TecService_init_(TecService_ptr(self), hash_table_size)

TECC_API bool TecService_init_(TecServicePtr self, size_t hash_table_size);

// Register an RPC handler.
#define TecService_register(self, request_type, handler)\
    TecService_register_(TecService_ptr(self), TecMsg_type(request_type), (TecServiceFunc)(handler))

TECC_API void TecService_register_(TecServicePtr self, const char* func_name, TecServiceFunc handler);

// Makes an RPC through the daemon if possible (using the daemon's thread).
#define TecService_rpc(self, request, reply)\
    TecService_rpc_(TecService_ptr(self), (TecRequestPtr)(request), (TecReplyPtr)(reply))

TECC_API int TecService_rpc_(TecServicePtr self, TecRequestPtr request, TecReplyPtr reply);


// Destructor.
#define TecService_done_func(self) (TecService_ptr(self)->done)
#define TecService_done(self)\
    if(TecService_done_func(self)) TecService_done_func(self)(TecService_ptr(self))

// FOR CALLING FROM AN INHERITED OBJECT ONLY!
TECC_API void TecService_done_(TecServicePtr);


#ifdef __cplusplus
}
#endif

#endif // TECC_SERVICE_H
