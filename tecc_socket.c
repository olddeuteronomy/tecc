// Time-stamp: <Last changed 2026-04-29 11:15:34 by magnolia>
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
#  define _GNU_SOURCE
#endif

#ifndef __linux
#  ifndef _POSIX_C_SOURCE
     // This line fixes the "storage size of 'hints' isn't known" issue.
#    define _POSIX_C_SOURCE 200809L
#  endif
#endif

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "tecc/tecc_def.h"   // IWYU pragma: keep
#include "tecc/tecc_trace.h" // IWYU pragma: keep
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_socket.h"


// IPv4 address to bind/accept connections from any interface.
TECC_unused static const char kTecAnyAddr_[] = "0.0.0.0";
TECC_unused const char* const kTecAnyAddr = kTecAnyAddr_;
// IPv4 loopback address ["127.0.0.1"].
TECC_unused static const char kTecLocalAddr_[] = "127.0.0.1";
TECC_unused const char* const kTecLocalAddr = kTecLocalAddr_;
// ["localhost"].
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
    self->addr = kTecLocalAddr;
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


// Destructor. Does nothing by default.
TECC_IMPL void TecSocketParams_done(TecSocketParamsPtr self) {
    (void)self;
}

/*======================================================================
*
*                       TecSocket API
*
 *====================================================================*/

TECC_IMPL void TecSocket_init_(TecSocketPtr sock, TecSocketParamsPtr params) {
    sock->fd = -1;
    sock->port = -1;
    sock->addr[0] = 0;
    sock->flags = 0;
    sock->pai = NULL;
    sock->params = params;
    TecBuffer_init(&sock->buf, 0, params->buffer_size); // Empty buffer, no memory allocated.
}


TECC_IMPL void TecSocket_done_(TecSocketPtr sock) {
    TecSocket_close(sock);
    // NOTE: We do not deallocate the buffer!
}


// Uses `getpeername()` to obtain the actual peer socket info.
static int get_socket_info(TecSocketPtr sock) {
    static_assert(TECC_SOCK_ADDRLEN > INET6_ADDRSTRLEN, "Not enought space to store socket address");
    struct sockaddr_storage addr = {0};
    socklen_t size = sizeof(struct sockaddr_storage);
    int err = getpeername(sock->fd, (struct sockaddr*)&addr, &size);
    sock->port = -1;
    if (err == -1) {
        return errno;
    }
    if (addr.ss_family == AF_INET) {
        // IPv4
        struct sockaddr_in *s = (struct sockaddr_in*)&addr;
        inet_ntop(AF_INET, &s->sin_addr, sock->addr, INET_ADDRSTRLEN);
        sock->port = ntohs(s->sin_port);
    }
    else if (addr.ss_family == AF_INET6) {
        // IPv6
        struct sockaddr_in6 *s = (struct sockaddr_in6*)&addr;
        inet_ntop(AF_INET6, &s->sin6_addr, sock->addr, INET6_ADDRSTRLEN);
        sock->port = ntohs(s->sin6_port);
    }
    else {
        err = EPROTONOSUPPORT;
    }
    return err;
}


// Resolves peer address. On success, returns 0 and sets socket FD.
TECC_IMPL int TecSocket_open(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::open()");
    if (sock->params->socktype != SOCK_STREAM) {
        TECC_TRACE_EXIT();
        return EPROTONOSUPPORT;
    }
    int fd = -1;
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
        if (fd == -1) {
            err = errno;
            TECC_TRACE("!!! (%d) Error opening socket on %s:%d.\n", err, sock->params->addr, sock->params->port);
        }
        else {
            // OK
            sock->fd = fd;
            sock->pai = res;
            TECC_TRACE("Socket opened on %s:%d OK.\n", sock->params->addr, sock->params->port);
        }
    }
    TECC_TRACE_EXIT();
    return err;
}


TECC_IMPL int TecSocket_connect(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::connect()");
    // Connect to the socket.
    if (sock->fd == -1 || sock->pai == NULL) {
        TECC_TRACE_EXIT();
        return EBADF;
    }
    int err = 0;
    if (connect(sock->fd, sock->pai->ai_addr, sock->pai->ai_addrlen) == -1) {
        err = ECONNREFUSED;
    }
    // Check error.
    if (err) {
        TECC_TRACE("!!! (%d) Failed to connect to %s:%d.\n", err, sock->params->addr, sock->params->port);
        TecSocket_close(sock);
    }
    else {
        // OK, let's obtain peer information.
        err = get_socket_info(sock);
        if (err) {
            TECC_TRACE("??? (%d) Cannot obtain peer info.\n", err);
        }
        else {
            TECC_TRACE("Connected to %s:%d OK.\n", sock->addr, sock->port);
        }
    }
    TECC_TRACE_EXIT();
    return err;
}


// Set default socket options.
TECC_IMPL int TecSocket_set_options(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::set_options()");
    int err = 0;
    int res = 0;
    TECC_TRACE("`flags` is set to 0x%04x.\n", sock->params->flags);
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
        TECC_TRACE("!!! (%d) Listening on %s:%d failed.\n", err, sock->params->addr, sock->params->port);
    }
    else {
        // OK.
        TECC_TRACE("Listening on %s:%d OK.\n", sock->params->addr, sock->params->port);
    }
    TECC_TRACE_EXIT();
    return err;
}


// Server: Accept one incoming connection (blocking).
// Returns a new client socket.
TECC_IMPL TecSocket TecSocket_accept(TecSocketPtr sock) {
    TECC_TRACE_ENTER("Socket::accept()");
    TecSocket cli = {0};
    // Initialize the client with server's parameters.
    TecSocket_init(&cli, sock->params);
    int err = 0;
    // Wait for incomming connection.
    cli.fd = accept(sock->fd, NULL, NULL);
    // Check result.
    if (cli.fd == -1) {
        err = errno;
        if (err == EINVAL || err == EINTR || err == EBADF) {
            TECC_TRACE("Polling interrupted by signal %d.\n", err);
        }
        else {
            TECC_TRACE("!!! Failed with errno=%d.\n", err);
        }
    }
    else {
        // Get peer address and port number.
        err = get_socket_info(&cli);
        if (err) {
            TECC_TRACE("!!! (%d) Failed to obtain the peer name.\n", err);
        }
        else {
            // By default, use the host buffer.
            cli.buf = sock->buf;
            TECC_TRACE("Connection from %s:%d OK.\n", cli.addr, cli.port);
        }
    }
    TECC_TRACE_EXIT();
    return cli;
}


// Close socket.
TECC_IMPL void TecSocket_close(TecSocketPtr sock) {
    if (sock->fd != TECC_EOF) {
        TECC_TRACE_ENTER("Socket::close()");
        shutdown(sock->fd, SHUT_RDWR);
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
    size_t buffer_size = sock->buf.capacity;
    //
    // Read data from the socket.
    //
    do {
        received = read(sock->fd, buffer, buffer_size);
        if (len == 0 && received > 0) {
            // Length is unknown -- check for null-terminated string.
            if (buffer[received-1] == '\0') {
                TECC_TRACE("%s:%d EOT received.\n", sock->addr, sock->port);
                eot = true;
            }
        }
        if (received > 0) {
            TecBuffer_write(dst, buffer, received);
            TECC_TRACE("%s:%d --> RECV %zd bytes.\n", sock->addr, sock->port, received);
            total_received += received;
            if (len > 0 && len == total_received) {
                break;
            }
        }
        if (eot || received < (ssize_t)buffer_size) {
            break;
        }
    } while (received);
    //
    // Check for errors.
    //
    int err = 0;
    if (len > 0  &&  total_received == len) {
        // OK, do nothing.
        (void)0;
    }
    else if (received == 0) {
        err = errno;
        TECC_TRACE("%s:%d (%d) Peer closed the connection.\n", sock->addr, sock->port, err);
    }
    else if (received < 0) {
        err = errno;
        TECC_TRACE("!!! %s:%d (%d) Read error.\n", sock->addr, sock->port, err);
    }
    else if (len > 0  &&  total_received != len) {
        err = EIO;
        TECC_TRACE("!!! %s:%d (%d) Partial read: %zd bytes of %zu.\n",
                   sock->addr, sock->port, err,
                   total_received, len);
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
        sent = write(sock->fd, src->data, src->size);
        //
        // Check for errors.
        //
        if (sent < 0) {
            err = errno;
            TECC_TRACE("!!! %s:%d (%d) Write error.\n", sock->addr, sock->port, err);
        }
        else if (src->size != (size_t)sent) {
            err = EIO;
            TECC_TRACE("!!! %s:%d (%d) Partial write: %zd bytes of %zu.\n",
                       sock->addr, sock->port, err,
                       sent, src->size);
        }
    }
    TECC_TRACE("%s:%d <-- SENT %zd bytes.\n", sock->addr, sock->port, sent);
    TECC_TRACE_EXIT();
    return err;
}


// Writes the null-terminated string to the SOCK_STREAM socket.
// Returns 0 on success or an error code from <errno.h>
TECC_IMPL int TecSocket_write_str(TecSocketPtr sock, char* s) {
    if (s == NULL) {
        return 0;
    }
    int len = strlen(s) + 1; // Include terminating 0.
    // Temp static buffer.
    TecBuffer buf = {.data=s, .size=len};
    return TecSocket_write(sock, &buf);
}
