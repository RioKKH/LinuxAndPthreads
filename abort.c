#include <signal.h> // sigemptyset, sigaction
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// SIGSEGVシグナルハンドラ
void segfault_handler(int signal, siginfo_t *si, void *arg)
{
    printf("Caught SIGSEGV at address %p\n", si->si_addr);
    // ここで後処理等、必要な処理を行う

    // コアダンプを生成させる
    abort();
    // コアダンプは生成させたくなければ変わりにexit()する
    // exit(1);
}

int main()
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_handler;
    sa.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    // テストのために強制的にSIGSEGVを発生させる
    char *foo = "hello world";
    *foo = 'H';
}
