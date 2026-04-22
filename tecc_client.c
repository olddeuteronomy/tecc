// Time-stamp: <Last changed 2026-04-22 15:52:42 by magnolia>
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
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L   // This line fixes the "storage size of ‘hints’ isn’t known" issue.
#endif

#include "tecc/tecc_def.h"
#include "tecc/tecc_client.h"


TECC_IMPL void TecClientParams_init_(TecClientParamsPtr self) {
    self->mode = 0;
    self->hash_table_size = TECC_CLIENT_HASH_TABLE_SIZE;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                      TecService overrides
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* static void start(TecServicePtr svc, TecSignalPtr sig_started, int* error) { */

/* } */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*                        TecClient API
*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

TECC_IMPL void TecClient_init_(TecClientPtr self,
                               TecClientParamsPtr client_params,
                               TecSocketParamsPtr socket_params) {
    TecService_init(self, client_params->hash_table_size);
    self->client_params = client_params;
    self->socket_params = socket_params;
    // Empty buffer.
    TecBuffer_init(&self->buffer, 0, socket_params->buffer_size); // Empty buffer.
}

// Destructor.
TECC_IMPL void TecClient_done_(TecClientPtr self) {
    TecBuffer_done(&self->buffer);
    TecService_done_(&self->service);
}
