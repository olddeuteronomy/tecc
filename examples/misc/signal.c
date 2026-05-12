// Time-stamp: <Last changed 2026-05-13 01:45:28 by magnolia>

#include <stdio.h>

#include "tecc/tecc_time.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_threads.h"


TECC_THREAD_FUNC_RETVAL setter(void *arg) {
    TecSignalPtr sig = (TecSignal*)arg;
    tec_sleep_for(SECONDS(10));
    TecSignal_set(sig);
    return 0;
}


int main(void) {
    TecSignal sig;
    TecSignal_init(&sig);
    TecThread t;
    TecThread_init(&t);
    TecThread_create(&t, setter, &sig);

    if (TecSignal_wait_for(&sig, SECONDS(5))) {
        puts("Signal set.");
    } else {
        puts("Timeout!");
    }

    TecThread_join(&t);
    TecSignal_done(&sig);
    return 0;
}
