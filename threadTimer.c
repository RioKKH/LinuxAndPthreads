#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


volatile int isFunctionACompleted = 0;
int* dynamicMemory;

void cleanup_handler(void *arg)
{
    // メモリを解放する
    free(dynamicMemory);
    printf("Memory freed by cleanup handler.\n");
}

void* functionA(void* arg)
{
    // メモリを動的に割り当てる
    dynamicMemory = malloc(sizeof(int) * 10);
    if (dynamicMemory == NULL)
    {
        printf("Failed to allocate memory.\n");
        return NULL;
    }

    // クリーンアップハンドラを設定する
    // pthread_cancel関数はスレッドをキャンセルするが、その副作用としてスレッドが確保した
    // リソース（例えば動的に確保したメモリ）が解放されない場合がある。
    // この問題を解決する一つの方法は、スレッドのキャンセル時にクリーンアップハンドラを
    // 設定すること。これは特定の関数を登録し、スレッドがキャンセルされたときにその関数を
    // 自動的に呼び出す機能である。 
    // このコードではfunctionAの中でpthread_cleanup_push関数を使ってクリーンアップハンドラ
    // を設定する。このハンドラはスレッドがキャンセルされたときに呼び出され、動的に割り当て
    // られたメモリを解放する。処理が正常に完了した場合は、pthread_cleanup_pop()を呼び出して
    // クリーンアップハンドラを削除する。
    pthread_cleanup_push(cleanup_handler, dynamicMemory);

    // ここで何等かの長い処理を行う
    for (int i = 0; i < 10; i++)
    {
        printf("Function A is runnning... %d\n", i);
        sleep(1);
    }

    // クリーンアップハンドラを取り除く。
    // ここでは実行せずに取り除く
    // pthread_cleanup_pushとpthread_cleanup_popは一緒に使われる必要がある。
    // これらの関数はそれぞれハンドラの登録と解除を行う。これらはある種のスタック構造を作り
    // 出すため、それぞれのpthread_cleanup_push()に対して、対応するpthread_cleanup_popが
    // 必要になる。
    // pthread_cleanup_pushに登録したハンドラは、以下のいずれかの条件が満たされた場合に
    // 呼び出される：
    // 1. pthrad_cancelによってスレッドがキャンセルされた場合
    // 2. pthread_exitが呼び出された場合
    // 3. pthread_cleanup_popが非ゼロの引数で呼び出された場合
    // このコード例では、関数Ａが終了した際にpthread_cleanup_pop(0)が呼び出されている。
    // この0の引数はハンドラを呼び出さずに登録を解除することを意味する。
    // このため、関数Aが正常に完了した場合（つまり、タイムアウトせずに）には、ハンドラは
    // 呼び出されず、動的に確保されたメモリはpthread_cleanup_popで直前に解放される。
    // 一方、関数Aがタイムアウトでキャンセルされた場合には、ハンドラが自動的に呼び出され、
    // 動的メモリが解放される。
    pthread_cleanup_pop(0);

    isFunctionACompleted = 1;

    return NULL;
}


void* timerThread(void* arg)
{
    pthread_t tid = *(pthread_t*)arg;

    for (int i = 0; i < 5; i++)
    {
        sleep(1);

        // 関数Aが終了したかどうかをチェックする。
        // 終了していたらタイマースレッドも終了する。
        if (isFunctionACompleted)
        {
            printf("Function A completed before timeout. Timer thread is exiting.\n");
            return NULL;
        }
    }

    // タイムアウトしたので関数Aをキャンセルする。
    pthread_cancel(tid);
    printf("Function A was cancelled due to timeout.\n");

    return NULL;
}


int main()
{
    pthread_t tidA, tidTimer;

    if (pthread_create(&tidA, NULL, functionA, NULL) != 0)
    {
        printf("Failed to create thread for function A.\n");
        exit(1);
    }

    if (pthread_create(&tidTimer, NULL, timerThread, &tidA) != 0)
    {
        printf("Failed to create timer thread.\n");
        exit(1);
    }

    pthread_join(tidA, NULL);
    pthread_join(tidTimer, NULL);

    printf("Exiting the program.\n");

    return 0;
}


