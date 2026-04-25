// Time-stamp: <Last changed 2026-04-23 11:10:37 by magnolia>
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
#include <stdio.h>
#include <stdarg.h>
#include <threads.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_signal.h"
#include "tecc/tecc_time.h"
#include "tecc/tecc_trace.h"


static TecMutex guard_ = {false};

static const int shift = 2;
thread_local static int level = 0;
static const char zs[1] = {0};


TECC_IMPL void tec_trace_init() {
    TecMutex_init(&guard_);
}

TECC_IMPL void tec_trace_done() {
    TecMutex_destroy(&guard_);
}

TECC_IMPL void tec_trace_enter(TecTracerPtr tr) {
    if (!guard_.ok) return;
    TecMutex_lock(&guard_);
    printf("%*s+ %s entered @TID=%08lx.\n", level, zs, tr->name, thrd_current());
    level += shift;
    TecMutex_unlock(&guard_);
}

TECC_IMPL void tec_trace_exit(TecTracerPtr tr) {
    if (!guard_.ok) return;
    TecMutex_lock(&guard_);
    level -= shift;
    TecTimePoint diff_time_ns = tec_tp_now() - tr->start_time;
    TecTimePoint diff_time_ms = diff_time_ns / 1000000; // Milliseconds
    TecTimePoint diff_time_us = diff_time_ns / 1000;    // Microseconds
    printf("%*s- %s completed in %ldms/%ldus.\n",  level, zs, tr->name, diff_time_ms, diff_time_us);
    TecMutex_unlock(&guard_);
}


TECC_IMPL void tec_trace(TecTracerPtr tr, const char* fmt, ...) {
    if (!guard_.ok) return;
    TecMutex_lock(&guard_);
    printf("%*s* %s: ", level, zs, tr->name);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    TecMutex_unlock(&guard_);
}
