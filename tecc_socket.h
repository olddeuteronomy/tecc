// Time-stamp: <Last changed 2026-04-22 13:13:53 by magnolia>
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

#ifndef TECC_SOCKET_H
#define TECC_SOCKET_H

/*

Default parameters defined in tecc_socket.c
-------------------------------------------

// IPv4 address to bind/accept connections from any interface.
static const char kAnyAddr[] = "0.0.0.0";

// IPv4 loopback address (localhost).
static const char kLocalAddr[] = "127.0.0.1";

// Hostname that resolves to localhost for both IPv4 and IPv6.
static const char kLocalURI[] = "localhost";

// IPv6 address to bind/accept connections from any interface.
static const char kAnyAddrIP6[] = "::";

// IPv6 loopback address (localhost).
static const char kLocalAddrIP6[] = "::1";

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

Server options
--------------

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

*/

#include "tecc/tecc_def.h"
#include "tecc/tecc_buffer.h"


// 255 bytes is the DNS packet limit.
#define TECC_MAX_URI_LEN 256

// Defalut socket buffer size.
#define TECC_SOCKET_BUFFER_SIZE 1024

// End of file.
#define TECC_EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagTecSocketParams TecSocketParams;
typedef TecSocketParams* TecSocketParamsPtr;

typedef struct tagTecSocketParams {
    char addr[TECC_MAX_URI_LEN];
    int port;           // Port number to connect to or bind.
    int family;         // Address family (AF_INET, AF_INET6, AF_UNSPEC, ...).
    int socktype;       // Socket type (SOCK_STREAM, SOCK_DGRAM, ...).
    int protocol;       // Protocol (usually 0).
    int client_flags;   // Flags passed to getaddrinfo() for client.
    int server_flags;   // Flags passed to getaddrinfo() for server.
    size_t buffer_size; // Size of internal buffer for read/write operations.
    // Server parameters.
    int mode;                // Data handling mode (character stream or binary network data).
    int queue_size;          // Maximum backlog for listen().
    int opt_reuse_addr;      // Whether to set SO_REUSEADDR (0 = no, 1 = yes).
    int opt_reuse_port;      // Whether to set SO_REUSEPORT (if available).
    size_t thread_pool_size; // Number of threads in the thread pool. 0 - no thread pool.
} TecSocketParams;

// Initialize with default values.
TECC_API void TecSocketParams_init(TecSocketParamsPtr self);

// Destructor. Does nothing by default.
TECC_API void TecSocketParams_done(TecSocketParamsPtr self);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                        TecSocket object
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


typedef struct tagTecSocket TecSocket;
typedef TecSocket* TecSocketPtr;

typedef struct tagTecSocket {
    int fd;
    int flags;
    TecBuffer buf;
    TecSocketParamsPtr params;
} TecSocket;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                       TecSocket API
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define TecSocket_ptr(ptr) ((TecSocketPtr)(ptr))

// Initialize the socket.
#define TecSocket_init(sock) TecSocket_init_(TecSocket_ptr(sock))
TECC_API void TecSocket_init_(TecSocketPtr sock);

// Destructor.
#define TecSocket_done(sock) TecSocket_done_(TecSocket_ptr(sock))
TECC_API void TecSocket_done_(TecSocketPtr sock);

// If `len' is 0, reads null-terminated string. Returns 0 on success or codes from <errno.h>
TECC_API int TecSocket_read(TecSocketPtr sock, TecBufferPtr dst, size_t len);

// Writes `src' buffer to the socket. Returns 0 on success or codes from <errno.h>
TECC_API int TecSocket_write(TecSocketPtr sock, TecBufferPtr src);

#ifdef __cplusplus
}
#endif

#endif // TECC_SOCKET_H
