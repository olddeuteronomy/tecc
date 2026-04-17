// Time-stamp: <Last changed 2026-04-17 13:50:03 by magnolia>
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

// gmtime_r trick
#if defined(__APPLE__)
  #define _DARWIN_C_SOURCE
#elif defined(__unix) || defined(__linux)
  #define _POSIX_C_SOURCE 200809L
#elif defined(_WIN64) || defined(_WIN32)
  #define USE_GMTIME_S 1
#else
  #error "Unknown platform"
#endif

#include <threads.h>

#include "tecc/tecc_def.h"
#include "tecc/tecc_time.h"


TECC_IMPL void tec_tp_to_ts(TecTimePoint tp, struct timespec* ts) {
    ts->tv_sec = tp / 1000000000L;
    ts->tv_nsec = tp - (ts->tv_sec * 1000000000L);
}

TECC_IMPL TecTimePoint tec_ts_to_tp(struct timespec* ts) {
    return (TecTimePoint)ts->tv_sec * 1000000000LL + (TecTimePoint)ts->tv_nsec;
}

TECC_IMPL void tec_ts_to_tm(struct timespec* ts, struct tm* tm) {
#if defined (USE_GMTIME_S)
    // Windows
    gmtime_s(tm, &ts->tv_sec);
#else
    // Apple, Linux, Unix
    gmtime_r(&ts->tv_sec, tm);
#endif
}

TECC_IMPL void tec_tp_to_tm(TecTimePoint tp, struct tm* tm) {
    struct timespec ts;
    tec_tp_to_ts(tp, &ts);
    tec_ts_to_tm(&ts, tm);
}

// Nanoseconds since UNIX epoch.
TECC_IMPL TecTimePoint tec_tp_now(void) {
    struct timespec now;
    if (timespec_get(&now, TIME_UTC) == 0) {
        return 0LL;
    }
    return tec_ts_to_tp(&now);
}

// Sleep the current thread for the given interval.
TECC_IMPL void tec_sleep_for(TecTimePoint dur) {
    struct timespec ts;
    tec_tp_to_ts(dur, &ts);
    thrd_sleep(&ts, NULL);
}
