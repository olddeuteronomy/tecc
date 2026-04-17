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

#ifndef TECC_MESSAGE_H
#define TECC_MESSAGE_H

#include <stdlib.h>
#include <string.h>

#include "tecc/tecc_def.h"  // IWYU pragma: keep
#include "tecc/tecc_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                        Message object
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct tagTecMsg TecMsg;
typedef TecMsg* TecMsgPtr;

typedef struct tagTecMsg {
    const char* tag;         // Message type as string.
    void (*done)(TecMsgPtr); // Destructor (NULL by default).
} TecMsg;

// Message callback function.
typedef void (*TecMsgCallbackFunc)(TecMsgPtr, void*);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                          Message API
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Message type as string.
#define TecMsg_type(type) type##_msg_

// Message object tag (type) as string.
#define TecMsg_tag(msg) (((TecMsgPtr)msg)->tag)

// Check message type.
#define TecMsg_typeof(type, msg) (strcmp(TecMsg_type(type), TecMsg_tag(msg)) == 0)

// Initialize a message by setting its type (tag).
#define TecMsg_init(type, msg)\
    (msg)->hdr.tag = TecMsg_type(type);\
    (msg)->hdr.done = NULL

// Allocate a new message and set its type.
#define TecMsg_new(type, ptr)\
    ptr = (type *)TECC_CALLOC(1, sizeof(type));\
    TecMsg_init(type, ptr)

#define TecMsg_done_func(ptr) ((TecMsgPtr)ptr)->done

// Call a destructor then deallocate a message.
#define TecMsg_free(msg) {\
    if (msg) {\
        if (TecMsg_done_func(msg)) { TecMsg_done_func(msg)((TecMsgPtr)msg); }\
        TECC_FREE(msg);\
        }\
    } while(0)

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Define a message object.

TECC_DEF_MESSAGE(MsgPoint)
    int x;
    int y;
TECC_END_MESSAGE(MsgPoint)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define TECC_DEF_MESSAGE(type)\
    TECC_unused static const char* TecMsg_type(type) = #type;\
    typedef struct tag##type { TecMsg hdr;

#define TECC_END_MESSAGE(type) } type;\
    typedef type * type##Ptr;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Create a message object of given type.

TECC_MESSAGE(MsgPoint, p);
p->x = 1;
p->y = 42;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define TECC_MESSAGE(type, var) type* var = NULL; TecMsg_new(type, var)


#ifdef __cplusplus
}
#endif

#endif // TECC_MESSAGE_H
