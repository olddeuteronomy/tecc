// Time-stamp: <Last changed 2026-05-01 12:32:53 by magnolia>
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
#ifndef TECC_WORKER_H
#define TECC_WORKER_H

#include "tecc/tecc_def.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_message.h"
#include "tecc/tecc_queue.h"
#include "tecc/tecc_map.h"
#include "tecc/tecc_thread.h"
#include "tecc/tecc_daemon.h"

#ifdef __cplusplus
extern "C" {
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*          Thread-safe synchronous message-processing daemon.
*   Creates and starts a background thread that runs the message loop.
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct tagTecWorker TecWorker;
typedef TecWorker* TecWorkerPtr;

typedef int (*TecWorkerFunc)(TecWorkerPtr) ;

// Inherited from TecDaemon, see `tecc_daemon.h`.
typedef struct tagTecWorker {
    TecDaemon daemon;
    // Parameters.
    size_t hash_table_size;
    // Status.
    int error;  // 0 if OK.
    // Signals.
    TecSignal sig_running;    // Indicating the worker is running.
    TecSignal sig_terminated; // Indicating the worker has terminated.
    // Thread-safe message queue.
    TecQueue queue;
    // Message callbacks.
    TecMap callbacks;
    // Worker's init/exit handlers. Both are NULL by default.
    TecWorkerFunc on_init; // Called on starting the worker thread.
    TecWorkerFunc on_exit; // Called on exiting the worker thread if the worker has been inited successfully.
    // Worker thread.
    TecMutex mtx_guard;        // Worker thread guard.
    TecThread worker_thread;   // Worker thread.
    TecThreadFunc worker_func; // Worker function.
    // Message dispatcher.
    void (*dispatch)(TecMsgPtr, TecWorkerPtr);
    void (*on_msg)(TecMsgPtr, TecWorkerPtr); // Called on a message arrival.
    void (*on_rpc)(TecRPCPtr, TecWorkerPtr); // Called on an RPC message arrival.
} TecWorker;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                         TecWorker API
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define TecWorker_ptr(w) ((TecWorkerPtr)(w))

// Initializes the worker. Use hash_table_size=0 for the default map size.
TECC_API bool TecWorker_init(TecWorkerPtr w, size_t hash_table_size);

// Registers a callback function to process a message.
#define TecWorker_register(w, type, callback)\
    TecWorker_register_(TecWorker_ptr(w), TecMsg_type(type), (TecCallbackFunc)callback)

TECC_API void TecWorker_register_(TecWorkerPtr w, const char* func_name, TecCallbackFunc callback);

// Assigns an initialization handler that called on Worker thread starting.
#define TecWorker_set_on_init(w, h) TecWorker_ptr(w)->on_init = (TecWorkerFunc)(h)

// Assigns an exit handler that called on Worker thread exiting.
#define TecWorker_set_on_exit(w, h) TecWorker_ptr(w)->on_exit = (TecWorkerFunc)(h)

#define TecWorker_done(self) TecDaemon_done(self)

// FOR CALLING FROM AN INHERITED OBJECT ONLY.
TECC_API void TecWorker_done_(TecDaemonPtr);


#ifdef __cplusplus
}
#endif

#endif // TECC_WORKER_H
