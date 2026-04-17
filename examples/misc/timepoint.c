// Time-stamp: <Last changed 2026-04-13 11:23:06 by magnolia>

#include <stdio.h>

#include "tecc/tecc_time.h"


void print_timepoint(TecTimePoint tp) {
    struct tm t;
    tec_tp_to_tm(tp, &t);
    printf("%ld = ", tp);
    char buf[80];
    strftime(buf, sizeof(buf), "%FT%TZ", &t);
    puts(buf);
}


int main(void) {
    print_timepoint(0);
    print_timepoint(tec_tp_now());
    print_timepoint(tec_tp_now() + DAYS(2));

    return 0;
}
