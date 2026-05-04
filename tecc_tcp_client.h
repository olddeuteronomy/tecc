// Time-stamp: <Last changed 2026-05-05 02:32:32 by magnolia>
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
#ifndef TECC_TCP_CLIENT_H
#define TECC_TCP_CLIENT_H

#include "tecc/tecc_def.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_service.h"
#include "tecc/tecc_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
*
*      Asynchronous, thread-safe TCP client implemented as Service
*
 *====================================================================*/

typedef struct tagTecTCPClient TecTCPClient;
typedef TecTCPClient* TecTCPClientPtr;

typedef struct tagTecTCPClient {
    TecService service;
    TecSocketParamsPtr socket_params;
    TecSocket sock;
    TecBuffer buffer;
    TecMutex guard;
    int (*send)(TecTCPClientPtr, TecBufferPtr);
    int (*send_str)(TecTCPClientPtr, char*);
    int (*recv)(TecTCPClientPtr, TecBufferPtr);
    int (*send_recv)(TecTCPClientPtr self, TecBufferPtr data_in, TecBufferPtr data_out);
    int (*send_recv_str)(TecTCPClientPtr self, char* str, TecBufferPtr data_out);
} TecTCPClient;

/*======================================================================
*
*                       TecTCPClient API
*
 *====================================================================*/

#define TecTCPClient_ptr(ptr) ((TecTCPClientPtr)(ptr))

// Initialization.
#define TecTCPClient_init(self, socket_params)\
    TecTCPClient_init_(TecTCPClient_ptr(self), TecSocketParams_ptr(socket_params))

TECC_API void TecTCPClient_init_(TecTCPClientPtr, TecSocketParamsPtr);

// Destructor.
#define TecTCPClient_done(self) TecService_done(self)

// FOR CALLING FROM AN INHERITED OBJECT ONLY!
TECC_API void TecTCPClient_done_(TecServicePtr self);

#ifdef __cplusplus
}
#endif

#endif // TECC_TCP_CLIENT_H
