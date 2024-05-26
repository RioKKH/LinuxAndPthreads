#include <signal.h> // sigemptyset, sigaction
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
	int a;
	int b;
} DATA;

// SIGSEGVシグナルハンドラ
void segfault_handler(int signal, siginfo_t *si, void *arg)
{

	printf("### segfault_handler ###\n");
    printf("Caught SIGSEGV at address %p\n", si->si_addr);
    // ここで後処理等、必要な処理を行う

    // コアダンプを生成させる
    abort();
    // コアダンプは生成させたくなければ変わりにexit()する
    // exit(1);
}

void functionA(void)
{
	printf("### functionA ###\n");
    // テストのために強制的にSIGSEGVを発生させる
    char *foo = "hello world";
    *foo = 'H';
}


void wrapper_functionA(void)
{
	DATA data = {1, 2};
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_handler;
    sa.sa_flags = SA_SIGINFO;
	sa.arg = (void *)data;

    if (sigaction(SIGSEGV, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

	printf("### wrapper_functionA ###\n");
	functionA();
	printf("### End of wrapper_functionA ###\n");
}

int main()
{
	printf("### main ###\n");
	wrapper_functionA();

	return 0;
}
