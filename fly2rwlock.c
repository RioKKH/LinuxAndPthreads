/**
 * read/writeロック
 * mutexとは MUTual EXecutionの意味
 * pthread_rwlock_init
 * pthread_rwlock_destroy
 *
 * rdlock 読み出しロック
 * pthread_rwlock_rdlock
 * pthread_rwlock_tryrdlock
 *
 * wrlock 書き込みロック
 * pthread_rwlock_wrlock
 * pthread_rwlock_trywrlock
 *
 * pthread_rwlock_unlock
 */

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define WIDTH 78 /*スクリーン幅*/
#define HEIGHT 23 // スクリーン高さ
#define MAX_FLY 6 // 描画するハエの数
const char *flyMarkList = "o@*+.#"; // ハエの描画文字一覧
#define DRAW_CYCLE 50 // 描画周期 ミリ秒
#define MIN_SPEED 1.0 // ハエの最低移動速度
#define MAX_SPEED 20.0 // ハエの最大移動速度

int stopRequest; // スレッド終了フラグ

/**
 * ミリ秒単位でスリープする
 */
void mSleep(int msec)
{
	struct timespec ts;
	ts.tv_sec = msec/1000;
	ts.tv_nsec = (msec%1000)*1000000;
	nanosleep(&ts, NULL);
}


/**
 * minValue以上maxValue未満のランダム値を得る
 */
double randDouble(double minValue, double maxValue)
{
	return minValue + (double)rand() / ((double)RAND_MAX+1) * (maxValue - minValue);
}

/**
 * 画面クリア
 */
void clearScreen()
{
	//- 以下のエスケープコードをターミナルに送ると画面がクリアされる
	fputs("\033[2J\033[1;1H", stdout);
}

/**
 * カーソル移動
 */
void moveCursor(int x, int y)
{
	//- このエスケープコードをターミナルに送ると、カーソル位置がx, yになる
	printf("\033[%d,%dH", y, x);
}

/**
 * ハエ構造体
 */
typedef struct {
	char mark; // 表示キャラクタ
	double x, y; // 座標
	double angle; // 移動方向(角度)
	double speed; // 移動速度(ピクセル/秒)
	pthread_rwlock_t rwlock; 
	// pthread_mutex_t mutex;
} Fly;


Fly flyList[MAX_FLY];

/**
 * ハエの状態をランダムに初期化
 */
void FlyInitRandom(Fly *fly, char mark_)
{
	fly->mark = mark_;
	pthread_rwlock_init(&fly->rwlock, NULL);
	// pthread_mutex_init(&fly->mutex, NULL);
	fly->x = randDouble(0, (double)(WIDTH-1));
	fly->y = randDouble(0, (double)(HEIGHT-1));
	fly->angle = randDouble(0, M_2_PI);
	fly->speed = randDouble(MIN_SPEED, MAX_SPEED);
}


/**
 * ハエ構造体の利用終了
 */
void FlyDestroy(Fly *fly)
{
	pthread_rwlock_destroy(&fly->rwlock);
	// pthread_mutex_destroy(&fly->mutex);
}

/**
 * ハエを移動する
 */
void FlyMove(Fly *fly)
{
	pthread_rwlock_wrlock(&fly->rwlock);
	// pthread_mutex_lock(&fly->mutex);
    fly->x += cos(fly->angle);
    fly->y += sin(fly->angle);
	//- X方向の縁にぶつかったら方向を変える
	if (fly->x < 0)
	{
		fly->x = 0;
		fly->angle = M_PI - fly->angle;
	}
	else if (fly->x > WIDTH - 1)
	{
		fly->x = WIDTH - 1;
		fly->angle = M_PI - fly->angle;
	}
	//- Y方向の縁にぶつかったら方向を変える
	if (fly->y < 0)
	{
		fly->y = 0;
		fly->angle = -fly->angle;
	}
	else if (fly->y > HEIGHT - 1)
	{
		fly->y = HEIGHT - 1;
		fly->angle = -fly->angle;
	}
	pthread_rwlock_unlock(&fly->rwlock);
	// pthread_mutex_unlock(&fly->mutex);
}

/**
 * ハエが指定座標にあるかどうか
 */
int FlyIsAt(Fly *fly, int x, int y)
{
	int res;
	pthread_rwlock_rdlock(&fly->rwlock);
	// pthread_mutex_lock(&fly->mutex);
	res = ((int)(fly->x) == x) && ((int)(fly->y) == y);
	pthread_rwlock_unlock(&fly->rwlock);
	// pthread_mutex_unlock(&fly->mutex);
	return res;
}


/**
 * ハエを動かし続けるスレッド
 */
void *doMove(void *arg)
{
	Fly *fly = (Fly *)arg;
	while(!stopRequest)
	{
		FlyMove(fly);
		mSleep((int)(1000.0 / fly->speed));
	}
	return NULL;
}

/**
 * スクリーンを描画する
 */
void drawScreen()
{
	int x, y;
	char ch;
	int i;

	moveCursor(0, 0);
	for (y = 0; y < HEIGHT; y++)
	{
		for (x = 0; x < WIDTH; x++)
		{
			ch = 0;
			//- x, yの位置にあるハエがあればそのmarkを表示する
			for (i = 0; i < MAX_FLY; i++)
			{
				if (FlyIsAt(&flyList[i], x, y))
				{
					ch = flyList[i].mark;
					break;
				}
			}
			if (ch != 0)
			{
				putchar(ch);
			}
			else if ((y == 0) || (y == HEIGHT - 1))
			{
				//- 上下の枠線を表示する
				putchar('-');
			}
			else if ((x == 0) || (x == WIDTH - 1))
			{
				//- 左右の枠線を表示する
				putchar('|');
			}
			else
			{
				//- 枠線でもハエでもない
                putchar(' ');
			}
		}
		putchar('\n');
	}
}


/**
 * スクリーンを描画し続けるスレッド
 */
void *doDraw(void *arg)
{
	while (!stopRequest)
	{
		clearScreen();
		drawScreen();
		mSleep(DRAW_CYCLE);
	}
	return NULL;
}


int main()
{
	pthread_t drawThread;
	pthread_t moveThread[MAX_FLY];
	int i;
	char buf[40];

	//- 初期化
	srand((unsigned int)time(NULL));
	clearScreen();
	for (i = 0; i < MAX_FLY; i++)
	{
		FlyInitRandom(&flyList[i], flyMarkList[i]);
	}

	//- ハエの動作処理
	for (i = 0; i < MAX_FLY; i++)
	{
		pthread_create(&moveThread[i], NULL, doMove, (void *)&flyList[i]);
	}

	//- 描画処理
	pthread_create(&drawThread, NULL, doDraw, NULL);

	//- メインスレッドは何か入力されるのを待つだけ
	fgets(buf, sizeof(buf), stdin);
	stopRequest = 1;

	//- スレッド撤収
	pthread_join(drawThread, NULL);
	for (i = 0; i < MAX_FLY; i++)
	{
		pthread_join(moveThread[i], NULL);
		FlyDestroy(&flyList[i]);
	}

	return 0;
}
