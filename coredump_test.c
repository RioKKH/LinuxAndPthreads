#include <stdio.h>
#include <signal.h>
#include <errno.h>

int main(void)
{
    int *p = NULL;
    printf("%d=n", *p);
}
