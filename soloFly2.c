#include <pthread.h>
#include <unistd.h> // nanosleep
#include <stdio.h>  // printf
#include <stdlib.h> // strtod
#include <string.h> // strncmp
#include <math.h>   // atan2, cos, sin, sqrt

/**
 * 実行を同期したいケース
 * このプログラムの場合、スリープを使ってメインスレッドで値が入力されるのを
 * 待っているので、入力が完了してからハエが移動を開始するまでに最大で100msの
 * 遅延が発生する可能性がある。
 * かといって、スリープの時間を短くするとCPUの負荷が高くなるので、それはそれで
 * 問題になる。
 */

#define WIDTH 78  /* スクリーンの幅 */
#define HEIGHT 23 /* スクリーンの高さ */
#define MAX_FLY 1 /* 描画するハエの数 */
#define DRAW_CYCLE 50 /* 描画周期(ミリ秒) */

int stopRequest = 0; /* スレッド終了フラグ */

/*
 * ミリ秒単位でスリープする
 */
void mSleep(int msec) {
    struct timespec ts;
    ts.tv_sec = msec/1000;
    ts.tv_nsec = (msec%1000) * 1000000;
    nanosleep(&ts, NULL);
}

/*
 * 画面クリア
 */
void clearScreen() {
    /* このエスケープコードをターミナルに送ると画面がクリアされる */
    fputs("\033[2J", stdout);
}

/*
 * カーソル移動
 */
void moveCursor(int x, int y) {
    /* このエスケープコードをターミナルに送るとカーソルが(x, y)に移動する */
    printf("\033[%d;%dH", y, x);
}

/*
 * カーソル位置保存
 */
void saveCursor() {
    /* このエスケープコードをターミナルに送るとカーソル位置を記憶する */
    printf("\0337");
}

/*
 * カーソル位置復帰
 */
void restoreCursor() {
    /* このエスケープコードをターミナルに送ると記憶したカーソル位置に戻る */
    printf("\0338");
}

/*
 * ハエの構造体
 */
typedef struct {
    char mark;    /* 表示キャラクタ */
    double x, y;  /* 座標 */
    double angle; /* 移動方向(角度) */
    double speed; /* 移動速度 (ピクセル/秒) */
    double destX, destY; /* 目標地点 */
    int busy;     /* 移動中フラグ */
    pthread_mutex_t mutex;
    pthread_cond_t cond; /* 目標地点セット待ち */
} Fly;

Fly flyList[MAX_FLY];

/*
 * ハエの状態を初期化
 */
void FlyInitCenter(Fly *fly, char mark_) {
    fly->mark = mark_;
    pthread_mutex_init(&fly->mutex, NULL); /* ミューテックスの初期化 */
    pthread_cond_init(&fly->cond, NULL);   /* 条件変数の初期化 */
    fly->x = (double)WIDTH / 2.0;
    fly->y = (double)HEIGHT / 2.0;
    fly->angle = 0;
    fly->speed = 2;
    fly->destX = fly->x;
    fly->destY = fly->y;
    fly->busy = 0;
}

/*
 * ハエ構造体の利用終了
 */
void FlyDestroy(Fly *fly) {
    pthread_mutex_destroy(&fly->mutex); /* ミューテックスの破棄 */
    pthread_cond_destroy(&fly->cond); /* 条件変数の破棄 */
}

/*
 * ハエを移動する
 */
void FlyMove(Fly *fly) {
    int i;
    pthread_mutex_lock(&fly->mutex);
    fly->x += cos(fly->angle);
    fly->y += sin(fly->angle);
    pthread_mutex_unlock(&fly->mutex);
}

/*
 * ハエが指定座標にあるかどうか
 */
int FlyIsAt(Fly *fly, int x, int y) {
    int res;
    pthread_mutex_lock(&fly->mutex);
    res = ((int)fly->x == x) && ((int)fly->y == y);
    pthread_mutex_unlock(&fly->mutex);
    return res;
}

/*
 * 目標地点に合わせて移動方向と速度を調整する
 */
void FlySetDirection(Fly *fly) {
    pthread_mutex_lock(&fly->mutex);
    double dx = fly->destX - fly->x;
    double dy = fly->destY - fly->y;
    fly->angle = atan2(dy, dx);
    fly->speed = sqrt(dx*dx + dy*dy) / 5.0;
    if (fly->speed < 2) /* あまり遅すぎるとわかりづらいので */
        fly->speed = 2;
    pthread_mutex_unlock(&fly->mutex);
}

/*
 * 目標地点までの距離を得る
 */
double FlyDistanceToDestination(Fly *fly) {
    double dx, dy, res;
    pthread_mutex_lock(&fly->mutex);
    dx = fly->destX - fly->x;
    dy = fly->destY - fly->y;
    res = sqrt(dx*dx + dy*dy);
    pthread_mutex_unlock(&fly->mutex);
    return res;
}

/*
 * ハエの目標地点をセットする
 */
int FlySetDestination(Fly *fly, double x, double y) {
    /* 移動中はセット禁止 */
    if (fly->busy) return 0;
    pthread_mutex_lock(&fly->mutex);
    fly->destX = x;
    fly->destY = y;
    pthread_cond_signal(&fly->cond); /* 移動先セットを通知 */
    pthread_mutex_unlock(&fly->mutex);
    return 1;
}

/*
 * 目標地点がセットされるまで待つ
 */
void FlyWaitForSetDestination(Fly *fly) {
    pthread_mutex_lock(&fly->mutex);
    /*
     * pthread_cond_wait()を呼び出すと以下のような処理が行われる
     * 1. 条件変数をOFFにする
     * 2. ミューテックスをアンロックする
     * 3. 条件変数がONになるのを待ち続ける
     * 4. (条件変数がONになったら)ミューテックスをロックする
     */
    if (pthread_cond_wait(&fly->cond, &fly->mutex) != 0) {
        printf("Fatal error on pthread_cond_wait.\n");
        exit(1);
    }
    /* 条件変数がONになったら、ミューテックスがロックされている */
    pthread_mutex_unlock(&fly->mutex);
}

/*
 * ハエを動かすスレッド
 */
void *doMove(void *arg) {
    Fly *fly = (Fly *)arg;
    while (!stopRequest) {
        /* 行先がセットされるのを待つ */
        fly->busy = 0;
        FlyWaitForSetDestination(fly);
        if (FlyDistanceToDestination(fly) < 1) {
            continue;
        }
        /* condを導入することによって、以下のような無駄なループを回避できる
        while ((FlyDistanceToDestination(fly) < 1) && !stopRequest) {
            mSleep(100);
        }
        */
        fly->busy = 1;
        /* 目標地点の方向をセット */
        FlySetDirection(fly);
        /* 行き先に到着するまで移動する */
        while ((FlyDistanceToDestination(fly) >= 1) && !stopRequest) {
            FlyMove(fly);
            mSleep((int)(1000.0/fly->speed));
        }
    }
    return NULL;
}

/*
 * スクリーンを描画する
 */
void drawScreen() {
    int x, y;
    char ch;
    int i;

    saveCursor();
    moveCursor(0, 0);
    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            ch = 0;
            /* x, yの位置にあるハエがあればそのmarkを表示する */
            for (i = 0; i < MAX_FLY; i++) {
                if (FlyIsAt(&flyList[i], x, y)) {
                    ch = flyList[i].mark;
                    break;
                }
            }
            if (ch != 0) {
                putchar(ch);
            } else if ((y == 0) || (y == HEIGHT - 1)) {
                /* 上下の枠線を表示する */
                putchar('-');
            } else if ((x == 0) || (x == WIDTH - 1)) {
                /* 左右の枠線を表示する */
                putchar('|');
            } else {
                /* 枠線でもハエでもない */
                putchar(' ');
            }
        }
        putchar('\n');
    }
    restoreCursor();
    fflush(stdout);
}

/*
 * スクリーンを描画し続けるスレッド
 */
void *doDraw(void *arg) {
    while (!stopRequest) {
        drawScreen();
        mSleep(DRAW_CYCLE);
    }
    return NULL;
}

int main() {
    pthread_t drawThread;
    pthread_t moveThread;
    int i;
    char buf[40], *cp;
    double destX, destY;

    /* 初期化 */
    clearScreen();
    FlyInitCenter(&flyList[0], '@');

    /* ハエの動作処理 */
    pthread_create(&moveThread, NULL, doMove, (void *)&flyList[0]);

    /* 描画処理 */
    pthread_create(&drawThread, NULL, doDraw, NULL);

    /* メインスレッドは何か入力されるのを待ち、ハエの目標点をセットする */
    while(1) {
        printf("Destination? ");
        fflush(stdout);
        fgets(buf, sizeof(buf), stdin);
        if (strncmp(buf, "stop", 4) == 0) /* "stop"と入力するとプログラム終了 */
            break;
        /* 座標を読み取ってセットする */
        destX = strtod(buf, &cp);
        destY = strtod(cp, &cp);
        if (!FlySetDestination(&flyList[0], destX, destY)) {
            printf("The fly is busy now. Try later.\n");
        }
    }
    stopRequest = 1;

    /* スレッド撤収 */
    pthread_join(drawThread, NULL);
    pthread_join(moveThread, NULL);
    FlyDestroy(&flyList[0]);

    return 0;
}



