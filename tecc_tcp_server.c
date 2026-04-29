// Time-stamp: <Last changed 2026-04-29 15:14:28 by magnolia>
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

#include "tecc/tecc_buffer.h"
#include "tecc/tecc_def.h"
#include "tecc/tecc_service.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_socket.h"
#include <sys/socket.h>
#include "tecc/tecc_tcp_server.h"


TECC_IMPL void TecTCPServerParams_init_(TecTCPServerParamsPtr self) {
    self->mode = 0;
    self->hash_table_size = TECC_TCP_SERVER_HASH_TABLE_SIZE;
    self->worker_pool_size = 0;
}


/*======================================================================
*
*                      TecService API
*
 *====================================================================*/

static void poll_(TecTCPServerPtr self, TecSignalPtr sig_started) {
    TecSignal_set(sig_started);
    while (atomic_load(&self->running)) {
        // Get client socket.
        TecSocket cli = TecSocket_accept(&self->sock);
        if (TecSocket_is_valid(&cli)) {
            // TODO
        }
        if (self->server_params->worker_pool_size == 0) {
            TecSocket_done(&cli);
        }
    }
    TecSignal_set(&self->sig_polling_stopped);
}

static void start_(TecServicePtr svc, TecSignalPtr sig_started, int* error) {
    TecTCPServerPtr self = TecTCPServer_ptr(svc);
    TecSocketPtr sock = &self->sock;
    int err = TecSocket_open(sock);
    if (!err) {
        TecSocket_set_options(sock);
        err = TecSocket_bind(sock);
    }
    if (!err) {
        err = TecSocket_listen(sock);
    }
    if (err) {
        *error = err;
        TecSignal_set(&self->sig_polling_stopped);
        TecSignal_set(sig_started);
        return;
    }
    // Allocate an internal buffer and start polling incoming connections.
    if (self->server_params->worker_pool_size == 0) {
        size_t bufsize = sock->params->buffer_size;
        TecBuffer_init(&self->buffer, bufsize, bufsize);
        self->sock.buf = self->buffer;
    }
    *error = err;
    self->poll(self, sig_started);
}


static void shutdown_(TecServicePtr svc, TecSignalPtr sig_stopped) {
    TecTCPServerPtr self = TecTCPServer_ptr(svc);
    atomic_store(&self->running, false);
    // Wait until polling stops.
    TecSignal_wait(&self->sig_polling_stopped);
    TecSocket_close(&self->sock);
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
    TecService_init(&self->service, server_params->hash_table_size);
    self->server_params = server_params;
    self->socket_params = socket_params;
    // Socket initilization.
    TecSocket_init_server(&self->sock, socket_params);
    // Empty buffer.
    TecBuffer_init(&self->buffer, 0, socket_params->buffer_size);
    // Polling
    TecSignal_init(&self->sig_polling_stopped);
    self->running = true;
    // Overrides.
    self->service.start = start_;
    self->service.shutdown = shutdown_;
    self->service.done = TecTCPServer_done_;
    // TecTCPServer API
    self->poll = poll_;
}


TECC_IMPL void TecTCPServer_done_(TecServicePtr svc) {
    TecTCPServerPtr self = TecTCPServer_ptr(svc);
    TecSignal_done(&self->sig_polling_stopped);
    TecBuffer_done(&self->buffer);
    TecSocket_done(&self->sock);
    TecService_done_(&self->service);
}
