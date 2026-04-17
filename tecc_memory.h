// Time-stamp: <Last changed 2026-04-17 13:50:05 by magnolia>
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

#ifndef TECC_MEMORY_H
#define TECC_MEMORY_H

#include "tecc/tecc_def.h" // IWYU pragma: keep

#ifdef __cplusplus
extern "C" {
#endif

#define TECC_MALLOC(size) malloc(size)
#define TECC_CALLOC(num, size) calloc(num, size)
#define TECC_FREE(ptr) free(ptr)

#ifdef __cplusplus
}
#endif

#endif // TECC_MEMORY_H
