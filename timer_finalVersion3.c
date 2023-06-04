#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

typedef struct {
    int val1;
    int val2;
} FunctionA_Args;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int isFunctionACompleted = 0;

void cleanup(void *arg) {
    free(arg);
    printf("Cleanup handler was called. Memory was freed.\n");
}

// 関数Aの定義
int functionA(int val1, int val2) {
    // 関数Aの処理（仮にここではval1とval2を足し合わせる処理とします）
    int result = val1 + val2;
    for (int i = 0; i < 10; i++) {
        printf("Function A is running... %d\n", i);
        sleep(1);
    }
    return result;
}

// 関数Aのラッパー関数
void* functionA_wrapper(void* args) {
    FunctionA_Args* func_args = (FunctionA_Args*)args;
    int result = functionA(func_args->val1, func_args->val2);

    // 動的にメモリを確保して結果を返します
    int* res_ptr = (int*)malloc(sizeof(int));
    if (res_ptr == NULL) {
        perror("Failed to allocate memory for result");
        return NULL;
    }

    pthread_cleanup_push(cleanup, res_ptr);

    *res_ptr = result;

    pthread_mutex_lock(&mutex);
    isFunctionACompleted = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_cleanup_pop(0);

    return (void*)res_ptr;
}

void* timerThread(void* arg) {
    pthread_t tidA = *(pthread_t*)arg;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;

    pthread_mutex_lock(&mutex);
    while (!isFunctionACompleted) {
        if (pthread_cond_timedwait(&cond, &mutex, &ts) == ETIMEDOUT) {
            pthread_cancel(tidA);
            printf("Function A was cancelled due to timeout.\n");
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    pthread_t tidA, tidTimer;
    FunctionA_Args args = {3, 7};  // 関数Aに渡す引数
    void* retval;

    if (pthread_create(&tidA, NULL, functionA_wrapper, &args) != 0) {
        perror("Failed to create thread for function A");
        printf("At %s:%d\n", __FILE__, __LINE__);
        exit(1);
    }

    if (pthread_create(&tidTimer, NULL, timerThread, &tidA) != 0) {
        perror("Failed to create timer thread");
        printf("At %s:%d\n", __FILE__, __LINE__);
        exit(1);
    }

    pthread_join(tidA, &retval);
    if (retval != NULL) {
        printf("Function A returned: %d\n", *(int*)retval);
        free(retval);  // 確保したメモリを解放
    }

    pthread_join(tidTimer, NULL);

    printf("Exiting the program.\n");
    return 0;
}
