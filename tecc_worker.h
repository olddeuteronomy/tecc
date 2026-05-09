// Time-stamp: <Last changed 2026-05-09 14:52:19 by magnolia>
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
#include "tecc/tecc_daemon.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_message.h"
#include "tecc/tecc_queue.h"
#include "tecc/tecc_map.h"
#include "tecc/tecc_threads.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
*
*          Thread-safe synchronous message-processing daemon.
*   Creates and starts a background thread that runs the message loop.
*
 *====================================================================*/

typedef struct tagTecWorker TecWorker;
typedef TecWorker* TecWorkerPtr;

#ifdef TECC_PTHREAD
// POSIX
#define TECC_THREAD_FUNC_RETVAL void*
#define TECC_THREAD_FUNC_RETURN(val_) return &(val_)
typedef void* (*TecWorkerFunc)(TecWorkerPtr);

#else
// STDC
#define TECC_THREAD_FUNC_RETVAL int
#define TECC_THREAD_FUNC_RETURN(val_) return (val_)
typedef int (*TecWorkerFunc)(TecWorkerPtr);
#endif

// Inherited from TecDaemon, see `tecc_daemon.h`. 560 bytes.
typedef struct tagTecWorker {
    TecDaemon daemon;
    // Parameters.
    size_t hash_table_size; //
    // Status.
    int error;  // 0 if OK.
    // Signals.
    TecSignal sig_running;    // Indicates then the worker thread has started.
    TecSignal sig_terminated; // Indicates that the worker thread has terminated.
    // Thread-safe message queue.
    TecQueue queue;
    // Message handlers.
    TecMap callbacks;
    // Worker's init/exit handlers. Both are NULL by default.
    int (*on_init)(void* arg); // Called when the worker thread starts.
    int (*on_exit)(void* arg); // Called when the worker thread exits.
    // Worker thread.
    TecMutex lock;             // Worker thread guard.
    TecThread worker_thread;   // Worker thread.
    TecThreadFunc worker_func; // Worker function.
    // Message dispatchers. May be overwritten.
    void (*dispatch)(TecMsgPtr, void*);
    void (*on_msg)(TecMsgPtr, void*); // Called when a message arrives.
    void (*on_rpc)(TecRPCPtr, void*); // Called when an RPC message arrives.
} TecWorker;

/*======================================================================
*
*                         TecWorker API
*
 *====================================================================*/
#define TecWorker_ptr(w) ((TecWorkerPtr)(w))

// Initializes the worker. Use hash_table_size=0 for the default map size (117).
TECC_API bool TecWorker_init(TecWorkerPtr w, size_t hash_table_size);

// Registers a handler that processes messages of type `type`.
#define TecWorker_register(w, type, callback)\
    TecWorker_register_(TecWorker_ptr(w), TecMsg_type(type), (TecCallbackFunc)(callback))

TECC_API void TecWorker_register_(TecWorkerPtr w, const char* func_name, TecCallbackFunc callback);

// Assigns an initialization handler that is called when a worker thread starts.
#define TecWorker_set_on_init(w, h) TecWorker_ptr(w)->on_init = (h)

// Assigns an exit handler that is called when a worker thread exits.
#define TecWorker_set_on_exit(w, h) TecWorker_ptr(w)->on_exit = (h)

#define TecWorker_done(w) TecDaemon_done(w)

// FOR CALLING FROM AN INHERITED OBJECT ONLY.
TECC_API void TecWorker_done_(TecDaemonPtr);


#ifdef __cplusplus
}
#endif

#endif // TECC_WORKER_H
