#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PRIME_NUMBERS 100000


/* 素数の一覧表。nPrimeNumber個が小さい順番に入っていく*/
int primeNumber[MAX_PRIME_NUMBERS];
int nPrimeNumber;

/* 素数かどうかを確認済みの最大の数 */
int primeNumberChecked;

pthread_mutex_t usingPrimeNumber;


/**
 * mが素数かどうかを確認する
 * m/2以下の素数がすべてprimeNumberテーブルに入っているものとする
 */
int isPrimeNumber(int m)
{
	int i;
	for (i = 0; i < nPrimeNumber; i++)
	{
		if (primeNumber[i] > m/2)
		{
			return 1; // m/2より大きい数で割り切れることはない
		}
		if (m % primeNumber[i] == 0)
		{
			return 0;
		}
	}
	return 1;
}

/**
 * n以下の素数をprimeNumberテーブルに入れる
 */
void generatePrimeNumbers(int n)
{
	int i;

	//- mutexをロックする
	pthread_mutex_lock(&usingPrimeNumber);

	if (n <= primeNumberChecked)
	// すでに素数一覧を作成済みなので何もすることはない
	{
		//- することがないので、ロックしたmutexをアンロックする
		pthread_mutex_unlock(&usingPrimeNumber);
		return;
	}
	// 足りない分の素数表を作成する
	for (i = primeNumberChecked+1; i <= n; i++)
	{
		if (isPrimeNumber(i))
		{
			if (nPrimeNumber >= MAX_PRIME_NUMBERS)
			{
				printf("Oops, too many prime nubers.\n");
				exit(1);
			}
			primeNumber[nPrimeNumber] = i;
			nPrimeNumber++;
			// printf("%d: %d is a prime number\n", nPrimeNumber, i);
		}
	}
	primeNumberChecked = n;
	pthread_mutex_unlock(&usingPrimeNumber);
}

/**
 * n以下の素数の個数を数える	
 */
int countPrimeNumbers(int n)
{
	int count, i;

	generatePrimeNumbers(n);
	/**
	 * primeNumberCheckedとnPrimeNumberは小さくなることがなく
	 * primeNumber[0...nPrimeNumber]は書き換わることがないので、
	 * この先は履いた制御する必要がない
	 */
	count = 0;
	for (i = 0; i < nPrimeNumber; i++)
	{
		if(primeNumber[i] > n)
		{
			break;
		}
		count++;
	}
	return count;
}


void *threadFunc(void *arg)
{
	int n = (int)arg;
	int x;

	x = countPrimeNumbers(n);
	printf("Number of prime numbers under %d is %d\n", n, x);

	return NULL;
}


int main()
{
	int numberList[6] = {1, 10, 100, 1000, 100000, 1000000};
	pthread_t threads[6];
	int i;

	pthread_mutex_init(&usingPrimeNumber, NULL);
	nPrimeNumber = 0;
	primeNumberChecked = 1;

	for (i = 0; i < 6; i++)
	{
		if (pthread_create(&threads[i], NULL, threadFunc, (void *)numberList[i]) != 0)
		{
			printf("Can't create thread (%d)\n", i);
			exit(1);
		}
	}

	for (i = 0; i < 6; i++)
	{
		pthread_join(threads[i], NULL);
	}

	pthread_mutex_destroy(&usingPrimeNumber);
	printf("Done\n");

	return 0;
}
