#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * 実行を同期したいケース
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
} Fly;

Fly flyList[MAX_FLY];

/*
 * ハエの状態を初期化
 */
void FlyInitCenter(Fly *fly, char mark_) {
    fly->mark = mark_;
    pthread_mutex_init(&fly->mutex, NULL);
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
    pthread_mutex_destroy(&fly->mutex);
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
    pthread_mutex_unlock(&fly->mutex);
    return 1;
}

/*
 * ハエを動かすスレッド
 */
void *doMove(void *arg) {
    Fly *fly = (Fly *)arg;
    while (!stopRequest) {
        /* 行先がセットされるのを待つ */
        fly->busy = 0;


