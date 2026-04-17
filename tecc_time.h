// Time-stamp: <Last changed 2026-04-17 13:50:07 by magnolia>
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

#ifndef TECC_TIME_H
#define TECC_TIME_H

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

#include <time.h>
#include <stdint.h>

#include "tecc_def.h" // IWYU pragma: keep

#ifdef __cplusplus
extern "C" {
#endif


// Nanoseconds since epoch (1970.01.01:00:00Z).
typedef int64_t TecTimePoint;

// Converting to nanoseconds.
#define MICROSECS(us) ((TecTimePoint)(ms  * 1000LL))
#define MILLISECS(ms) ((TecTimePoint)(ms  * 1000000LL))
#define SECONDS(sec)  ((TecTimePoint)(sec * 1000000000LL))
#define MINUTES(m)    (60LL * SECONDS(m))
#define HOURS(h)      (60LL * MINUTES(h))
#define DAYS(d)       (24LL * HOURS(d))

// Convertions between various time representations.
TECC_API void tec_tp_to_ts(TecTimePoint tp, struct timespec* ts);
TECC_API void tec_tp_to_tm(TecTimePoint tp, struct tm* tm);

TECC_API TecTimePoint tec_ts_to_tp(struct timespec* ts);
TECC_API void tec_ts_to_tm(struct timespec* ts, struct tm* tm);

// Nanoseconds since UNIX epoch.
TECC_API TecTimePoint tec_tp_now(void);

// Sleep the current thread for the given time interval.
TECC_API void tec_sleep_for(TecTimePoint dur);


#ifdef __cplusplus
}
#endif

#endif // TECC_TIME_H
