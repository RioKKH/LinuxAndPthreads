#include <asm-generic/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
	int val1;
	int val2;
	int ret;
} FunctionA_Args;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int isFunctionACompleted = 0;

// メモリクリンナップハンドラの定義
void cleanup(void *arg)
{
	printf("Cleanup hander was called. Memory was freed.\n");
	FunctionA_Args *func_args = (FunctionA_Args *)arg;
	func_args->ret = 777;
	printf("cleanup: %d\n", func_args->ret);
}

// 関数Aの定義
int functionA(int val1, int val2)
{ // 関数Aの処理 (仮にここではval1とval2を足し合わせる処理とする)
	int result = val1 + val2;
	// for (int i = 0; i < 1; i++)
	for (int i = 0; i < 10; i++)
	{
		printf("Function A is running... %d\n", i);
		sleep(1);
	}
	return result;
}

// 関数Aのラッパー関数
void* functionA_wrapper(void* args)
{
	FunctionA_Args* func_args = (FunctionA_Args*)args;

	// int* res_ptr = (int*)malloc(sizeof(int));
	// if (res_ptr == NULL)
	// {
	// 	perror("Failed to create thread for function A");
	// 	printf("At %s:%d\n", __FILE__, __LINE__);
	// 	return NULL;
	// }
	// int result;
	pthread_cleanup_push(cleanup, func_args);

	func_args->ret = functionA(func_args->val1, func_args->val2);
	// *res_ptr = result;
	printf("result: %d\n", func_args->ret);

	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);

	printf("Reached at the end of functionA_wrapper\n");
	pthread_cleanup_pop(0);

	// return (void *)res_ptr;
	return NULL;
}

void* timerThread(void * arg)
{
	pthread_t tidA = *(pthread_t *)arg;
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 3;

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
	pthread_t tidA;
	pthread_t tidTimer;
	FunctionA_Args args = {3, 7, 0}; // 関数Aに渡す引数
	void *retval;

	if (pthread_create(&tidA, NULL, functionA_wrapper, &args) != 0)
	{
		perror("Failed to create thread for function A");
		printf("At %s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	if (pthread_create(&tidTimer, NULL, timerThread, &tidA) != 0)
	{
		perror("Failed to create timer thread");
		printf("At %s:%d\n", __FILE__, __LINE__);
		exit(1);
	}

	pthread_join(tidA, NULL);
	printf("%d\n", args.ret);
	// pthread_join(tidA, &retval);
	/*
	if (retval == NULL)
	{
		printf("retval is NULL\n");
	}
	else
	{
		printf("retval is not NULL\n");
		// printf("DEBUG: Function A returned: %d\n", *(int *)retval);
		// int r = *((int *)retval);
		// printf("pthread_join: %d\n", r);
		// free(retval);
	}
    free(retval); // 確保したメモリを開放
	*/

	pthread_join(tidTimer, NULL);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	printf("Exiting the program.\n");
	return 0;
}

