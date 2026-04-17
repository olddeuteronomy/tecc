// Time-stamp: <Last changed 2026-04-07 16:31:21 by magnolia>

#include <stdio.h>

#include "tecc/tecc_signal.h"


int setter(void *arg)
{
    TecSignalPtr sig = (TecSignal*)arg;
    tec_sleep_for(SECONDS(10));
    TecSignal_set(sig);
    return 0;
}


int main(void)
{
    TecSignal sig;
    TecSignal_init(&sig);

    thrd_t t;
    thrd_create(&t, setter, &sig);

    if (TecSignal_wait_for(&sig, SECONDS(5))) {
        puts("Signal set.");
    } else {
        puts("Timeout!");
        /* thrd_detach(t); */
    }

    thrd_join(t, NULL);
    TecSignal_done(&sig);
    return 0;
}
