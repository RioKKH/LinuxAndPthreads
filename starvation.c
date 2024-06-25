#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


/**
 * このコードはスタベーションを説明するためのコードである。
 * スタベーションとは、複数のスレッドが資源獲得競争をしたときに
 * いつまでたっても資源にありつけないものが発生してしまう現象の事
 */

#define N_RAT 3 /* ネズミの数 */

int foodCount; /* 餌の数 */

pthread_mutex_t foodMutex; /* foodCountアクセス同期用ミューテックス */
pthread_cond_t foodCond;   /* foodcountアクセス同期用条件変数 */


int stopRequest;

void mSleep(int msec) {
    struct timespec ts;
    ts.tv_sec = msec/1000;
    ts.tv_nsec = (msec%1000)*1000000;
    nanosleep(&ts, NULL);
}


/*
 * ネズミの動作スレッド
 */
void *ratLife(void *arg) {
    int n = (intptr_t)arg;
    int eatCount = 0;

    while (!stopRequest) {
        pthread_mutex_lock(&foodMutex);
        if (pthread_cond_wait(&foodCond, &foodMutex) != 0) {
            printf("Fatal error on pthread_cond_wait\n");
            exit(1);
        }
        /* 給餌イベント検出 */
        printf("Rat-%d: Got food event.\n", n);
        if (foodCount > 0) {
            /* 餌にありついた */
            printf("Rat-%d: Eat foot.\n", n);
            foodCount--;
            pthread_mutex_unlock(&foodMutex);
            mSleep(20); /* 餌を食べるのに20msecかかるものとする */
            eatCount++;
        } else {
            /* 餌にありつけなかった。少し休んで待つ */
            printf("Rat-%d: orz...\n", n);
            pthread_mutex_unlock(&foodMutex);
            mSleep(50);
        }
    }
    printf("Rat-%d: Ate %d times.\n", n, eatCount);
    return NULL;
}

int main() {
    pthread_t ratThread[N_RAT];
    int i;
    char buf[40];

    pthread_mutex_init(&foodMutex, NULL);
    pthread_cond_init(&foodCond, NULL);
    foodCount = 0;
    stopRequest = 0;

    /* ネズミをN_RAT匹作る */
    for (i = 0; i < N_RAT; i++) {
        /* 
         * void *を使うのは、void *が汎用ポインタであるため、事名rうデータ型を
         * 扱う場合に汎用的なインターフェースを提供することが出来るため
         *
         * intptr_tは、ポインタを格納するために十分なビット幅を持つ整数型として
         * 定義されている。そのため、intptr_t型にキャストすることで、ポインタと
         * 整数間のキャストが安全に行えるようになる。
         **/
        pthread_create(&ratThread[i], NULL, ratLife, (void *)(intptr_t)(i + 1));
    }

    /* メインスレッドは餌の数が入力されるのを待つ */
    while (1) {
        fgets(buf, sizeof(buf), stdin);
        if (strncmp(buf, "stop", 4) == 0) {
            break;
        }
        i = atoi(buf);
        if (i > 0) {
            pthread_mutex_lock(&foodMutex);
            foodCount += i;
            pthread_cond_broadcast(&foodCond);
            pthread_mutex_unlock(&foodMutex);
        }
    }

    stopRequest = 1;
    pthread_cond_broadcast(&foodCond); /* stopRequestにきづかせるため */
    for (i = 0; i < N_RAT; i++) {
        pthread_join(ratThread[i], NULL);
    }

    pthread_mutex_destroy(&foodMutex);
    pthread_cond_destroy(&foodCond);

    return 0;
}
