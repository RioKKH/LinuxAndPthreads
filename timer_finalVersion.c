#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h> // ETIMEDOUT

pthread_mutex_t lock;
pthread_cond_t cv;

typedef struct {
	int arg1;
	float arg2;
} MyArgs;

// これがラッパー関数であり、引数の型と戻り値の型がvoid*である。
void *functionA_wrapper(void *arg)
{
	MyArgs *args = (MyArgs *)arg;
	int *result_ptr = (int *)malloc(sizeof(int));

	// 本番のコードではここに関数Aの処理が入る
	for (int i = 0; i < 1; i++)
	{
		printf("Function A is running... %d\n", i);
		printf("Arg1: %d, Arg2: %f\n", args->arg1, args->arg2);
		sleep(1);
	}
	// スレッドの中で値を修正
	args->arg1 = 10;

	// 関数Aが終了したので、共有状態を更新する
	pthread_mutex_lock(&lock);
	pthread_cond_signal(&cv);
	pthread_mutex_unlock(&lock);

	*result_ptr = args->arg1;
	return (void *)result_ptr;
	// return (void *)(long)args->arg1;
	// return NULL;
}

void *timerThread(void *arg)
{
	pthread_t tid = *(pthread_t *)arg;
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 5; // 今から5秒後まで待つ、の意味

	pthread_mutex_lock(&lock);
	// 第3引数には"この時刻担ったら条件待ちをやめて帰ってくる"
	// という時刻を引数として指定する

	// switch文を使った書き方
	switch(pthread_cond_timedwait(&cv, &lock, &ts))
	{
		case 0:
		// タイムアウト前に関数Aが終了
			printf("Function A completed before timeout. Timer thread is exiting.\n");
			break;
		case ETIMEDOUT:
		// タイムアウトしたので関数Aをキャンセルする
			pthread_cancel(tid);
			printf("Function A was cancelled due to timeout.\n");
			break;
		default:
			perror("timerThread: Error on pthread_cond_timedwait\n");
			printf("File: %s, Line: %d\n", __FILE__, __LINE__);
			exit(1);
	}

	/* switch文を使わない書き方
	if (pthread_cond_timedwait(&cv, &lock, &ts) == ETIMEDOUT)
	{
		pthread_cancel(tid);
		printf("Function A was cancelled due to timeout.\n");
	}
	else
	{
		printf("Function A completed before timeout. Timer thread is exiting.\n");
	}
	*/

	pthread_mutex_unlock(&lock);

	return NULL;
}

int main()
{
	pthread_t tidA, tidTimer;
	MyArgs args = {123, 456.789};
	void *ret;

	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cv, NULL);

	if (pthread_create(&tidA, NULL, functionA_wrapper, &args) != 0)
	{
		perror("Failed to create thread for function A. Error");
		printf("File: %s, Line: %d\n", __FILE__, __LINE__);
		exit(1);
	}

	if (pthread_create(&tidTimer, NULL, timerThread, &tidA) != 0)
	{
		perror("Failed to create timer thread. Error");
		printf("File: %s, Line: %d\n", __FILE__, __LINE__);
		exit(1);
	}

	pthread_join(tidA, &ret);
	// pthread_join(tidA, NULL);
	pthread_join(tidTimer, NULL);

	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cv);

	printf("args->arg1: %d\n", args.arg1);
    printf("functionA_wrapper returned: %d\n", *(int*)ret);
	printf("Exiting the program.\n");
	free(ret);

	return  0;
}
