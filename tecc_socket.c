// Time-stamp: <Last changed 2026-04-27 15:49:07 by magnolia>
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
#include <arpa/inet.h>
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
    // 127.0.0.1 by default.
    strncpy(self->addr, kTecLocalAddr, TECC_MAX_URI_LEN-1);
    self->addr[TECC_MAX_URI_LEN-1] = 0;
    self->port = kTecDefaultPort;
    self->family = kTecDefaultFamily;
    self->socktype = kTecDefaultSockType;
    self->protocol = kTecDefaultProtocol;
    self->flags = kTecDefaultClientFlags;
    self->buffer_size = TECC_SOCK_BUFFER_SIZE;
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
    TecBuffer_init(&sock->buf, 0, params->buffer_size); // Empty buffer, no memory allocated.
    sock->in.port = -1;
    sock->in.addr[0] = 0;
}


TECC_IMPL void TecSocket_done_(TecSocketPtr sock) {
    TecSocket_close(sock);
    // TODO: We do not destroy the buffer!
    TecBuffer_done(&sock->buf);
}


// Resolves peer address. On success, returns 0 and sets socket FD.
TECC_IMPL int TecSocket_open(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::open()");
    if (sock->params->socktype != SOCK_STREAM) {
        TECC_TRACE_EXIT();
        return ENOTSOCK;
    }
    int fd = TECC_EOF;
    struct addrinfo hints = {
        .ai_family = sock->params->family,
        .ai_socktype = sock->params->socktype,
        .ai_flags = sock->params->flags // AI_PASSIVE for servers.
    };
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
        TECC_TRACE("Address %s:%d resolved OK.\n", sock->params->addr, sock->params->port);
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
            TECC_TRACE("Socket FD=%d opened OK.\n", sock->fd);
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
    TECC_TRACE_ENTER("Socket::set_options()");
    int err = 0;
    int res = 0;
    TECC_TRACE("FD=%d: `flags` is set to 0x%04x.\n", sock->fd, sock->params->flags);
    if (TecSocket_is_valid(sock) && TecSocket_is_server(sock)) {
        // SO_REUSEADDR
        res = setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR,
                   &sock->params->opt_reuse_addr, sizeof(int));
        if (res < 0) {
            err = errno;
            TECC_TRACE("SO_REUSEADDR failed with err=%d.\n", err);
        }
        else {
            TECC_TRACE("SO_REUSEADDR set to %d.\n", sock->params->opt_reuse_addr);
        }
        // SO_REUSEPORT
        res = setsockopt(sock->fd, SOL_SOCKET, SO_REUSEPORT,
                       &sock->params->opt_reuse_port, sizeof(int));
        if (res < 0) {
            err = errno;
            TECC_TRACE("SO_REUSEPORT failed with err=%d.\n", err);
        }
        else {
            TECC_TRACE("SO_REUSEPORT set to %d.\n", sock->params->opt_reuse_port);
        }
    }
    TECC_TRACE_EXIT();
    return err;
}


// Server: bind a name to the listening socket.
TECC_IMPL int TecSocket_bind(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::bind()");
    int err = 0;
    if (TecSocket_is_valid(sock) && sock->pai) {
        if (bind(sock->fd, sock->pai->ai_addr, sock->pai->ai_addrlen) == -1) {
            err = errno;
        }
    }
    else {
        err = EBADF;
    }
    if (err) {
        TECC_TRACE("!!! (%d) Binding to %s:%d failed.\n",
                   err, sock->params->addr, sock->params->port);
    }
    else {
        TECC_TRACE("Bounded to %s:%d OK.\n",
                   sock->params->addr, sock->params->port);
    }
    TECC_TRACE_EXIT();
    return err;
}


// Server: listen for connections on the socket.
TECC_IMPL int TecSocket_listen(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::listen()");
    int err = 0;
    if (TecSocket_is_valid(sock)) {
        if (listen(sock->fd, sock->params->queue_size) == -1) {
            err = errno;
        }
    }
    else {
        err = EBADF;
    }
    if (err) {
        TECC_TRACE("!!! (%d) Listening on %s:%d failed.\n",
                   err, sock->params->addr, sock->params->port);
    }
    else {
        // Allocate server's buffer.
        TecBuffer_init(&sock->buf, sock->params->buffer_size, sock->params->buffer_size);
        TECC_TRACE("Listening on %s:%d OK.\n",
                   sock->params->addr, sock->params->port);
    }
    TECC_TRACE_EXIT();
    return err;
}


static void get_socket_info(TecSocketPtr sock, struct sockaddr_storage* client_addr) {
    sock->in.port = -1;
    if (client_addr->ss_family == AF_INET) {
        // IPv4
        struct sockaddr_in *s = (struct sockaddr_in*)&client_addr;
        inet_ntop(AF_INET, &s->sin_addr, sock->in.addr, TECC_SOCK_ADDRLEN);
        sock->in.port = ntohs(s->sin_port);
    }
    else if (client_addr->ss_family == AF_INET6) {
        // IPv6
        struct sockaddr_in6 *s = (struct sockaddr_in6*)&client_addr;
        inet_ntop(AF_INET6, &s->sin6_addr, sock->in.addr, TECC_SOCK_ADDRLEN);
        sock->in.port = ntohs(s->sin6_port);
    }
}


// Server: Accept one incoming connection (blocking).
// Returns a new client socket.
TECC_IMPL TecSocket TecSocket_accept(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::accept()");
    TecSocket cli = {0};
    // Initialize the client with server's parameters.
    TecSocket_init(&cli, sock->params);
    struct sockaddr_storage cli_addr = {0};
    socklen_t sin_size = sizeof(struct sockaddr_storage);
    int err = 0;
    cli.fd = accept(sock->fd, (struct sockaddr *)&cli_addr, &sin_size);
    // Check result.
    if (cli.fd == TECC_EOF) {
        err = errno;
        if (err == EINVAL || err == EINTR || err == EBADF) {
            TECC_TRACE("Polling interrupted by signal %d.\n", err);
        }
        else {
            TECC_TRACE("!!! Failed with errno=%d.\n", err);
        }
    }
    else {
        // Get incoming address and port number.
        get_socket_info(&cli, &cli_addr);
        TECC_TRACE("Connection from %s:%d OK.\n", cli.in.addr, cli.in.port);
    }

    TECC_TRACE_EXIT();
    return cli;
}


// Close socket.
TECC_IMPL void TecSocket_close(TecSocketPtr sock) {
    if (sock->fd != TECC_EOF) {
        TECC_TRACE_ENTER("Socket::close()");
        /* shutdown(sock->fd, SHUT_RDWR); */
        close(sock->fd);
        sock->fd = EOF;
        if (sock->pai) {
            freeaddrinfo(sock->pai);
            sock->pai = NULL;
        }
        TECC_TRACE_EXIT();
    }
}


// If `len' is 0, reads null-terminated string. Returns 0 on success.
TECC_IMPL int TecSocket_read(TecSocketPtr sock, TecBufferPtr dst, size_t len) {
    TECC_TRACE_ENTER("Socket::read()");
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
    TECC_TRACE_ENTER("Socket::write()");
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
    TECC_TRACE("%s:%d <-- SENT %ld bytes.\n",
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
