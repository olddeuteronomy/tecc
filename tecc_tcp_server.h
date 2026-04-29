// Time-stamp: <Last changed 2026-04-29 14:49:38 by magnolia>
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
#ifndef TECC_TCP_SERVER_H
#define TECC_TCP_SERVER_H

#include <stdatomic.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_service.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_socket.h"

// Service hash table size for BSD-socket TecTCPServer.
#define TECC_TCP_SERVER_HASH_TABLE_SIZE 3


#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
*
*                    TecTCPServer parameters
*
 *====================================================================*/

typedef struct tagTecTCPServerParams TecTCPServerParams;
typedef TecTCPServerParams* TecTCPServerParamsPtr;

typedef struct tagTecTCPServerParams {
    int mode;                // Processing mode [0] - string mode.
    size_t hash_table_size;  // Service hash table size [TECC_TCP_SERVER_HASH_TABLE_SIZE].
    size_t worker_pool_size; // [0] - no worker pool.
} TecTCPServerParams;

#define TecTCPServerParams_ptr(ptr) ((TecTCPServerParamsPtr)(ptr))

#define TecTCPServerParams_init(ptr) TecTCPServerParams_init_(TecTCPServerParams_ptr(ptr))
TECC_API void TecTCPServerParams_init_(TecTCPServerParamsPtr);

#define TecTCPServerParams_done(ptr) ((void)(ptr))
#define TecTCPServerParams_done_(ptr) ((void)(ptr))

/*======================================================================
*
*               TecTPCServer, inherited from TecService
*
 *====================================================================*/

typedef struct tagTecTCPServer TecTCPServer;
typedef TecTCPServer* TecTCPServerPtr;

typedef struct tagTecTCPServer {
    TecService service;
    TecTCPServerParamsPtr server_params;
    TecSocketParamsPtr socket_params;
    TecSocket sock;
    TecBuffer buffer;
    atomic_bool running;
    TecSignal sig_polling_stopped;
    void (*poll)(TecTCPServerPtr, TecSignalPtr);
} TecTCPServer;

/*======================================================================
*
*                       BSD-socket TecTCPServer API
*
 *====================================================================*/

#define TecTCPServer_ptr(ptr) ((TecTCPServerPtr)(ptr))

// Initialization.
#define TecTCPServer_init(self, server_params, socket_params)\
    TecTCPServer_init_(TecTCPServer_ptr(self),\
                       TecTCPServerParams_ptr(server_params),\
                       TecSocketParams_ptr(socket_params))

TECC_API void TecTCPServer_init_(TecTCPServerPtr,
                                 TecTCPServerParamsPtr,
                                 TecSocketParamsPtr);

// Destructor.
#define TecTCPServer_done(self) TecService_done(TecService_ptr(self))

// FOR CALLING FROM AN INHERITED OBJECT ONLY!
TECC_API void TecTCPServer_done_(TecServicePtr);


#ifdef __cplusplus
}
#endif

#endif // TECC_TCP_SERVER_H
