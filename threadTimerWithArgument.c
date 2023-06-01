#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


volatile int isFunctionACompleted = 0;
int* dynamicMemory;

// 複数の
int functionWithTwoArguments(int arg1, int arg2)
{
	printf("In functionWithTwoArguments\n");
	printf("argument1: %d\n", arg1);
	printf("argument2: %d\n", arg2);

	return 0;
}

// スレッド関数に渡すデータの構造体
typedef struct {
	int arg1;
	int arg2;
} ThreadData;


// ラッパー関数(引数と戻り値がvoid*)
void* threadFunction(void* arg) {
	// 引数をThreadData*にキャストする
	ThreadData* data = (ThreadData*)arg;

	int result = functionWithTwoArguments(data->arg1, data->arg2);

	// 結果をvoid*にキャストして返す
	// 注意：この方法は簡単のためのもので、ポータビリティや安全性は保証されない
	return (void*)(long)result;
}


int main()
{
	pthread_t thread;
	ThreadData data;
	data.arg1 = 123;
	data.arg2 = 456;

	pthread_create(&thread, NULL, threadFunction, &data);
	pthread_join(thread, NULL);

	return 0;
}
