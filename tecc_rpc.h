// Time-stamp: <Last changed 2026-04-30 15:05:13 by magnolia>
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
#ifndef TECC_REQUEST_H
#define TECC_REQUEST_H

#include "tecc/tecc_def.h"
#include "tecc/tecc_message.h"
#include "tecc/tecc_signal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward references.
typedef struct tagTecRPC TecRPC;
typedef TecRPC* TecRPCPtr;

typedef TecMsg TecRequest;
typedef TecMsgPtr TecRequestPtr;

typedef TecMsg TecReply;
typedef TecMsgPtr TecReplyPtr;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*          RPC object holding both Request and Reply messages
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Should return 0 on success, or domain-specific error code.
typedef int (*TecRPCHandlerFunc)(TecRequestPtr, TecReplyPtr, void*);

TECC_DEF_MESSAGE(TecRPC)
    TecRequestPtr request;
    TecReplyPtr reply;
    TecSignalPtr sig_ready;
    int error;
TECC_END_MESSAGE(TecRPC)

TECC_API void TecRPC_init(TecRPCPtr, TecRequestPtr, TecReplyPtr, TecSignalPtr);
TECC_API void TecRPC_done(TecRPCPtr);


#ifdef __cplusplus
}
#endif

#endif // TECC_REQUEST_H
