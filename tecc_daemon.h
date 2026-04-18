// Time-stamp: <Last changed 2026-04-18 13:55:24 by magnolia>
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
#ifndef TECC_DAEMON_H
#define TECC_DAEMON_H

#include "tecc/tecc_def.h" // IWYU pragma: keep
#include "tecc/tecc_signal.h"
#include "tecc/tecc_message.h"
#include "tecc/tecc_rpc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*        Daemon - an abstract interface for long-lived service
*                    and processing component
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct tagTecDaemon TecDaemon;
typedef TecDaemon* TecDaemonPtr;

typedef struct tagTecDaemon {
    // Reserved for flags.
    unsigned long flags;
    // Signaled when the daemon is running.
    TecSignalPtr sig_running;
    // Signaled when the daemon has terminated.
    TecSignalPtr sig_terminated;
    // Starts the daemon. Returns 0 on success.
    int (*run)(TecDaemonPtr);
    // Stops the daemon. Returns 0 on success.
    int (*terminate)(TecDaemonPtr);
    // Sends a message to the daemon for non-blocking processing;
    // the daemon processes the message asynchronously.
    void (*send)(TecDaemonPtr, TecMsgPtr);
    // Calls the RPC-style procedure (blocks until reply).
    // Returns 0 on success; on error, returns a domain-specific error code.
    int (*rpc)(TecDaemonPtr, TecRequestPtr, TecReplyPtr);
    // Destructor. Called if not NULL (default).
    void (*done)(TecDaemonPtr);
} TecDaemon;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                        Daemon API
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define TECC_ERR_OK                 0
#define TECC_ERR_SYSTEM            -1
#define TECC_ERR_HANDLER_NOT_FOUND -2

#define TecDaemon_ptr(self) ((TecDaemonPtr)(self))

#define TecDaemon_init(self)\
    TecDaemon_ptr(self)->flags = 0;\
    TecDaemon_ptr(self)->sig_running = NULL;\
    TecDaemon_ptr(self)->sig_terminated = NULL;\
    TecDaemon_ptr(self)->run = NULL;\
    TecDaemon_ptr(self)->terminate = NULL;\
    TecDaemon_ptr(self)->send = NULL;\
    TecDaemon_ptr(self)->rpc = NULL;\
    TecDaemon_ptr(self)->done = NULL

#define TecDaemon_run(self)\
    TecDaemon_ptr(self)->run(TecDaemon_ptr(self))

#define TecDaemon_terminate(self)\
    TecDaemon_ptr(self)->terminate(TecDaemon_ptr(self))

#define TecDaemon_get_running_signal(self)\
    TecDaemon_ptr(self)->sig_running

#define TecDaemon_get_terminated_signal(self)\
    TecDaemon_ptr(self)->sig_terminated

#define TecDaemon_wait_until_running(self)\
    TecSignal_wait(TecDaemon_ptr(self)->sig_running)

#define TecDaemon_wait_until_terminated(self)\
    TecSignal_wait(TecDaemon_ptr(self)->sig_terminated)

#define TecDaemon_send(self, msg)\
    TecDaemon_ptr(self)->send(TecDaemon_ptr(self), (TecMsgPtr)(msg))

#define TecDaemon_rpc(self, request, reply)\
    TecDaemon_ptr(self)->rpc(TecDaemon_ptr(self), (TecRequestPtr)(request), (TecReplyPtr)(reply))

#define TecDaemon_done_func(self) (TecDaemon_ptr(self)->done)

#define TecDaemon_done(self)\
    if (TecDaemon_done_func(TecDaemon_ptr(self))) TecDaemon_done_func(TecDaemon_ptr(self))(TecDaemon_ptr(self));\
    TecDaemon_done_func(self) = NULL

// FOR CALLING FROM AN INHERITED OBJECT ONLY!
#define TecDaemon_done_(self) ((void)(self))

#ifdef __cplusplus
}
#endif

#endif // TECC_DAEMON_H
