#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *thread_function(void *arg);

int main(int argc, char* argv[])
{
    pthread_t thread;

    pthread_create(&thread, NULL, thread_function, NULL);

    // メインスレッドで5秒待機する
    sleep(5);

    // スレッドをキャンセルする
    if (pthread_cancel(thread) != 0)
    {
        perror("pthread_cancel");
        exit(EXIT_FAILURE);
    }

    pthread_join(thread, NULL);

    return 0;
}

void *thread_function(void *arg)
{
    // スレッドで実行する処理を記述する
    int i;
    for (i = 0; ; i++)
    {
        printf("Thread is running %d\n", i);
        // スレッドがキャンセルされたかどうかを確認する
        if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0)
        {
            perror("pthread_setcancelstate");
            exit(EXIT_FAILURE);
        }

        if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
        {
            perror("pthread_setcanceltype");
            exit(EXIT_FAILURE);
        }
    }
}

