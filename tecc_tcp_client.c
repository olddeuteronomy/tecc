// Time-stamp: <Last changed 2026-05-05 02:20:53 by magnolia>
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
#include "tecc/tecc_signal.h"
#include "tecc/tecc_socket.h"
#include "tecc/tecc_trace.h" // IWYU pragma: keep
#include "tecc/tecc_tcp_client.h"


/*======================================================================
*
*                      TecService overrides
*
 *====================================================================*/

static void start_(TecServicePtr svc, TecSignalPtr sig_started, int* error) {
    TecTCPClientPtr self = TecTCPClient_ptr(svc);
    int err = TecSocket_open(&self->sock);
    if (!err) {
        err = TecSocket_connect(&self->sock);
    }
    if (!err) {
        // Allocate the buffer.
        TecBuffer_init(&self->buffer,
                       self->socket_params->buffer_size,
                       self->socket_params->buffer_size);
    }
    *error = err;
    TecSignal_set(sig_started);
}


static void shutdown_(TecServicePtr svc, TecSignalPtr sig_stopped) {
    TecTCPClientPtr self = TecTCPClient_ptr(svc);
    TecSocket_close(&self->sock);
    TecSignal_set(sig_stopped);
}

/*======================================================================
*
*                        TecTCPClient API
*
 *====================================================================*/

static int send_(TecTCPClientPtr self, TecBufferPtr data) {
    TecMutex_lock(&self->guard);
    int error = TecSocket_write(&self->sock, data);
    TecMutex_unlock(&self->guard);
    return error;
}

static int send_str_(TecTCPClientPtr self, char* str) {
    TecMutex_lock(&self->guard);
    int error = TecSocket_write_str(&self->sock, str);
    TecMutex_unlock(&self->guard);
    return error;
}


static int recv_(TecTCPClientPtr self, TecBufferPtr data) {
    TecMutex_lock(&self->guard);
    int error = TecSocket_read(&self->sock, data);
    TecMutex_unlock(&self->guard);
    return error;
}

static int send_recv_(TecTCPClientPtr self, TecBufferPtr data_in, TecBufferPtr data_out) {
    TecMutex_lock(&self->guard);
    int error = TecSocket_write(&self->sock, data_in);
    if (!error) {
        error = TecSocket_read(&self->sock, data_out);
    }
    TecMutex_unlock(&self->guard);
    return error;
}

static int send_recv_str_(TecTCPClientPtr self, char* str, TecBufferPtr data_out) {
    TecMutex_lock(&self->guard);
    int error = TecSocket_write_str(&self->sock, str);
    if (!error) {
        error = TecSocket_read(&self->sock, data_out);
    }
    TecMutex_unlock(&self->guard);
    return error;
}


TECC_IMPL void TecTCPClient_init_(TecTCPClientPtr self, TecSocketParamsPtr socket_params) {
    TecService_init(&self->service);
    self->socket_params = socket_params;
    TecSocket_init(&self->sock, socket_params);
    // Empty buffer.
    TecBuffer_init(&self->buffer, 0, socket_params->buffer_size);
    // Guard.
    TecMutex_init(&self->guard);
    // Overrides.
    self->service.start = start_;
    self->service.shutdown = shutdown_;
    self->service.done = TecTCPClient_done_;
    // TCPClient API.
    self->send = send_;
    self->send_str = send_str_;
    self->recv = recv_;
    self->send_recv = send_recv_;
    self->send_recv_str = send_recv_str_;
}

// Destructor.
TECC_IMPL void TecTCPClient_done_(TecServicePtr svc) {
    TecTCPClientPtr self = TecTCPClient_ptr(svc);
    TecMutex_destroy(&self->guard);
    TecBuffer_done(&self->buffer);
    TecSocket_done(&self->sock);
    TecService_done_(&self->service);
}
