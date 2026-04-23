// Time-stamp: <Last changed 2026-04-23 02:35:34 by magnolia>
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
#ifndef _POSIX_C_SOURCE
// This line fixes the "storage size of 'hints' isn't known" issue.
#define _POSIX_C_SOURCE 200809L
#endif

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "tecc/tecc_def.h"   // IWYU pragma: keep
#include "tecc/tecc_trace.h" // IWYU pragma: keep
#include "tecc/tecc_socket.h"


// IPv4 address to bind/accept connections from any interface.
TECC_unused static const char kAnyAddr[] = "0.0.0.0";

// IPv4 loopback address (localhost).
TECC_unused static const char kLocalAddr[] = "127.0.0.1";

// Hostname that resolves to localhost for both IPv4 and IPv6.
static const char kLocalURI[] = "localhost";

// IPv6 address to bind/accept connections from any interface.
TECC_unused static const char kAnyAddrIP6[] = "::";

// IPv6 loopback address (localhost).
TECC_unused static const char kLocalAddrIP6[] = "::1";

// Default port number used for testing and examples.
static const int kDefaultPort = 4321;

// Default address family: AF_UNSPEC allows both IPv4 and IPv6.
static const int kDefaultFamily = AF_UNSPEC;

// Default socket type: TCP stream socket.
static const int kDefaultSockType = SOCK_STREAM;

// Default protocol: 0 for "any appropriate protocol".
static const int kDefaultProtocol = 0;

// Default addrinfo flags for client sockets (no special behaviour).
static const int kDefaultClientFlags = 0;

// Default addrinfo flags for server sockets (bind to local address).
static const int kDefaultServerFlags = AI_PASSIVE;

// Disable SO_REUSEADDR option.
static const int kDefaultOptReuseAddress = 0;

// Enable SO_REUSEADDR and SO_REUSEPORT (if supported).
static const int kDefaultOptReusePort = 1;

// Default number of threads in the thread pool. Initially disabled.
static const int kDefaultNumThreads = 0;

// Maximum queue length specifiable by `listen()', usually 4096.
static const int kDefaultConnQueueSize = SOMAXCONN;


// Initialize with default values.
TECC_IMPL void TecSocketParams_init(TecSocketParamsPtr self) {
    strncpy(self->addr, kLocalURI, TECC_MAX_URI_LEN-1);
    self->addr[TECC_MAX_URI_LEN-1] = 0;
    self->port = kDefaultPort;
    self->family = kDefaultFamily;
    self->socktype = kDefaultSockType;
    self->protocol = kDefaultProtocol;
    self->client_flags = kDefaultClientFlags;
    self->server_flags = kDefaultServerFlags;
    self->buffer_size = TECC_SOCKET_BUFFER_SIZE;
    // Server parameters.
    self->mode = 0;
    self->queue_size = kDefaultConnQueueSize;
    self->opt_reuse_addr = kDefaultOptReuseAddress;
    self->opt_reuse_port = kDefaultOptReusePort;
    self->thread_pool_size = kDefaultNumThreads;
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
    sock->params = params;
    TecBuffer_init(&sock->buf); // Empty buffer, no memory allocated.
}


TECC_IMPL void TecSocket_done_(TecSocketPtr sock) {
    sock->fd = TECC_EOF;
    // We do not destroy the buffer!
}


static int resolve_address(TecSocketPtr sock, struct addrinfo** ai) {
    TECC_TRACE_ENTER("Socket::resolve_address()");
    int err = 0;
    TECC_TRACE("Resolving address %s:%d...\n",
               sock->params->addr, sock->params->port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = sock->params->family;
    hints.ai_socktype = sock->params->socktype;
    hints.ai_protocol = sock->params->protocol;
    // getaddrinfo() returns a list of address structures.
    char port_str[16];
    snprintf(port_str, 15, "%d", sock->params->port);
    err = getaddrinfo(sock->params->addr, port_str,
                      &hints, ai);
    if( err ) {
        TECC_TRACE("(%d) Error resolving address.\n", err);
    }
    else {
        TECC_TRACE("Address resolved OK.\n");
    }
    TECC_TRACE_EXIT();
    return err;
}

static int get_socket_fd(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::get_socket_fd()");
    int fd = TECC_EOF;
    struct addrinfo* servinfo = NULL;
    struct addrinfo* p = NULL;
    // Resolve address.
    int err = resolve_address(sock, &servinfo);
    if (!err) {
        TECC_TRACE("Connecting...\n");
        // If socket() (or connect()) fails, we close the socket
        // and try the next address.
        for (p = servinfo; p != NULL; p = p->ai_next) {
            fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (fd == TECC_EOF) {
                // Try next socket.
                continue;
            }
            if (connect(fd, p->ai_addr, p->ai_addrlen) != TECC_EOF) {
                // Success.
                break;
            }
            // Try next socket.
            close(fd);
        }
        // No longer needed.
        freeaddrinfo(servinfo);
    }
    if (p == NULL) {
        TECC_TRACE("Failed to connect to %s:%d.\n",
                   sock->params->addr, sock->params->port);
    }
    else {
        TECC_TRACE("Connected OK.\n");
    }
    TECC_TRACE_EXIT();
    return fd;
}


// On success, returns 0 and sets socket FD.
TECC_IMPL int TecSocket_connect(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::connect()");
    int fd = get_socket_fd(sock);
    int err = (fd == TECC_EOF) ? ECONNREFUSED : 0;
    if (!err) {
        sock->fd = fd;
    }
    TECC_TRACE_EXIT();
    return err;
}


// Close socket.
TECC_IMPL void TecSocket_close(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::close()");
    if (sock->fd != TECC_EOF) {
        shutdown(sock->fd, SHUT_RDWR);
        close(sock->fd);
        sock->fd = EOF;
    }
    TECC_TRACE_EXIT();
}


// If `len' is 0, reads null-terminated string. Returns 0 on success.
TECC_IMPL int TecSocket_read(TecSocketPtr sock, TecBufferPtr dst, size_t len) {
    TECC_TRACE_ENTER("Socket_read");
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
        TECC_TRACE("%s:%d (%d) Read error.\n",
                   sock->params->addr, sock->params->port, err);
    }
    else if (len > 0  &&  total_received != len) {
        err = EIO;
        TECC_TRACE("%s:%d (%d) Partial read: %z bytes of %z.\n",
                   sock->params->addr, sock->params->port, err, total_received, len);
    }
    TECC_TRACE_EXIT();
    return err;
}


// Writes `len' bytes to the socket. Returns 0 on success.
TECC_IMPL int TecSocket_write(TecSocketPtr sock, TecBufferPtr src) {
    TECC_TRACE_ENTER("Socket_write");
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
            TECC_TRACE("%s:%d (%d) Write error.\n",
                       sock->params->addr, sock->params->port, err);
        }
        else if (src->size != (size_t)sent) {
            err = EIO;
            TECC_TRACE("%s:%d (%d )Partial write: %z bytes of %z.\n",
                       sock->params->addr, sock->params->port, err, sent, src->size);
        }
    }
    TECC_TRACE("%s:%d <-- SEND %ld bytes.\n",
               sock->params->addr, sock->params->port, sent);
    TECC_TRACE_EXIT();
    return err;
}
