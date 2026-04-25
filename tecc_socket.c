// Time-stamp: <Last changed 2026-04-25 16:14:27 by magnolia>
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
#ifdef __gnu_linux__
// This lines fixes the issue with SO_REUSEPORT on Linux.
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include <netdb.h>
#include <sys/socket.h>

#include "tecc/tecc_def.h"   // IWYU pragma: keep
#include "tecc/tecc_trace.h" // IWYU pragma: keep
#include "tecc/tecc_socket.h"


// IPv4 address to bind/accept connections from any interface.
TECC_unused static const char kTecAnyAddr_[] = "0.0.0.0";
TECC_unused const char* const kTecAnyAddr = kTecAnyAddr_;
// IPv4 loopback address ["127.0.0.1"].
TECC_unused static const char kTecLocalAddr_[] = "127.0.0.1";
TECC_unused const char* const kTecLocalAddr = kTecLocalAddr_;
// Hostname that resolves to localhost for both IPv4 and IPv6 ["localhost"].
TECC_unused static const char kTecLocalHost_[] = "localhost";
TECC_unused const char* const kTecLocalHost = kTecLocalHost_;
// IPv6 address to bind/accept connections from any interface ["::"].
TECC_unused static const char kTecAnyAddrIP6_[] = "::";
TECC_unused const char* const kTecAnyAddrIP6 = kTecAnyAddrIP6_;
// IPv6 loopback address ["::1"].
TECC_unused static const char kTecLocalAddrIP6_[] = "::1";
TECC_unused const char* const kTecLocalAddrIP6 = kTecLocalAddrIP6_;
// Default port number used for testing and examples [4321].
TECC_unused const int kTecDefaultPort = 4321;
// Default address family: AF_UNSPEC allows both IPv4 and IPv6 [AF_UNSPEC].
TECC_unused const int kTecDefaultFamily = AF_UNSPEC;
// Default socket type: TCP stream socket.
TECC_unused const int kTecDefaultSockType = SOCK_STREAM;
// Default protocol: 0 for any appropriate protocol [0].
TECC_unused const int kTecDefaultProtocol = 0;
// Default addrinfo flags for client sockets (no special behaviour).
TECC_unused const int kTecDefaultClientFlags = 0;
// Default addrinfo flags for server sockets.
TECC_unused const int kTecDefaultServerFlags = AI_PASSIVE;
// Disable SO_REUSEADDR option.
TECC_unused const int kTecDefaultOptReuseAddress = 0;
// Enable SO_REUSEADDR and SO_REUSEPORT (if supported).
TECC_unused const int kTecDefaultOptReusePort = 1;
// Maximum queue length specifiable by `listen()`, usually 4096.
TECC_unused const int kTecDefaultConnQueueSize = SOMAXCONN;


// Initialize with default values.
TECC_IMPL void TecSocketParams_init(TecSocketParamsPtr self) {
    strncpy(self->addr, kTecLocalHost, TECC_MAX_URI_LEN-1);
    self->addr[TECC_MAX_URI_LEN-1] = 0;
    self->port = kTecDefaultPort;
    self->family = kTecDefaultFamily;
    self->socktype = kTecDefaultSockType;
    self->protocol = kTecDefaultProtocol;
    self->flags = kTecDefaultClientFlags;
    self->buffer_size = TECC_SOCKET_BUFFER_SIZE;
    // Server parameters.
    self->queue_size = kTecDefaultConnQueueSize;
    self->opt_reuse_addr = kTecDefaultOptReuseAddress;
    self->opt_reuse_port = kTecDefaultOptReusePort;
}


TECC_IMPL void TecSocketParams_set_addr(TecSocketParamsPtr self, const char* addr) {
    strncpy(self->addr, addr, TECC_MAX_URI_LEN-1);
    self->addr[TECC_MAX_URI_LEN-1] = 0;
}


// Destructor. Does nothing by default.
TECC_IMPL void TecSocketParams_done(TecSocketParamsPtr self) {
    (void)self;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                       TecSocket API
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

TECC_IMPL void TecSocket_init_(TecSocketPtr sock, TecSocketParamsPtr params) {
    sock->fd = TECC_EOF;
    sock->flags = 0;
    sock->pai = NULL;
    sock->params = params;
    TecBuffer_init(&sock->buf); // Empty buffer, no memory allocated.
}


TECC_IMPL void TecSocket_done_(TecSocketPtr sock) {
    TecSocket_close(sock);
    // We do not destroy the buffer!
}


// Resolves peer address. On success, returns 0 and sets socket FD.
TECC_IMPL int TecSocket_open(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::open()");
    if (sock->params->socktype != SOCK_STREAM) {
        TECC_TRACE_EXIT();
        return ENOTSOCK;
    }
    int fd = TECC_EOF;
    struct addrinfo hints = { .ai_socktype = SOCK_STREAM };
    struct addrinfo *res = NULL;
    int err = 0;
    char port_str[16];
    snprintf(port_str, 15, "%d", sock->params->port);
    // Resolve address
    err = getaddrinfo(sock->params->addr, port_str, &hints, &res);
    if( err ) {
        TECC_TRACE("!!! (%d) Error resolving address %s:%d.\n",
                       err, sock->params->addr, sock->params->port);
    }
    else {
        TECC_TRACE("Address resolved OK.\n");
        // Open the socket.
        fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fd == TECC_EOF) {
            err = errno;
            TECC_TRACE("!!! (%d) Error opening socket on %s:%d.\n",
                       err, sock->params->addr, sock->params->port);
        }
        else {
            sock->fd = fd;
            sock->pai = res;
            TECC_TRACE("Socket opened OK.\n");
        }
    }
    TECC_TRACE_EXIT();
    return err;
}


TECC_IMPL int TecSocket_connect(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::connect()");
    // Connect to the socket.
    if (sock->fd == TECC_EOF || sock->pai == NULL) {
        TECC_TRACE_EXIT();
        return EBADF;
    }
    int err = 0;
    if (connect(sock->fd, sock->pai->ai_addr, sock->pai->ai_addrlen) == TECC_EOF) {
        err = ECONNREFUSED;
    }
    // Check error.
    if (err) {
        TECC_TRACE("!!! (%d) Failed to connect to %s:%d.\n",
                   err, sock->params->addr, sock->params->port);
        TecSocket_close(sock);
    }
    else {
        TECC_TRACE("Connected to %s:%d OK.\n",
                   sock->params->addr, sock->params->port);
    }
    TECC_TRACE_EXIT();
    return err;
}


// Set default server socket options.
TECC_IMPL int TecSocket_set_options(TecSocketPtr sock) {
    int err = 0;
    int res = 0;
    if (TecSocket_is_valid(sock) && TecSocket_is_server(sock)) {
        res = setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR,
                   &sock->params->opt_reuse_addr, sizeof(int));
        if (res < 0) {
            err = errno;
        }
        res = setsockopt(sock->fd, SOL_SOCKET, SO_REUSEPORT,
                       &sock->params->opt_reuse_port, sizeof(int));
        if (res < 0) {
            err = errno;
        }
    }
    return err;
}


// Close socket.
TECC_IMPL void TecSocket_close(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::close()");
    if (sock->fd != TECC_EOF) {
        shutdown(sock->fd, SHUT_RDWR);
        close(sock->fd);
        sock->fd = EOF;
        if (sock->pai) {
            freeaddrinfo(sock->pai);
            sock->pai = NULL;
        }
    }
    TECC_TRACE_EXIT();
}


// If `len' is 0, reads null-terminated string. Returns 0 on success.
TECC_IMPL int TecSocket_read(TecSocketPtr sock, TecBufferPtr dst, size_t len) {
    TECC_TRACE_ENTER("Socket::recv()");
    size_t total_received = 0;
    ssize_t received = 0;
    bool eot = false; // End of transfer.
    char* buffer = sock->buf.data;
    size_t buffer_size = sock->buf.size;
    //
    // Read data from the socket.
    //
    while ((received = recv(sock->fd, buffer, buffer_size, sock->flags)) > 0) {
        if (len == 0 && received > 0) {
            // Length is unknown -- check for null-terminated string.
            if (buffer[received-1] == '\0') {
                TECC_TRACE("%s:%d EOT received.",
                           sock->params->addr, sock->params->port);
                eot = true;
            }
        }
        if (received > 0) {
            TecBuffer_write(dst, buffer, received);
            TECC_TRACE("%s:%d --> RECV %z bytes.",
                       sock->params->addr, sock->params->port, received);
            total_received += received;
            if (len > 0 && len == total_received) {
                break;
            }
        }
        if (eot || received < (ssize_t)buffer_size) {
            break;
        }
    }
    //
    // Check for errors.
    //
    int err = 0;
    if (len > 0  &&  total_received == len) {
        // OK, do nothing.
        (void)0;
    }
    else if (received == 0) {
        err = EIO;
        TECC_TRACE("%s:%d (%d) Peer closed the connection.\n",
                   sock->params->addr, sock->params->port, err);
    }
    else if (received < 0) {
        err = errno;
        TECC_TRACE("!!! %s:%d (%d) Read error.\n",
                   sock->params->addr, sock->params->port, err);
    }
    else if (len > 0  &&  total_received != len) {
        err = EIO;
        TECC_TRACE("!!! %s:%d (%d) Partial read: %z bytes of %z.\n",
                   sock->params->addr, sock->params->port, err, total_received, len);
    }
    TECC_TRACE_EXIT();
    return err;
}


// Writes the `src` buffer to the SOCK_STREAM socket.
// Returns 0 on success or an error code from <errno.h>
TECC_IMPL int TecSocket_write(TecSocketPtr sock, TecBufferPtr src) {
    TECC_TRACE_ENTER("Socket::send()");
    ssize_t sent = 0;
    int err = 0;
    //
    // Write data to the socket.
    //
    if (src->size) {
        sent = send(sock->fd, src->data, src->size, sock->flags);
        //
        // Check for errors.
        //
        if (sent < 0) {
            err = errno;
            TECC_TRACE("!!! %s:%d (%d) Write error.\n",
                       sock->params->addr, sock->params->port, err);
        }
        else if (src->size != (size_t)sent) {
            err = EIO;
            TECC_TRACE("!!! %s:%d (%d) Partial write: %z bytes of %z.\n",
                       sock->params->addr, sock->params->port, err, sent, src->size);
        }
    }
    TECC_TRACE("%s:%d <-- SEND %ld bytes.\n",
               sock->params->addr, sock->params->port, sent);
    TECC_TRACE_EXIT();
    return err;
}


// Writes the null-terminated string to the SOCK_STREAM socket.
// Returns 0 on success or an error code from <errno.h>
TECC_IMPL int TecSocket_write_str(TecSocketPtr sock, char* s) {
    if (s == NULL) {
        return 0;
    }
    int len = strlen(s);
    // Temp buffer.
    TecBuffer buf = {.data=s, .size=len+1};
    return TecSocket_write(sock, &buf);
}
