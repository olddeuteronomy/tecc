// Time-stamp: <Last changed 2026-04-18 13:11:36 by magnolia>
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
#include <stdbool.h>
#include <threads.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_daemon.h"
#include "tecc/tecc_message.h"
#include "tecc/tecc_rpc.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_worker.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                       Thread functions
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static int worker_func(void* arg) {
    TecWorkerPtr w = (TecWorkerPtr)arg;
    // Call
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
    if (w->on_exit && !w->error) {
        w->error = w->on_exit(w);
    }
    TecSignal_set(&w->sig_terminated);
    return w->error;
};


// Process a "normal" message.
static void on_msg(TecMsgPtr msg, TecWorkerPtr w) {
    TecMsgCallbackFunc callback = TecMap_get(&w->msg_handlers, TecMsg_tag(msg));
    if (callback) {
        // Process a message.
        callback(msg, w);
    }
    // Deallocate a message.
    TecMsg_free(msg);
}


// Process an RPC message.
static void on_rpc(TecMsgPtr msg, TecWorkerPtr w) {
    // Handle an RPC call.
    TecRPCPtr rpc = (TecRPCPtr)msg;
    if (rpc->request) {
            TecRPCHandlerFunc handler = TecMap_get(&w->msg_handlers, TecMsg_tag(rpc->request));
            if (handler) {
                rpc->error = handler(rpc->request, rpc->reply, w);
            }
            else {
                // Handler not found.
                rpc->error = TECC_ERR_HANDLER_NOT_FOUND;
            }
            TecSignal_set(rpc->sig_ready);
    }
    // We do not deallocate an RPC request!
}


// Message dispatcher.
static void dispatch(TecMsgPtr msg, TecWorkerPtr w) {
    if (TecMsg_typeof(TecRPC, msg)) {
        w->on_rpc(msg, w);
    }
    else {
        w->on_msg(msg, w);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                     TecDaemon interface
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static int TecWorker_run_(TecDaemonPtr d) {
    TecWorkerPtr w = (TecWorkerPtr)d;
    TecMutex_lock(&w->mtx_guard);
    int status = thrd_create(&w->worker_thread, w->worker_func, w);
    if (status != thrd_success) {
        // System error.
        return TECC_ERR_SYSTEM;
    }
    TecSignal_wait(&w->sig_running);
    int result = 0;
    if (w->error) {
        thrd_join(w->worker_thread, &result);
    }
    TecMutex_unlock(&w->mtx_guard);
    return result;
}


static int TecWorker_terminate_(TecDaemonPtr d) {
    TecWorkerPtr w = (TecWorkerPtr)d;
    TecMutex_lock(&w->mtx_guard);
    int res = 0;
    TecDaemon_send(w, NULL);
    thrd_join(w->worker_thread, &res);
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
    TecMap_done(&w->msg_handlers);
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
    ok = ok && TecMap_init(&w->msg_handlers, w->hash_table_size);
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


TECC_IMPL void TecWorker_register_(TecWorkerPtr w, const char* func_name, TecMsgCallbackFunc callback) {
    TecMutex_lock(&w->mtx_guard);
    TecMap_set(&w->msg_handlers, func_name, (void*)callback);
    TecMutex_unlock(&w->mtx_guard);
}


TECC_IMPL void TecWorker_register_rpc_(TecWorkerPtr w, const char* func_name, TecRPCHandlerFunc handler) {
    TecWorker_register_(w, func_name, (void*)handler);
}
