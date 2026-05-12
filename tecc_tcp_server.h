// Time-stamp: <Last changed 2026-05-12 12:13:11 by magnolia>
/*----------------------------------------------------------------------
------------------------------------------------------------------------
Copyright (c) 2026 The Emacs Cat (https://github.com/olddeuteronomy/tecc).

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
#ifndef TECC_TCP_SERVER_H
#define TECC_TCP_SERVER_H

#include <stdatomic.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_service.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
*
*               TecTPCServer, inherited from TecService
*
 *====================================================================*/

// Forward references.
typedef struct tagTecThrPool TecThrPool;
typedef TecThrPool* TecThrPoolPtr;

typedef struct tagTecTCPServer TecTCPServer;
typedef TecTCPServer* TecTCPServerPtr;

// A handler for incoming connections.
typedef int (*TecTCPClientFunc)(TecSocketPtr, void* arg);

// Server special log statuses.
enum {
    TECC_LOG_SVR_CONNECTING = 29001,
    TECC_LOG_SVR_CONN_OK,
    TECC_LOG_SVR_CONN_FAILED,
    TECC_LOG_SVR_SHUTDOWN
};
// Logging.
typedef void (*TecTCPServerLogFunc)(int, TecSocketPtr, TecTCPServerPtr);

// Multithreaded (if configured with thread pool) TCP server. 368 bytes.
typedef struct tagTecTCPServer {
    TecService service;
    TecSocketParamsPtr socket_params;
    // Listening socket.
    TecSocket sock;
    // Polling.
    atomic_bool running;
    TecSignal sig_polling_stopped;
    void (*poll)(TecTCPServerPtr);
    // Incoming connections.
    TecBuffer buffer;          // Single-threaded server buffer.
    TecThrPoolPtr thread_pool; // NULL by default (single-threaded server).
    void (*dispatch_client)(TecSocketPtr, TecBuffer, TecTCPServerPtr);
    // Logging
    TecTCPServerLogFunc log;   // NULL, no logging.
    // A handler for incoming connections.
    TecTCPClientFunc client_proc;
} TecTCPServer;

/*======================================================================
*
*                         TecTCPServer API
*
 *====================================================================*/

#define TecTCPServer_ptr(ptr) ((TecTCPServerPtr)(ptr))

// Initializes the server using the specified listening‑socket parameters.
TECC_API void TecTCPServer_init(TecTCPServerPtr, TecSocketParamsPtr);

// Destructor.
#define TecTCPServer_done(self) TecService_done(TecService_ptr(self))

// Sets a handler for incoming connections.
TECC_API void TecTCPServer_set_client_proc(TecTCPServerPtr, TecTCPClientFunc);

// Attaches a thread pool that handles incoming connections concurrently.
TECC_API void TecTCPServer_use_thread_pool(TecTCPServerPtr, TecThrPoolPtr);

// FOR CALLING FROM AN INHERITED OBJECT ONLY!
TECC_API void TecTCPServer_done_(TecServicePtr);


#ifdef __cplusplus
}
#endif

#endif // TECC_TCP_SERVER_H
