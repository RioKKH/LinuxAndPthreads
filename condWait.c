#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex;
pthread_cond_t cond;

void *threadFunc(void *arg) {
    printf("threadFunc: Start\n");
    sleep(2);
    // pthread_cond_wait()関数を呼び出す前にcondと対になるmutexをロックしておく
    pthread_mutex_lock(&mutex);
    printf("threadFunc: Wait for signal\n");
    // pthread_cond_wait()関数を呼び出すと、
    // 1. 条件変数をOFFにして、
    // 2. mutexをアンロックしてから待機する
    // 3. 条件変数がONになるのを待ち続ける
    // 4. 条件変数がONになったら、mutexをロックしてから待機を終了する
    // 1 ~ 3の処理はatomicに行われる
    // pthread_cond_wait()関数から帰ってきた時点でmutexはロックされている点も重要
    if (pthread_cond_wait(&cond, &mutex) != 0) {
        printf("threadFunc: Error on pthread_cond_wait\n");
        exit(1);
    }
    printf("threadFunc: Got signal\n");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_t thread;

    /* 条件変数を使う時には、原則としてそれと対になるミューテックスを用意する */
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&thread, NULL, threadFunc, NULL);
    sleep(3);
    printf("main: Signal\n");
    pthread_cond_signal(&cond); // ここで待っているスレッドにシグナルを送る
    pthread_join(thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}

