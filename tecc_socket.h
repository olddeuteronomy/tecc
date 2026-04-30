// Time-stamp: <Last changed 2026-04-30 12:06:27 by magnolia>
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

#include "tecc/tecc_def.h"
#include "tecc/tecc_buffer.h"

// 255 bytes is the DNS packet limit.
#define TECC_MAX_URI_LEN 256

// Default socket buffer size.
#define TECC_SOCK_BUFFER_SIZE 1024

// Socket address info. Should be enough to store IPv6 address.
#define TECC_SOCK_ADDRLEN 56

// End of stream.
#define TECC_EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
*
*               Default socket parameters and constants
*
 *====================================================================*/

// Server: IPv4 address to bind/accept connections from IPv4 address ["0.0.0.0"].
extern const char* const kTecAnyAddr;
// Client: IPv4 loopback address ["127.0.0.1"].
extern const char* const kTecLocalAddr;
// ["localhost"].
extern const char* const kTecLocalHost;
// Server: IPv6 address to bind/accept connections from any IPv6 or IPv4 address ["::"].
extern const char* const kTecAnyAddrIP6;
// Client: IPv6 loopback address ["::1"].
extern const char* const kTecLocalAddrIP6;
// Default port number used for testing and examples [4321].
extern const int kTecDefaultPort;
// Default address family: AF_UNSPEC allows both IPv4 and IPv6 [AF_UNSPEC].
extern const int kTecDefaultFamily;
// Default socket type: TCP stream socket [SOCK_STREAM].
extern const int kTecDefaultSockType;
// Default protocol: any appropriate protocol [0].
extern const int kTecDefaultProtocol;

/* Client parameters */

// Default flags for client sockets [0].
extern const int kTecDefaultClientFlags;

/* Server parameters */

// Default flags for server sockets [AI_PASSIVE].
extern const int kTecDefaultServerFlags;
// SO_REUSEADDR option initially disabled [0].
extern const int kTecDefaultOptReuseAddress;
// SO_REUSEPORT option initially enabled [1].
extern const int kTecDefaultOptReusePort;
// Maximum queue length specifiable by `listen()`, usually 4096.
extern const int kTecDefaultConnQueueSize;

/*======================================================================
*
*                   TecSocketParameters
*
 *====================================================================*/

typedef struct tagTecSocketParams TecSocketParams;
typedef TecSocketParams* TecSocketParamsPtr;

// 56 bytes.
typedef struct tagTecSocketParams {
    // Common parameters.
    const char* addr;       // Host address [127.0.0.1].
    int port;               // Port number to connect to or bind.
    int family;             // Address family [AF_UNSPEC].
    int socktype;           // Socket type [SOCK_STREAM].
    int protocol;           // Protocol (usually 0, any appropriate).
    int flags;              // AI_PASSIVE for servers, 0 for clients.
    size_t buffer_size;     // Size of internal buffer for send/recv operations.
    // Server parameters.
    int queue_size;         // Maximum backlog for `listen()` [4096].
    int opt_reuse_addr;     // Socket option SO_REUSEADDR [0]
    int opt_reuse_port;     // Socket option SO_REUSEPORT [1]
} TecSocketParams;

#define TecSocketParams_ptr(ptr) ((TecSocketParamsPtr)(ptr))

// Initialize with default values.
TECC_API void TecSocketParams_init(TecSocketParamsPtr self);

// Destructor. Does nothing by default.
TECC_API void TecSocketParams_done(TecSocketParamsPtr self);

/*======================================================================
*
*                        TecSocket object
*
 *====================================================================*/

// Forward reference. We hide implementation details.
struct addrinfo;

typedef struct tagTecSocket TecSocket;
typedef TecSocket* TecSocketPtr;

// 128 bytes.
typedef struct tagTecSocket {
    int fd;                       // Socket FD.
    int port;                     // Port number.
    char addr[TECC_SOCK_ADDRLEN]; // Socket address.
    int flags;                    // Socket flags.
    struct addrinfo* pai;         // Internal host addrinfo.
    TecBuffer buf;                // Internal buffer for read/write operations.
    TecSocketParamsPtr params;    // Host parameters.
} TecSocket;

/*======================================================================
*
*                       TecSocket API
*
 *====================================================================*/

#define TecSocket_ptr(ptr) ((TecSocketPtr)(ptr))

// Initialize the socket -- client version.
#define TecSocket_init(sock, params)\
    TecSocket_init_(TecSocket_ptr(sock), TecSocketParams_ptr(params))
TECC_API void TecSocket_init_(TecSocketPtr, TecSocketParamsPtr);

// Initialize the socket -- server version.
#define TecSocket_init_server(sock, params) do {\
    TecSocketParams_ptr(params)->flags |= kTecDefaultServerFlags;\
    TecSocket_init(sock, params);\
    } while (0)

// Destructor.
#define TecSocket_done(sock) TecSocket_done_(TecSocket_ptr(sock))
TECC_API void TecSocket_done_(TecSocketPtr sock);

// Resolves peer address and opens the socket. On success, returns 0 and sets socket FD.
// Currently, only SOCK_STREAM sockets (TCP) are supported.
TECC_API int TecSocket_open(TecSocketPtr);

// Sets socket options.
TECC_API int TecSocket_set_options(TecSocketPtr);

// Client: connect to the host.
// Currently, only SOCK_STREAM sockets (TCP) are supported.
TECC_API int TecSocket_connect(TecSocketPtr);

#define TecSocket_is_server(ptr) ((TecSocket_ptr(ptr)->params->flags) & kTecDefaultServerFlags)

#define TecSocket_is_valid(ptr) ((TecSocket_ptr(ptr)->fd) != -1)

// Server: bind a name to the listening socket.
TECC_API int TecSocket_bind(TecSocketPtr);

// Server: listen for connections on the socket.
TECC_API int TecSocket_listen(TecSocketPtr);

// Server: accept an incoming connection (blocking).
// Returns a new client socket, possible with fd=-1.
TECC_IMPL TecSocket TecSocket_accept(TecSocketPtr sock);

// Closes the socket.
TECC_API void TecSocket_close(TecSocketPtr);

// Reads `len` bytes from the SOCK_STREAM socket into the `dst` buffer.
// If `len` is 0, reads a null-terminated string.
// Returns 0 on success or an error code from <errno.h> on failure.
TECC_API int TecSocket_read(TecSocketPtr sock, TecBufferPtr dst, size_t len);

// Writes the `src` buffer to the SOCK_STREAM socket.
// Returns 0 on success or an error code from <errno.h> on failure.
TECC_API int TecSocket_write(TecSocketPtr sock, TecBufferPtr src);

// Writes the null-terminated string to the SOCK_STREAM socket.
// Returns 0 on success or an error code from <errno.h> on failure.
TECC_API int TecSocket_write_str(TecSocketPtr sock, char* s);

#ifdef __cplusplus
}
#endif

#endif // TECC_SOCKET_H
