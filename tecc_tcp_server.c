// Time-stamp: <Last changed 2026-05-10 14:31:46 by magnolia>
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
#include <stdatomic.h>
#include <stdbool.h>

#include "tecc/tecc_def.h"    // IWYU pragma: keep
#include "tecc/tecc_trace.h"  // IWYU pragma: keep
#include "tecc/tecc_signal.h"
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_socket.h"
#include "tecc/tecc_service.h"
#include "tecc/tecc_thread_pool.h"
#include "tecc/tecc_tcp_server.h"


TECC_IMPL void TecTCPServerParams_init_(TecTCPServerParamsPtr self) {
    self->worker_pool_size = 0;
}

/*======================================================================
*
*                      TecTCPServer API
*
 *====================================================================*/

// Default processing.
static void process_client(TecSocketPtr sock) {
    (void)sock;
    TECC_TRACE_ENTER("process_client()");
    TECC_TRACE_EXIT();
}

static void dispatch_client(TecSocketPtr sock, TecBuffer buf, TecTCPServerPtr srv) {
    TECC_TRACE_ENTER("dispatch_client()");
    sock->buf = buf;
    srv->process_client(sock);
    TecSocket_done(sock);
    TECC_TRACE_EXIT();
}


static void poll_(TecTCPServerPtr self) {
    TECC_TRACE_ENTER("TecTCPServer::poll()");
    while (atomic_load(&self->running)) {
        // Get client socket.
        TecSocket cli = TecSocket_accept(&self->sock);
        if (TecSocket_is_valid(&cli)) {
            if (self->thread_pool) {
                // Multi-threaded server that uses the thread pool.
                TecThrPool_enqueue(self->thread_pool, (TecTaskFunc)dispatch_client,
                                   &cli, self);
            }
            else {
                // Single-threaded server.
                dispatch_client(&cli, self->buffer, self);
            }
        }
    }
    TECC_TRACE_EXIT();
    TecSignal_set(&self->sig_polling_stopped);
}

/*======================================================================
*
*                      TecService API
*
 *====================================================================*/

static void start_(TecServicePtr svc, TecSignalPtr sig_started, int* error) {
    TECC_TRACE_ENTER("TecTCPServer::start()");
    TecTCPServerPtr self = TecTCPServer_ptr(svc);
    // Start the TCP server.
    TecSocketPtr sock = &self->sock;
    int err = TecSocket_open(sock);
    if (!err) {
        TecSocket_set_options(sock);
        err = TecSocket_bind(sock);
    }
    if (!err) {
        err = TecSocket_listen(sock);
    }
    *error = err;
    if (err) {
        TECC_TRACE("!!! Failed to start with code=%d.\n", err);
        TecSignal_set(&self->sig_polling_stopped);
        TecSignal_set(sig_started);
    }
    else {
        // Use local buffer if no thread pool attached.
        if (self->thread_pool == NULL) {
            TecBuffer_init(&self->buffer,
                           self->socket_params->buffer_size,
                           self->socket_params->buffer_size);
        }
        // Start polling.
        TECC_TRACE("Service started OK.\n");
        TecSignal_set(sig_started);
        self->poll(self);
    }
    TECC_TRACE_EXIT();
}


static void shutdown_(TecServicePtr svc, TecSignalPtr sig_stopped) {
    TecTCPServerPtr self = TecTCPServer_ptr(svc);
    // Close listening socket.
    TecSocket_close(&self->sock);
    atomic_store(&self->running, false);
    // Wait until polling stops.
    TecSignal_wait(&self->sig_polling_stopped);
    TecSignal_set(sig_stopped);
}

/*======================================================================
*
*                    TecTCPServer API
*
 *====================================================================*/

TECC_IMPL void TecTCPServer_init_(TecTCPServerPtr self,
                                  TecTCPServerParamsPtr server_params,
                                  TecSocketParamsPtr socket_params) {
    TecService_init(&self->service);
    self->server_params = server_params;
    self->socket_params = socket_params;
    // Initialize the listening socket.
    TecSocket_init_server(&self->sock, socket_params);
    // Internal buffer for single-threaded server; initially empty.
    TecBuffer_init(&self->buffer, 0, self->socket_params->buffer_size);
    // Polling.
    TecSignal_init(&self->sig_polling_stopped);
    atomic_init(&self->running, true);
    self->thread_pool = NULL; // Single-threaded server by default.
    // Overrides.
    self->service.start = start_;
    self->service.shutdown = shutdown_;
    self->service.done = TecTCPServer_done_;
    // TecTCPServer API.
    self->poll = poll_;
    self->dispatch_client = dispatch_client;
    self->process_client = process_client;
}


TECC_IMPL void TecTCPServer_done_(TecServicePtr svc) {
    TecTCPServerPtr self = TecTCPServer_ptr(svc);
    TecSignal_done(&self->sig_polling_stopped);
    TecSocket_done(&self->sock);
    TecBuffer_done(&self->buffer);
    TecService_done_(&self->service);
}
