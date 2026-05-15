// Time-stamp: <Last changed 2026-05-15 12:00:11 by mac>

#include <stdio.h>

#include "tecc/tecc_time.h"
#include "tecc/tecc_message.h"
#include "tecc/tecc_threads.h"
#include "tecc/tecc_queue.h"



TECC_DEF_MESSAGE(MsgPoint)
    int x;
    int y;
TECC_END_MESSAGE(MsgPoint)

TECC_DEF_MESSAGE(MsgStr80)
    char s[80];
TECC_END_MESSAGE(MsgStr80)


TECC_THREAD_FUNC_RETVAL test1(void* arg) {
    TecQueuePtr q = (TecQueuePtr)arg;
    int counter = 0;
    while (true) {
        TecMsgPtr msg = TecQueue_pop(q);
        if (!msg) {
            break;
        }
        counter += 1;
        printf("%s #%02d: ", TecMsg_tag(msg), counter);
        if (TecMsg_typeof(MsgPoint, msg)) {
            MsgPoint* p = (MsgPoint*)msg;
            printf("{x=%d, y=%d}\n", p->x, p->y);
        }
        else if (TecMsg_typeof(MsgStr80, msg)) {
            MsgStr80* s = (MsgStr80*)msg;
            printf("\"%s\"\n", s->s);
        }
        else {
            printf("Unknown message type.\n");
        }
        TecMsg_free(msg);
    }
    return 0;
}


int main(void) {
    TecQueue q;
    TecQueue_init(&q);

    TecThread t1;
    TecThread_init(&t1);
    TecThread_create(&t1, test1, &q);

    for (int i = 0; i < 10; ++i) {
        // MsgPoint
        TECC_MESSAGE(MsgPoint, p);
        p->x = i;
        p->y = i + 42;
        TecQueue_push(&q, p);
        // MsgStr80
        TECC_MESSAGE(MsgStr80, s);
        sprintf(s->s, "(%d) Hello world!", i + 1);
        TecQueue_push(&q, s);
        // Pause
        tec_sleep_for(MILLISECS(200));
    }
    TecQueue_push(&q, NULL);

    TecThread_join(&t1);
    TecQueue_done(&q);
    return 0;
}
