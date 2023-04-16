#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/**
 * スレッドの属性はOSやライブラリのバージョンの違いにより
 * 正しく動作しないことがある。どうしても使わなければいけない
 * 理由がない限りは、デフォルトの属性(NULL)を使うことが推奨
 * される
 */

#define SIZE 10000000

void *threadFunc(void *arg)
{
    // threadFunc()がスタック上に80MB以上の巨大な変数を作る
    // pthreadsのデフォルトのスタックサイズはそれよりも小さい
    // 事が多い(通常20MB程度)。そのため、このままではスタック
    // オーバーフローしてしまう
    double table[SIZE];
    int i;

    for (i = 0; i < SIZE; i++)
    {
        table[i] = i * 3.14;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_attr_t attr;
    pthread_t thread;

    // 上述したスタックオーバーフローを回避するため、スレッド属性を用いて
    // スタックサイズを指定する
    pthread_attr_init(&attr);
    if (pthread_attr_setstacksize(&attr, SIZE*sizeof(double)+100000) != 0)
    {
        printf("Failed to set stack size.\n");
        exit(1);
    }

    if (pthread_create(&thread, &attr, threadFunc, NULL) != 0)
    {
        printf("Error: Failed to create new thread.\n");
        exit(1);
    }

    if (pthread_join(thread, NULL) != 0)
    {
        printf("Error: Failed to wait for the thread termination.\n");
        exit(1);
    }

    return 0;
}
