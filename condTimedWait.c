#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>   // clock_gettime, timespec
#include <errno.h> // ETIMEDOUT


pthread_mutex_t mutex;
pthread_cond_t cond;


void *threadFunc(void *arg)
{
	struct timespec ts;

	printf("threadFunc: Start\n");
	sleep(2);
	pthread_mutex_lock(&mutex);
	printf("threadFunc: WAit for signal\n");
	// 現在時刻の2秒後まで成立条件を待つ
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 2;

	// pthread_cond_timedwait()は、
	// 1. 指定時刻に達する前に条件変数がONになれば0を、
	// 2. 条件変数がONにならないまま指定時刻に達したらETIMEDOUTを、
	// 3. 条件変数が初期化されていなどのエラーが発生した時はそれ以外の値を
	// 返す
	switch(pthread_cond_timedwait(&cond, &mutex, &ts))
	{
		case 0:
			printf("threadFunc: Got signal\n");
			break;
		case ETIMEDOUT:
			printf("threadFunc: Timeout\n");
			break;
		default:
			printf("threadFunc: Error on pthread_cond_wait\n");
			exit(1);
	}
	// pthread_cond_timedwait関数はpthread_cond_wait関数と同様に
	// 帰ってくる直前でミューテックスをロックする 
	pthread_mutex_unlock(&mutex);
	return NULL;
}

int main()
{
	pthread_t thread;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	pthread_create(&thread, NULL, threadFunc, NULL);
	sleep(1);
	printf("main: Signal\n");
	pthread_cond_signal(&cond);
	pthread_join(thread, NULL);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	return 0;
}

