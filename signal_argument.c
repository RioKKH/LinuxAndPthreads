#include <signal.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct my_sigaction_ {
    void (*handler)(int);
	void *arg;
} MY_SIGACTION;

void my_handler(int signal, void *arg)
{
	printf
	*(int *)arg = 20;
}

int main()
{
	MY_SIGACTION sa;
	sa.handler = (void *)my_handler;
	sa.arg = (void *)10;
	sigaction(SIGINT, (void *)&sa, NULL);
	// raise(SIGINT);
	printf("%d\n", *(int *)sa.arg);

	return 0;
}


