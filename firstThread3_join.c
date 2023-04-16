#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


void *threadFunc(void *arg)
{
    int  n = (int)arg;
    int i;

    for (i = 0; i < n; i++)
    {
        printf("I'm threadFunc: %d\n", i);
        sleep(1);
    }
    return NULL;
}

/**
 * thredFunc内のループ回数がコマンドライン引数で与えられ、
 * プログラミング時には不定である場合
 */
int main(int argc, char *argv[])
{
    pthread_t thread;
    int n, i;

    if (argc > 1)
    {
        n = atoi(argv[1]);
    }
    else
    {
        n = 1;
    }

    // pthread_createの定義に従うため引数を無理やりvoid *型に強引にキャストしている
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

    // 他のスレッドの終了を確実に待ってからプロセスを終了する、もしくは他の処理を
    // 開始するようにする。
    // $1 pthread_create関数でスレッドを作成するときに作られたスレッドID pthread_t thread
    // $2 スレッドの戻り値を格納する場所の指定 void **retval
    // -->スレッドの戻り値を使う必要がない梅安いはNULLを指定する
    //
    // 他のスレッド を生成したら、生成したスレッドが責任をもって生成されたスレッドすべての
    // 終了を見届ける、というプログラミングスタイルが鉄則である。
    if (pthread_join(thread, NULL) != 0)
    {
        printf("Error: Failed to wait for the thread termination\n");
        exit(1);
    }

    printf("Bye.\n");

    return 0;
}
