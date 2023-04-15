#include <pthread.h> // pthreadsライブラリを使うため
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


void *threadFunc(void *arg)
{
    int i;

    for (i = 0; i < 3; i++)
    {
        printf("I'm thradFunc: %d\n", i);
        sleep(1);
    }

    return NULL;
}


int main(void)
{
    pthread_t thread;
    int i;

    // スレッドを新たに生成し、別スレッドで関数の実行を開始させる
    // $1 thread IDを格納
    // $2 スレッドの属性を指定する
    // $3 新たなスレッドで実行する関数
    // $4 start_routineの引数になる値
    if (pthread_create(&thread, NULL, threadFunc, NULL) != 0)
    {
        printf("Erro: Failed to create new thread.\n");
        exit(1);
    }
    // 終了を待たずにすぐに帰ってくることに注意

    for (i = 0; i < 5; i++)
    {
        printf("I'm main: %d\n", i);
        sleep(1);
    }

    return 0;
}


