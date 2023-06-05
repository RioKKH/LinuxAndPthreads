#include <asm-generic/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
	int val1;
	int val2;
} FunctionA_Args;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int isFunctionACompleted = 0;

// メモリクリンナップハンドラの定義
void cleanup(void *arg)
{
	printf("Cleanup hander was called. Memory was freed.\n");
	free(arg);
}

// 関数Aの定義
int functionA(int val1, int val2)
{ // 関数Aの処理 (仮にここではval1とval2を足し合わせる処理とする)
	int result = val1 + val2;
	for (int i = 0; i < 1; i++)
	// for (int i = 0; i < 10; i++)
	{
		printf("Function A is running... %d\n", i);
		sleep(1);
	}
	return result;
}

// 関数Aのラッパー関数
// 動的にメモリを確保して結果を返す
// ptread_createの第3引数でしていするスレッド関数のプロトタイプはvoid *(*)(void *)
// でなければならないという制約がある。つまり戻り値としてvoidのポインタを返すことが
// 求められている。
// 関数Aがvoid*以外の型を返す場合、スレッドからその戻り値を適切に取得するためには、
// それをヒープ領域 (mallocなどで確保した領域)に保存し、そのアドレスを返すことで
// 実現する。なぜならば、ヒープ領域は関数のスコープを超えても生存期間が続くから。
// もしスタック領域(ローカル変数が確保される領域)に保存しようとすると、関数が終了
// した時点でそのメモリが開放されてしまい、無効なポインタを返すことになってしまう。
// したがって、ここではfunctionAの結果をmallocでヒープ領域に確保したメモリに格納
// している。このような処理は、スレッドの戻り値を主スレッド側で取得するために必要
// な手段となる。
// ただし、このような方法を撮った場合、メモリリークを防ぐために、必ずスレッドの
// 戻り値をfreeする必要がある。このプログラムではmain関数内のpthread_join後で
// それを行っている。
void* functionA_wrapper(void* args)
{
	FunctionA_Args* func_args = (FunctionA_Args*)args;
	int result = functionA(func_args->val1, func_args->val2);

	int* res_ptr = (int*)malloc(sizeof(int));
	if (res_ptr == NULL)
	{
		perror("Failed to create thread for function A");
		printf("At %s:%d\n", __FILE__, __LINE__);
		return NULL;
	}

	pthread_cleanup_push(cleanup, res_ptr);

	*res_ptr = result;

	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);

	printf("Reached at the end of functionA_wrapper\n");
	pthread_cleanup_pop(0);

	return (void *)res_ptr;
}

void* timerThread(void * arg)
{
	pthread_t tidA = *(pthread_t *)arg;
	// printf("timerThread: %ld\n", tidA);
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 5;

	pthread_mutex_lock(&mutex);

	switch (pthread_cond_timedwait(&cond, &mutex, &ts))
	{
		case 0:
			printf("timerThread: Got signal\n");
			break;

		case ETIMEDOUT:
			pthread_cancel(tidA);
			printf("timerThread was cancelled due to timeout.\n");
			break;

		default:
			printf("timerThread: Error on pthread_cond_timedwait ");
			exit(1);
	}

	pthread_mutex_unlock(&mutex);

	return NULL;
}
	

int main()
{
	pthread_t tidA, tidTimer;
	printf("main1: %ld\n", tidA);
	FunctionA_Args args = {3, 7}; // 関数Aに渡す引数
	void *retval = NULL;

	if (pthread_create(&tidA, NULL, functionA_wrapper, &args) != 0)
	{
		perror("Failed to create thread for function A");
		printf("At %s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	printf("main2: %ld\n", tidA);
	if (pthread_create(&tidTimer, NULL, timerThread, &tidA) != 0)
	{
		perror("Failed to create timer thread");
		printf("At %s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	pthread_join(tidA, &retval);
	if (retval != NULL)
	{
		// printf("DEBUG: Function A returned: %d\n", *(int *)retval);
		printf("retval is not NULL\n");
		printf("pthread_join: %d\n", *(int *)retval);
		free(retval);
	}
	else
	{
		printf("retval is NULL\n");
	}
	// functionA_wrapper関数で動的に確保しためもりを、main関数で開放している。
	// free(retval); // 確保したメモリを開放

	pthread_join(tidTimer, NULL);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	printf("Exiting the program.\n");
	return 0;
}

