#include "xyqueue.h"
#include <pthread.h>
#include <unistd.h> // sleep
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <error.h>

#define WIDTH 78       /* スクリーン幅 */
#define HEIGHT 23      /* スクリーン高さ */
#define MAX_FLV 1      /* 描画するハエの数 */

#define QUEUE_SIZE 10  /* 目標地点キュー要素数 */

static int stopRequest; /* スレッド終了フラグ */
static int drawRequest; /*( 描画要求フラグ */
static pthread_mutex_t drawMutex; /* 描画要求待ち用ミューテックス */
static pthread_cond_t  drawCond;  /* 描画要求待ち用ミューテックス */

static void requestDraw(void);


/*
 * ミリ秒単位でスリープする
 */
void mSleep(int msec) {
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

/*
 * ミリ秒単位で条件町をする
 */
int pthread_cond_timedwait_msec(pthread_cond_t *cond, pthread_mutex_t *mutex, long msec) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += msec / 1000;
    ts.tv_nsec += (msec % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    return pthread_cond_timedwait(cond, mutex, &ts);
}

