// Time-stamp: <Last changed 2026-04-21 14:16:40 by magnolia>
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

#include "tecc/tecc_def.h"
#include "tecc/tecc_rpc.h"
#include "tecc/tecc_trace.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_worker.h"
#include "tecc/tecc_trace.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                       Thread functions
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Runs the message loop.
static int worker_func(void* arg) {
    TECC_TRACE_ENTER("worker_func()");
    TecWorkerPtr w = (TecWorkerPtr)arg;
    // Initialization.
    if (w->on_init && !w->error) {
        w->error = w->on_init(w);
    }
    TecSignal_set(&w->sig_running);
    if (!w->error) {
        while (true) {
            TecMsg* msg = TecQueue_pop(&w->queue);
            if (msg == NULL) {
                break;
            }
            w->dispatch(msg, w);
        }
    }
    // Exiting.
    if (w->on_exit && !w->error) {
        w->error = w->on_exit(w);
    }
    TecSignal_set(&w->sig_terminated);
    TECC_TRACE_EXIT();
    return w->error;
}


// Processes a "normal" message.
static void on_msg(TecMsgPtr msg, TecWorkerPtr w) {
    TecCallbackFunc callback = (TecCallbackFunc)TecMap_get(&w->callbacks, TecMsg_tag(msg));
    if (callback) {
        // Process a message.
        callback(msg, w);
    }
    // Deallocate a message.
    TecMsg_free(msg);
}


// Processes an RPC message.
static void on_rpc(TecRPCPtr rpc, TecWorkerPtr w) {
    // RPC request is not supported in Worker.
    (void)w;
    rpc->error = TECC_ERR_NOT_SUPPORTED;
    TecSignal_set(rpc->sig_ready);
    // We do not deallocate an RPC request!
}


// Message dispatcher.
static void dispatch(TecMsgPtr msg, TecWorkerPtr w) {
    TECC_TRACE_ENTER("Worker.dispatch()");
    TECC_TRACE("MsgType: %s\n", TecMsg_tag(msg));
    if (TecMsg_typeof(TecRPC, msg)) {
        w->on_rpc((TecRPCPtr)msg, w);
    }
    else {
        w->on_msg(msg, w);
    }
    TECC_TRACE_EXIT();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                     TecDaemon interface
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static int TecWorker_run_(TecDaemonPtr d) {
    TecWorkerPtr w = (TecWorkerPtr)d;
    TecMutex_lock(&w->mtx_guard);
    // Create the worker thread.
    TecThread_create(&w->worker_thread, w->worker_func, w);
    if (!w->worker_thread.ok) {
        // System error.
        TecMutex_unlock(&w->mtx_guard);
        return TECC_ERR_SYSTEM;
    }
    // Waits until the worker thread has started.
    TecSignal_wait(&w->sig_running);
    if (w->error) {
        // Finish the worker thread.
        TecThread_join(&w->worker_thread);
    }
    TecMutex_unlock(&w->mtx_guard);
    return w->error;
}


static int TecWorker_terminate_(TecDaemonPtr d) {
    TecWorkerPtr w = (TecWorkerPtr)d;
    TecMutex_lock(&w->mtx_guard);
    TecDaemon_send(w, NULL);
    TecThread_join(&w->worker_thread);
    int res = w->error;
    TecMutex_unlock(&w->mtx_guard);
    return res;
}


static void TecWorker_send_(TecDaemonPtr d, TecMsgPtr msg) {
    TecWorkerPtr w = (TecWorkerPtr)d;
    TecQueue_push(&w->queue, msg);
}


static int TecWorker_rpc_(TecDaemonPtr d, TecRequestPtr request, TecReplyPtr reply) {
    TecWorkerPtr w = (TecWorkerPtr)d;
    TecSignal sig_ready;
    TecSignal_init(&sig_ready);
    TecRPC rpc;
    TecRPC_init(&rpc, request, reply, &sig_ready);
    TecDaemon_send(w, &rpc);
    TecSignal_wait(&sig_ready);
    TecRPC_done(&rpc);
    TecSignal_done(&sig_ready);
    return rpc.error;
}


TECC_IMPL void TecWorker_done_(TecDaemonPtr d) {
    TecWorkerPtr w = (TecWorkerPtr)d;
    TecSignal_done(&w->sig_running);
    TecSignal_done(&w->sig_terminated);
    TecQueue_done(&w->queue);
    TecMap_done(&w->callbacks);
    TecMutex_destroy(&w->mtx_guard);
    // Clean up the parent object.
    TecDaemon_done_(d);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                         TecWorker interface
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

TECC_IMPL bool TecWorker_init(TecWorkerPtr w, size_t hash_table_size) {
    // Initialize the parent.
    TecDaemon_init(TecDaemon_ptr(w));
    // Initialize self.
    bool ok = true;
    w->hash_table_size = hash_table_size;
    // Flags
    w->error = false;
    // Initialize
    ok = ok && TecSignal_init(&w->sig_running);
    ok = ok && TecSignal_init(&w->sig_terminated);
    ok = ok && TecQueue_init(&w->queue);
    ok = ok && TecMap_init(&w->callbacks, w->hash_table_size);
    ok = ok && TecMutex_init(&w->mtx_guard);
    w->on_init = NULL; // No special initialization.
    w->on_exit = NULL; // No special exiting.
    w->worker_func = worker_func; // Default worker thread function.
    w->dispatch = dispatch;       // Default dispatcher.
    w->on_msg = on_msg; // Default message handler.
    w->on_rpc = on_rpc; // Default RPC handler.
    if (ok) {
        // Initialize the Daemon.
        w->daemon.sig_running = &w->sig_running;
        w->daemon.sig_terminated = &w->sig_terminated;
        w->daemon.run = TecWorker_run_;
        w->daemon.terminate = TecWorker_terminate_;
        w->daemon.send = TecWorker_send_;
        w->daemon.rpc = TecWorker_rpc_;
        w->daemon.done = TecWorker_done_;
    }
    return ok;
}


TECC_IMPL void TecWorker_register_(TecWorkerPtr w, const char* func_name, TecCallbackFunc callback) {
    TecMutex_lock(&w->mtx_guard);
    TecMap_set(&w->callbacks, func_name, callback);
    TecMutex_unlock(&w->mtx_guard);
}
