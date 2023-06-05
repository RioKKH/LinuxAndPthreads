#include <bits/time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // sleep
#include <time.h>
#include <errno.h> // ETIMEDOUT


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void cleanup(void *arg)
{
	printf("Cleaning up and freeing memory\n");
	free(arg);
}

void *thread_func(void *arg)
{
	char *buffer = malloc(100 * sizeof(char));
	if (buffer == NULL)
	{
		perror("Failed to allocate memory");
		pthread_exit(NULL);
	}

	pthread_cleanup_push(cleanup, buffer);

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += 5;
	// timeout.tv_sec += 1;

	pthread_mutex_lock(&mutex);

	int result = pthread_cond_timedwait(&cond, &mutex, &timeout);
	if (result == ETIMEDOUT)
	{
		printf("Timeout occurred\n");
	}
	else
	{
		printf("Condition signaled\n");
	}

	pthread_mutex_unlock(&mutex);

	pthread_cleanup_pop(1);

	return NULL;
}

int main()
{
	pthread_t thread;
	int status = pthread_create(&thread, NULL, thread_func, NULL);
	if (status != 0)
	{
		perror("Failed to create thread");
		return 1;
	}

	sleep(2);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);

	pthread_join(thread, NULL);

	return 0;
}
