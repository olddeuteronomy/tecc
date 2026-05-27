
# TECC

The TECC library provides portable components for C11, C17, and C23,
designed for use in concurrent environments.

TECC can be configured to use either the POSIX `<pthread.h>` API
(default on Linux and macOS) or the standard C `<threads.h>` API (C11 and
later), selectable at compile time.

TECC uses good old Makefiles for building (no CMake or other build
systems required).

TECC has no external dependencies.

All provided examples were tested with Valgrind (`valgrind
--leak-check=full --show-leak-kinds=all`) on Ubuntu 24.04 (gcc 13.3.0, clang 18.1.3) and macOS
10.14 (clang 11.0.0) -- the latter chosen intentionally old to ensure backward
compatibility. No memory leaks detected.

## Building TECC

TECC can be built with `gcc` (default) or `clang`. From the TECC root directory, run
```sh
make -k
```
It creates the following directory structure under the TECC root:
<pre>
tecc
   +--lib
        +--debug
               +--gcc
               +--clang
        +--release
               +--gcc
               +--clang
</pre>
By default,
```sh
gcc -std=c17 -Wall -Wextra -Werror -MMD -MP -O0 -g
```
is used. It also uses the POSIX `<pthread.h>` API. The resulting static library (`libtecc.a`)
is placed in the corresponding directory (`lib/debug/gcc` in this case).

### Build Options
 * **`CC_STD`**=[c11 | c17 | c23] (default is c17).
 * **`REL`**=1 (release build, `-O2`, no debug info).
 * **`CLANG`**=1 (to use `clang` instead of `gcc`).
 * **`NO_PTHREAD`**=1 (to use ISO C `<threads.h>` API instead of `<pthread.h>`).
 * **`TRACE_ON`**=1 (to turn on detailed tracing output to `stdout`).

For example,
```sh
make -k CC_STD=c11 CLANG=1 REL=1 NO_PTHREAD=1 rebuild
```
compiles the *release* (`-O2`) library with `clang` using *C11* standard and
ISO C `<threads.h>`. Tracing is off. The resulting `libtecc.a` is
placed in `lib/release/clang`.

## Examples

The same build rules can be used to compile the examples in the
[`tecc/examples/`](https://github.com/olddeuteronomy/tecc/tree/main/examples)
directory. The resulting executables are placed in the `tecc/build/` directory.
Use
```sh
make -k [options] TRACE_ON=1 all
```
to build all examples in the corresponding directory.

In the example below, we *construct* a multi-threaded TCP server with a thread
pool for handling incoming connections and arena-based allocation of
sockets and I/O buffers, using TECC's *components* such as
[`TecSignal`](https://github.com/olddeuteronomy/tecc/blob/main/tecc_signal.h),
[`TecBuffer`](https://github.com/olddeuteronomy/tecc/blob/main/tecc_buffer.h),
[`TecSocket`](https://github.com/olddeuteronomy/tecc/blob/main/tecc_socket.h),
[`TecThrPool`](https://github.com/olddeuteronomy/tecc/blob/main/tecc_thread_pool.h),
[`TecServiceWorker`](https://github.com/olddeuteronomy/tecc/blob/main/tecc_service_worker.h),
and others.

```c
// tecc/examples/socket/mt_server.c

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "tecc/tecc_def.h"    // IWYU pragma: keep
#include "tecc/tecc_trace.h"  // IWYU pragma: keep
#include "tecc/tecc_socket.h"
#include "tecc/tecc_buffer.h"
#include "tecc/tecc_thread_pool.h"
#include "tecc/tecc_tcp_server.h"
#include "tecc/tecc_service_worker.h"


static TecSignal sig_quit;

// Handle Ctrl-C.
static void handle_sigint(int sig) {
    (void)sig;
    TecSignal_set(&sig_quit);
}

// Usage: mt_server [ADDR] [PORT]
static void parse_args(int argc, char* argv[], TecSocketParamsPtr params) {
    if (argc > 1) {
        params->addr = argv[1];
    }
    if (argc > 2) {
        params->port = atoi(argv[2]);
    }
}

// Process client connection.
static int process_data(TecSocketPtr sock, void* arg) {
    (void)arg;
    TECC_TRACE_ENTER("process_data");
    TecBuffer data = TecBuffer_create(1024);
    int err = TecSocket_read(sock, &data);
    if (!err) {
        puts(data.data);
    }
    TecBuffer_done(&data);
    TECC_TRACE_EXIT();
    return err;
}


int main(int argc, char* argv[]) {
    TECC_TRACE_INIT();
    TECC_TRACE_ENTER("main");

    // Set Ctrl-C handler that stops the server.
    TecSignal_init(&sig_quit);
    signal(SIGINT, handle_sigint);

    // Define socket parameters.
    TecSocketParams socket_params;
    TecSocketParams_init(&socket_params);
    // Accept connections from any IPv4 or IPv6 addresses.
    socket_params.addr = kTecAnyAddrIP6;
    // Additional socket parameters if any.
    parse_args(argc, argv, &socket_params);

    // Create and run a thread pool to handle
    // incoming connections concurrently:
    // 8 threads;
    // 8 arena-preallocated IO buffers;
    // Payload: TecSocket objects are allocated
    // from the internal arena;
    // 32 arena-preallocated task slots per thread.
    TecThrPool thread_pool;
    TecThrPool_init(&thread_pool, 8, socket_params.buffer_size,
                    sizeof(TecSocket), 32);
    TecThrPool_run(&thread_pool);

    // Initialize the server.
    TecTCPServer server;
    TecTCPServer_init(&server, &socket_params);
    // Attach the thread pool.
    TecTCPServer_use_thread_pool(&server, &thread_pool);
    // Set up incoming connections handler.
    TecTCPServer_set_client_proc(&server, process_data);

    // Initialize the service worker that will run the server
    // in the dedicated thread.
    TecServiceWorker service_worker;
    TecServiceWorker_init(&service_worker, &server, 1);

    // Start up the server via the service worker using Daemon API.
    int error = TecDaemon_run(&service_worker);
    if (!error) {
        // Wait until `quit` signalled...
        TecSignal_wait(&sig_quit);
        // ... then terminate the service worker.
        TecDaemon_terminate(&service_worker);
        TecDaemon_wait_until_terminated(&service_worker);
    }

    // Clean up.
    TecSocketParams_done(&socket_params);
    TecTCPServer_done(&server);
    TecThrPool_done(&thread_pool);
    TecDaemon_done(&service_worker);
    TecSignal_done(&sig_quit);

    TECC_TRACE_EXIT();
    TECC_TRACE_DONE();
    return error;
}
```

### More Examples
- [server.c](https://github.com/olddeuteronomy/tecc/blob/main/examples/socket/server.c) -- A single-threaded TCP server.
- [ssocket.c](https://github.com/olddeuteronomy/tecc/blob/main/examples/socket/ssocket.c) -- A minimal single-threaded TCP server using the "pure" TecSocket API.
- [client.c](https://github.com/olddeuteronomy/tecc/blob/main/examples/socket/client.c) -- A TPC client using TecService API.
- [socket.c](https://github.com/olddeuteronomy/tecc/blob/main/examples/socket/socket.c) -- A minimal TCP client using the "pure" TecSocket API.

## ToDo
- [ ] Installation procedure.
