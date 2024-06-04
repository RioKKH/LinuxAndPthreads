/**
 * 条件変数を使うプログラム
 * pthread_cond_signal 通知元
 * pthread_cond_wait 通知を受ける方
 * 条件変数はpthread_cond_tという型を持ち、
 * ptyhread_cond_init()で初期化され、
 * pthread_cond_destroy()で消去する
 */

#include <pthread.h>
#include <unistd.h> // sleep
#include <stdio.h>  // printf
#include <stdlib.h> // NULL 一般ユーティリテに関する型、マクロ、関数


pthread_mutex_t mutex; // <-----------下の条件変数と対になるミューテックス
pthread_cond_t cond;   // <-----------条件変数

void *threadFunc(void *arg)
{
	printf("threadFunc: Start\n");
	sleep(2);

	pthread_mutex_lock(&mutex); // <-------------- lock
	// pthread_cond_wait()を呼び出す前にミューテックスをロックする

	printf("threadFunc: Wait for signal\n");
	// 1. 条件変数をOFFにする
	// 2. ミューテックスをアンロックする
	// 3. 条件変数がONになるのを待ち続ける
	// 4. 条件変数がONになったらミューテックスをロックする
	// 1 ~ 3の処理がアトミックに行われる
	if (pthread_cond_wait(&cond, &mutex) != 0)
	{
		printf("threadFunc: Error on pthread_cond_wait\n");
		exit(1);
	}
	printf("threadFunc: Got signal\n");

	pthread_mutex_unlock(&mutex); // <-------------- unlock

	return NULL;
}

int main()
{
	pthread_t thread;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	pthread_create(&thread, NULL, threadFunc, NULL);
	sleep(3);
	printf("main: Signal\n");
	printf("Before\n");
	pthread_cond_signal(&cond);
	pthread_join(thread, NULL);
	printf("After\n");

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	return 0;
}
