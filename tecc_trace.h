// Time-stamp: <Last changed 2026-04-21 02:56:54 by magnolia>
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

#ifndef TECC_TRACE_H
#define TECC_TRACE_H

#ifdef TECC_TRACE_ON

#include "tecc/tecc_def.h"    // IWYU pragma: keep
#include "tecc/tecc_time.h"

#define TECC_TRACE_VAR_ trace_me__0

#define TECC_TRACE_ENTER(name)\
    TecTracer TECC_TRACE_VAL_ = {name, tec_tp_now()};\
    tec_trace_enter(&(TECC_TRACE_VAL_))

#define TECC_TRACE_EXIT()\
    tec_trace_exit(&(TECC_TRACE_VAL_))

#define TECC_TRACE(...)\
    tec_trace(&(TECC_TRACE_VAL_), __VA_ARGS__)

#define TECC_TRACE_INIT() tec_trace_init()
#define TECC_TRACE_DONE() tec_trace_done()

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagTecTracer TecTracer;
typedef TecTracer* TecTracerPtr;

typedef struct tagTecTracer {
    const char* name;
    TecTimePoint start_time;
} TecTracer;

TECC_API void tec_trace_init();
TECC_API void tec_trace_done();

TECC_API void tec_trace_enter(TecTracerPtr ptr);
TECC_API void tec_trace_exit(TecTracerPtr ptr);
TECC_API void tec_trace(TecTracerPtr ptr, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#else
// !TECC_TRACE_ON
#define TECC_TRACE_ENTER(name) ((void)0)
#define TECC_TRACE_EXIT() ((void)0)

#define TECC_TRACE(...) ((void)0)

#define TECC_TRACE_INIT() ((void)0)
#define TECC_TRACE_DONE() ((void)0)

#endif // TECC_TRACE_ON

#endif // TECC_TRACE_H
