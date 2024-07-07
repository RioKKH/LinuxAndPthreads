#include <pthread.h>
#include <unistd.h>  // sleep(0
#include <stdio.h>   // printf()
#include <stdlib.h>  // exit()
#include <string.h>  // strcspn()
#include <time.h>
#include <errno.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/*
 * ミリ秒単位で条件待ちをする関数
 */
int pthread_cond_timedwait_msec(pthread_cond_t *cond,
                                pthread_mutex_t *mutex,
                                long msec)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += msec/1000;
    ts.tv_nsec += (msec%1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    return pthread_cond_timedwait(cond, mutex, &ts);
}

void *threadFunc(void *arg) {
    printf("threadFunc: Start\n");

    long msec = 3000;
    pthread_mutex_lock(&mutex);
    printf("threadFunc: Wait for signal\n");

    // ここでsignalを受け取るまで待機、もしくはmsecミリ秒経過したら
    // pthread_cond_timedwait_msec関数はETIMEDOUTを返す
    switch (pthread_cond_timedwait_msec(&cond, &mutex, msec)) {
        case 0:
            printf("%s: Got signal\n", __func__);
            break;
        case ETIMEDOUT:
            printf("%s: Timeout\n", __func__);
            exit(1);
        default:
            printf("Fatal error on %s.\n", __func__);
            exit(1);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}


int main() {
    pthread_t thread;
    char input[10];

    // pthread_create(&thread, NULL, threadFunc, (void *)1000);
    pthread_create(&thread, NULL, threadFunc, NULL);
    printf("Enter any text to signal the thread\n");
    fgets(input, sizeof(input), stdin);

    /*
     * fgetsによって読み込まれた入力文字列から末尾の改行文字を削除する
     *
     * strcpan関数
     * sizt_t strcspn(const char *str1, const char *str2);
     * str1の文字列の先頭からstr2の文字列に含まれる文字が現れるまでの文字数を返す
     *
     * input[index] = 0;
     * C言語では、文字列の終端はnull文字(\0, ASCIIコード0)で表される
     * この式は指定されたindexの位置にnull文字を設定する
     * これにより、その位置以降の文字は文字列の一部とみなされなくなる
     *
     * fgets関数
     * fgets関数は入力の最後に改行文字"\n"を含める
     *
     * 例
     * 例えば入力が"Hello\n"だった場合
     * 1. fgets関数によってinputには"Hello\n"が格納される
     * 2. strcspn(input, "\n")は5を返す ("Hello"の長さ)
     * 3. input[5] = 0;が実行される
     * 4. 結果の文字列: "Hello\0"
     */
    input[strcspn(input, "\n")] = 0;

    printf("main: Received input: %s\n", input);
    pthread_cond_signal(&cond); // スレッドにsignalを送る

    pthread_join(thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
