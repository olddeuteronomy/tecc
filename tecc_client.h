// Time-stamp: <Last changed 2026-04-25 23:33:59 by magnolia>
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
#ifndef TECC_CLIENT_H
#define TECC_CLIENT_H

#include "tecc/tecc_def.h"
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_service.h"
#include "tecc/tecc_socket.h"

// Service hash table size for BSD-socket TecClient.
#define TECC_CLIENT_HASH_TABLE_SIZE 3

#ifdef __cplusplus
extern "C" {
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                    TecClient parameters
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct tagTecClientParams TecClientParams;
typedef TecClientParams* TecClientParamsPtr;

typedef struct tagTecClientParams {
    int mode;               // Client mode [0]
    size_t hash_table_size; // Client service hash table size [TECC_CLIENT_HASH_TABLE_SIZE]
} TecClientParams;

#define TecClientParams_ptr(ptr) ((TecClientParamsPtr)(ptr))

#define TecClientParams_init(ptr) TecClientParams_init_(TecClientParams_ptr(ptr))
TECC_API void TecClientParams_init_(TecClientParamsPtr);

#define TecClientParams_done(ptr) ((void)(ptr))
#define TecClientParams_done_(ptr) ((void)(ptr))

/*======================================================================
*
*              BSD-socket TecClient, inherited from TecService
*
 *====================================================================*/

typedef struct tagTecClient TecClient;
typedef TecClient* TecClientPtr;

typedef struct tagTecClient {
    TecService service;
    TecClientParamsPtr client_params;
    TecSocketParamsPtr socket_params;
    TecSocket sock;
    TecBuffer buffer;
} TecClient;

/*======================================================================
*
*                       BSD-socket TecClient API
*
 *====================================================================*/

#define TecClient_ptr(ptr) ((TecClientPtr)(ptr))

// Initialization.
#define TecClient_init(self, client_params, socket_params)\
    TecClient_init_(TecClient_ptr(self),\
                    TecClientParams_ptr(client_params),\
                    TecSocketParams_ptr(socket_params))

TECC_API void TecClient_init_(TecClientPtr self,
                              TecClientParamsPtr client_params,
                              TecSocketParamsPtr socket_params);

// Destructor.
#define TecClient_done(self) TecService_done(TecService_ptr(self))

// FOR CALLING FROM AN INHERITED OBJECT ONLY!
TECC_API void TecClient_done_(TecServicePtr self);

#ifdef __cplusplus
}
#endif

#endif // TECC_CLIENT_H
