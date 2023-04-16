#include <pthread.h>
#include <stdio.h>


void *thread_function(void *arg);

int main(int argc, char* argv[])
{
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);

    // thread1とthread2が同じスレッドであるかどうかを確認する
    if (pthread_equal(thread1, thread2))
    {
        printf("thread1 and thread2 are equal\n");
    }
    else
    {
        printf("thread1 and thread2 are not equal\n");
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}

void *thread_function(void *arg)
{
    // スレッドで実行する処理を記述する
    return NULL;
}


