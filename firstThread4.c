#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * スレッドコンテキスト（スレッドの状態を保持するためのメモリ）
 * に含まれるスレッドの戻り値
 * この値はpthread_join関数の第2引数でしたいした場所に格納される
 */
void *threadFunc(void *arg)
{
   int  n = (int)arg;
    int i;

    for (i = 0; i < n; i++)
    {
        printf("I'm threadFunc: %d\n", i);
        sleep(1);
    }

    return (void *)n;
}

int main(int argc, char *argv[])
{
    pthread_t thread;
    int n, i, ret;

    if (argc > 1)
    {
        n = atoi(argv[1]);
    }
    else
    {
        n = 1;
    }

    if (pthread_create(&thread, NULL, threadFunc, (void *)n) != 0)
    {
        printf("Error: Failed to create new thread.\n");
        exit(1);
    }

    for (i = 0; i < 5; i++)
    {
        printf("I'm main: %d\n", i);
        sleep(1);
    }

    // スレッドの戻り値はpthread_join関数の第2引数で指定された場所に格納される
    if (pthread_join(thread, (void **)&ret) != 0)
    {
        printf("Error: Failed to wait for the thread termination.\n");
        exit(1);
    }

    // スレッドの戻り値を使う例
    printf("threadFunc has been terminated with number %d\n", ret);
    printf("Bye.\n");

    return 0;
}

