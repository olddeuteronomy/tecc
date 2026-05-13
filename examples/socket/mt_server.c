// Time-stamp: <Last changed 2026-05-13 10:41:03 by magnolia>
/*======================================================================
*
* Construct and run multi-threaded TCP server with arena allocator.
*
 *====================================================================*/

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


// Usage: server [ADDR] [PORT]
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
    TECC_TRACE_ENTER("process_str");
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

    // Set Ctrl-C handler that stops polling.
    TecSignal_init(&sig_quit);
    signal(SIGINT, handle_sigint);

    // Define socket parameters.
    TecSocketParams socket_params;
    TecSocketParams_init(&socket_params);
    // Accepting connections from any IPv4 or IPv6 addresses.
    socket_params.addr = kTecAnyAddrIP6;
    parse_args(argc, argv, &socket_params);

    // Create a thread pool for handling incoming connections concurently:
    // 8 threads,
    // 8 arena-preallocated buffers,
    // Payload: TecSocket allocated using internal arena when possible,
    // 32 arena-preallocated socket slots per thread.
    TecThrPool thread_pool;
    TecThrPool_init(&thread_pool, 8, socket_params.buffer_size,
                    sizeof(TecSocket), 32);
    TecThrPool_run(&thread_pool);

    // Initialize the server.
    TecTCPServer server;
    TecTCPServer_init(&server, &socket_params);
    // Attach the thread pool.
    TecTCPServer_use_thread_pool(&server, &thread_pool);
    // Incoming connection processor.
    TecTCPServer_set_client_proc(&server, process_data);

    // Initialize the service worker that runs the server in the dedicated thread.
    TecServiceWorker service_worker;
    TecServiceWorker_init(&service_worker, &server, 1);

    // Start the server via ServiceWorker using Daemon API.
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
