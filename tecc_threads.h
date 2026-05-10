// Time-stamp: <Last changed 2026-05-10 15:16:52 by magnolia>
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

#ifndef TECC_THREADS_H
#define TECC_THREADS_H

#include "tecc/tecc_def.h"

#ifdef TECC_PTHREAD
// Use POSIX
#include <pthread.h>

typedef pthread_t TecThrType;
typedef void* (*TecThreadFunc)(void*);
typedef pthread_mutex_t TecMutexType;
typedef pthread_cond_t TecCVType;

#define TECC_THREAD_FUNC_RETVAL void*
#define TECC_THREAD_FUNC_RETURN(val_) return &(val_)

#else
// Use STDC
#include <threads.h>

#define TECC_THREAD_FUNC_RETVAL int
#define TECC_THREAD_FUNC_RETURN(val_) return (val_)

typedef thrd_t TecThrType;
typedef thrd_start_t TecThreadFunc;
typedef mtx_t TecMutexType;
typedef cnd_t TecCVType;

#endif // STDC

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================
*
*                            TecThread
*
 *====================================================================*/
// 16 bytes.
typedef struct tagTecThread {
    int result;  // 0 == OK.
    TecThrType t;
} TecThread;
typedef TecThread* TecThreadPtr;

#define TecThread_result(self) ((self)->result)
#define TecThread_ok(self) (TecThread_result(self) == 0)

/*======================================================================
*
*                            TecMutex
*
 *====================================================================*/
typedef struct tagTecMutex {
    int result; // 0 == OK.
    TecMutexType m;
} TecMutex;
typedef TecMutex* TecMutexPtr;

#define TecMutex_result(self) ((self)->result)
#define TecMutex_ok(self) (TecMutex_result(self) == 0)

/*======================================================================
*
*                            TecCV
*
 *====================================================================*/
typedef struct tagTecCV {
    int result;  // 0 == OK.
    TecCVType c;
} TecCV;
typedef TecCV* TecCVPtr;

#define TecCV_result(self) ((self)->result)
#define TecCV_ok(self) (TecCV_result(self) == 0)

// Implementation

#ifdef TECC_PTHREAD
/*======================================================================
*
*                          POSIX TecThread
*
 *====================================================================*/
#define TecThread_init(self) do {\
    (self)->result = 22; } while(0)

#define TecThread_create(self, func, arg) do {\
        (self)->result = pthread_create(&(self)->t, NULL, (func), (arg));\
    } while(0)

#define TecThread_join(self) do {\
        if (TecThread_ok(self)) { pthread_join((self)->t, NULL);\
            (self)->result = 22; }\
    } while(0)

/*======================================================================
*
*                           POSIX TecMutex
*
 *====================================================================*/
#define TecMutex_init(self) do {\
        (self)->result = pthread_mutex_init(&(self)->m, NULL);\
    } while(0)

#define TecMutex_destroy(self) do {\
        if (TecMutex_ok(self)) { pthread_mutex_destroy(&(self)->m); }\
    } while(0)

#define TecMutex_lock(self) pthread_mutex_lock(&(self)->m)
#define TecMutex_unlock(self) pthread_mutex_unlock(&(self)->m)

/*======================================================================
*
*                             POSIX TecCV
*
 *====================================================================*/
#define TecCV_init(self) do {\
        (self)->result = pthread_cond_init(&(self)->c, NULL); } while(0)

#define TecCV_destroy(self) do {\
    if (TecCV_ok(self)) (self)->result = pthread_cond_destroy(&(self)->c); } while(0)

#define TecCV_signal(self) pthread_cond_signal(&(self)->c)
#define TecCV_broadcast(self) pthread_cond_broadcast(&(self)->c)
#define TecCV_wait(self, _mtx) pthread_cond_wait(&(self)->c, &(_mtx)->m)
#define TecCV_timedwait(self, _mtx, _ts) pthread_cond_timedwait(&(self)->c, &(_mtx)->m, (_ts))

#else

/*======================================================================
*
*                             STDC TecThread
*
 *====================================================================*/
#define TecThread_init(self) do { (self)->result = thrd_error; } while(0)

#define TecThread_create(self, func, arg) do {\
        (self)->result = thrd_create(&(self)->t, (func), (arg));\
    } while(0)

#define TecThread_join(self) do {\
        if (TecThread_ok(self)) { thrd_join((self)->t, &((self)->result));\
            (self)->result = 22; }\                                                               \
    } while(0)

/*======================================================================
*
*                           STDC TecMutex
*
 *====================================================================*/
#define TecMutex_init(self) do {\
        (self)->result = mtx_init(&(self)->m, mtx_plain);\
    } while(0)

#define TecMutex_destroy(self) do {\
        if (TecMutex_ok(self)) mtx_destroy(&(self)->m);\
    } while(0)

#define TecMutex_lock(self) mtx_lock(&(self)->m)
#define TecMutex_unlock(self) mtx_unlock(&(self)->m)

/*======================================================================
*
*                            STDC TecCV
*
 *====================================================================*/

#define TecCV_init(self) do {\
        (self)->result = cnd_init(&(self)->c); } while(0)

#define TecCV_destroy(self) do {\
        if (TecCV_ok(self)) cnd_destroy(&(self)->c); } while(0)

#define TecCV_signal(self) cnd_signal(&(self)->c)
#define TecCV_broadcast(self) cnd_broadcast(&(self)->c)
#define TecCV_wait(self, _mtx) cnd_wait(&(self)->c, &(_mtx)->m)
#define TecCV_timedwait(self, _mtx, _ts) cnd_timedwait(&(self)->c, &(_mtx)->m, _ts)

#endif // STDC


#ifdef __cplusplus
}
#endif

#endif // TECC_THREADS_H
