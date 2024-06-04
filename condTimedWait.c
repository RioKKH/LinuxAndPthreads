#include <pthread.h> /* pthread_* */
#include <unistd.h>  /* sleep */
#include <stdio.h>   /* printf */
#include <stdlib.h>  /* exit */
#include <time.h>    /* clock_gettime */
#include <errno.h>   /* ETIMEDOUT */


pthread_mutex_t mutex;
pthread_cond_t cond;

void *threadFunc(void *arg) {
    struct timespec ts;

    printf("threadFunc: Start\n");
    sleep(1);
    pthread_mutex_lock(&mutex);
    printf("threadFunc: Wait for signal\n");
    /* 現在時刻の2秒後まで条件成立を待つ */
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 2;
    switch(pthread_cond_timedwait(&cond, &mutex, &ts)) {
        case 0:
            printf("threadFunc: Got signal\n");
            break;
        case ETIMEDOUT:
            printf("threadFunc: Timeout\n");
            break;
        default:
            printf("threadFunc: Error on pthread_cond_wait\n");
            exit(1);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_t thread;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&thread, NULL, threadFunc, NULL);
    /* 2秒でタイムアウトするようにしているのだから、5秒待てばタイムアウトする */
    sleep(5);
    printf("main: Signal\n");
    pthread_cond_signal(&cond);
    pthread_join(thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}

