#include <pthread.h>
#include <stdio.h>


void *thread_function(void *arg);

int main(int argc, char* argv[])
{
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);

    // thread1とmain threadが同じスレッドであるかを確認する
    if (pthread_equal(pthread_self(), thread1))
    {
        printf("main thread and thread1 are equal\n");
    }
    else
    {
        printf("main thread and thread1 are not equal\n");
    }

    if (pthread_equal(pthread_self(), thread2))
    {
        printf("main thread and thread2 are equal\n");
    }
    else
    {
        printf("main thread and thread2 are not equal\n");
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}

void *thread_function(void *arg)
{
    // 自分自身のスレッドIDを表示する
    printf("Thread ID: %lu\n", pthread_self());

    return NULL;
}


