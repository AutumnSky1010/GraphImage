/**
 *
 *                グラフ描画ソフトウェア Ver 1.1.0-alpha
 *
 * 【概要】
 * 与えられた関数のグラフをBMP画像として出力します。
 *
 * 【ライセンス】
 *  MIT
 *
 * 【参考文献】
 * BMPファイルの構造
 * - http://coconut.sys.eng.shizuoka.ac.jp/bmp/
 * - https://qiita.com/spc_ehara/items/03d179f4901faeadb184
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "graph_writer.h"
#include "calclator.h"

// 画像の幅[ピクセル] 制約: 奇数
#define WIDTH 1001
// 画像の高さ[ピクセル] 制約: 奇数
#define HEIGHT 1001
// グラフ画像の中心X
#define CENTER_X 450
// グラフ画像の中心Y
#define CENTER_Y 0
// 最大のy座標
#define TOP (HEIGHT - 1) / 2 + CENTER_Y
// 最小のy座標
#define BOTTOM (-(HEIGHT - 1) / 2 + CENTER_Y)
// 最大のx座標
#define RIGHT (WIDTH - 1) / 2 + CENTER_X
// 最小のx座標
#define LEFT (-(WIDTH - 1) / 2 + CENTER_X)

// ファイルヘッダのサイズ
#define FILE_HEADER_SIZE 0x0e
#define INFO_HEADER_SIZE 0x28
// ピクセル毎のビット数(今回はフルカラーなので24 = 0x18ビット)
// 制約: 8の倍数
#define BIT_PER_PIXEL 0x18
// グラフの拡大率
#define MAGNIFICATION 150
// サンプリング数
#define SAMPLING_RATE 1000

// 点を表現する構造体
typedef struct point
{
    // x座標
    double X;
    // y座標
    double Y;
    // 次の点と連続かを表す真偽値
    bool IsContinue;
} Point;

// 線の太さを表す列挙型
typedef enum thickness
{
    bold,   // 太線
    normal, // 普通
} Thickness;

// 方向を表す列挙型
typedef enum direction
{
    top,
    bottom,
    right,
    left,
} Direction;

/* アプリケーションのライフサイクルに関する関数郡 */

/* 画像データ生成関連の関数郡 */
// 点を描画する。
void plot(Pixel *graph_image, Point, Pixel, Thickness);
// 2点間を結ぶ直線を指定したピクセルで描画する。
void draw_line(Pixel *graph_image, Point, Point, Pixel pixel, Thickness);
// 座標軸を描画する。
void draw_axis(Pixel *graph_image);
// 与えられた関数を用いて、点の集合をつくり、その先頭アドレスを返す。
Point *get_points(Node *node, double (*f)(double x, Node *node));
// 座標に対応する画像データのピクセルのポインタを返す。
Pixel *get_pixel(Pixel *graph_image, Point);
// 隣接したピクセルを取得する。
Pixel *get_adjacent_pixel(Pixel *graph_image, Pixel *base, Direction direction);

/* BMP画像関連の関数群 */
// BMP画像のファイルヘッダをファイルに書き込む。
void write_bmp_file_header(FILE *);
// BMP画像の情報ヘッダをファイルに書き込む。
void write_bmp_info_header(FILE *);
// BMP画像の画像データをファイルに書き込む。
void write_bmp_graph_image(FILE *, Pixel *graph_image);
// 画像データのサイズ[バイト]を計算する関数。
int calc_image_size();
// ファイルのサイズを計算する関数。
int calc_file_size();

// 与えられた二次元配列をすべて0で埋めます。
Pixel *init_graph_image()
{
    // ヒープ領域上に画像データを生成する。(auto変数はスタック領域に生成されるため)
    Pixel *graph_image = (Pixel *)calloc(HEIGHT * WIDTH, sizeof(Pixel));

    // 背景を白くする
    int i;
    Pixel *current = graph_image;
    for (i = 0; i < WIDTH * HEIGHT; i++)
    {
        current->R = 255;
        current->G = 255;
        current->B = 255;
        current++;
    }
    return graph_image;
}

void dispose_image(Pixel *graph_image)
{
    free(graph_image);
}

/**
 * ==================================================================
 *
 * 以下、グラフデータ生成関連の関数群(できることなら別ファイルにしたい)
 * 画像データに直接書き込む。
 *
 * ==================================================================
 */

// 座標軸を描画します。
void draw_axis(Pixel *graph_image)
{
    Pixel pixel = {192, 192, 192};
    Point west = {LEFT, 0};
    Point east = {RIGHT, 0};
    Point south = {0, BOTTOM};
    Point north = {0, TOP};
    // 格子を描画する(1*1の格子)
    int i = 0;
    while (LEFT <= -i || i <= RIGHT)
    {
        south.X = i;
        north.X = i;
        draw_line(graph_image, south, north, pixel, normal);
        south.X = -i;
        north.X = -i;
        draw_line(graph_image, south, north, pixel, normal);
        i += MAGNIFICATION;
    }
    i = 0;
    while (BOTTOM <= -i || i <= TOP)
    {
        west.Y = i;
        east.Y = i;
        draw_line(graph_image, west, east, pixel, normal);
        west.Y = -i;
        east.Y = -i;
        draw_line(graph_image, west, east, pixel, normal);
        i += MAGNIFICATION;
    }
    // 座標軸を描画する
    pixel.R = 128;
    pixel.G = 128;
    pixel.B = 128;
    west.Y = 0;
    east.Y = 0;
    south.X = 0;
    north.X = 0;
    draw_line(graph_image, west, east, pixel, bold);
    draw_line(graph_image, south, north, pixel, bold);
}

// 与えられた2点p1, p2間の直線を描画します。
void draw_line(Pixel *graph_image, Point p1, Point p2, Pixel pixel, Thickness thickness)
{
    // 差を求める
    int dx = p2.X - p1.X;
    int dy = p2.Y - p1.Y;

    // 繰り返しの回数を決めるために、差の絶対値を取って比較する。
    int dx_abs = abs(dx);
    int dy_abs = abs(dy);
    int count;
    // 値を代入していく最小の座標。2点で値が小さい方を代入する。
    int x = p1.X > p2.X ? p2.X : p1.X;
    int y = p1.Y > p2.Y ? p2.Y : p1.Y;
    // xの増加量とyの増加量を比較し、増加量が多い方を繰り返し回数とする。
    if (dx_abs > dy_abs)
    {
        count = dx_abs;
        // もし、始点のx座標が負の画面外の場合、始点を画面内にする。
        if (x < LEFT)
        {
            count -= abs(x - LEFT);
            x = LEFT;
        }
    }
    else
    {
        count = dy_abs;
        // もし、始点のy座標が負の画面外の場合、始点を画面内にする。
        if (y < BOTTOM)
        {
            count -= abs(y - BOTTOM);
            y = BOTTOM;
        }
    }

    int i;
    for (i = 0; i < count; i++)
    {
        Point point;
        if (dx_abs > dy_abs)
        {
            point.X = x + i;
            point.Y = (double)dy / (double)dx * (point.X - p1.X) + p1.Y;
            // 線が画面外に達した場合は処理を打ち切る
            if (point.X > RIGHT || point.X < LEFT)
            {
                break;
            }
        }
        else
        {
            point.Y = y + i;
            point.X = (double)dx / (double)dy * (point.Y - p1.Y) + p1.X;
            // 線が画面外に達した場合は処理を打ち切る
            if (point.Y > TOP || point.Y < BOTTOM)
            {
                break;
            }
        }

        plot(graph_image, point, pixel, thickness);
    }
}

// 点を描画します。
void plot(Pixel *graph_image, Point point, Pixel color, Thickness thickness)
{
    Pixel *pixel = get_pixel(graph_image, point);
    // 与えられた座標に対応するピクセルが存在場合はリターンする。
    if (pixel == NULL)
    {
        return;
    }
    *pixel = color;
    if (thickness == bold)
    {
        *(get_adjacent_pixel(graph_image, pixel, top)) = color;
        *(get_adjacent_pixel(graph_image, pixel, bottom)) = color;
        *(get_adjacent_pixel(graph_image, pixel, left)) = color;
        *(get_adjacent_pixel(graph_image, pixel, right)) = color;
    }
}

// 与えられた式のグラフを描画する関数。
void draw_graph_expression(Pixel *graph_image, Pixel color, char *expression)
{
    Token *token = lexical(expression);
    Node *node = parse(token);
    draw_graph_func(graph_image, color, node, calclate);
    dispose_tree(node);
}

// 数学的な関数を表現する関数を受け取り、グラフを描画する
void draw_graph_func(Pixel *graph_image, Pixel color, Node *node, double (*f)(double x, Node *node))
{
    Point *points = get_points(node, f);
    // メモリ解放用に、先頭アドレスを記憶する。
    Point *start = points;
    int i;
    // サンプリング数 + 左右両側(画面外)の点の数
    int count = SAMPLING_RATE + 2;
    Pixel pointPixel = {255, 255, 0};
    for (i = 0; i < count; i++)
    {
        if (i < count - 1 && points->IsContinue)
        {
            draw_line(graph_image, *points, *(points + 1), color, bold);
        }
        points++;
    }
    // メモリを開放する。
    free(start);
}

// 与えられた関数を用いて値を計算し、サンプリングレート+2個の座標配列を返します。(グラフが左右両側で途切れないようにするために、範囲外の点が２つ必要)
// nodeは与える関数によっては必須ではない。
// 注意：ヒープ領域上に配列を生成するので、使用後は必ずfree()でメモリを開放すること。
Point *get_points(Node *node, double (*f)(double x, Node *node))
{

    Point *points = (Point *)calloc(SAMPLING_RATE + 2, sizeof(Point));
    // 戻り値は先頭アドレスにしたいので、作業用ポインタ変数を生成している。
    Point *p = points;
    // 幅をレートで分割する
    double rate = WIDTH / (double)SAMPLING_RATE;
    // 外の点も計算したいので、レートを加算している。
    double x_max = RIGHT + rate;
    double x_min = LEFT - rate;
    int i;
    double x;
    for (i = 0; i < SAMPLING_RATE + 2; i++)
    {
        // x = Xの最小値 + 分割後一つ一つの幅 * i
        x = (x_min + rate * i);
        double next_x = x + rate;

        // xには、拡大率^-1を乗ずる必要がある。
        // y = f(x)をx軸方向、y軸方向にn倍拡大するには、
        // y = nf(x/n)として計算する必要があるから。
        double y = f(x / MAGNIFICATION, node);
        double next_y = f(next_x / MAGNIFICATION, node);

        // 次の点との高さの差が画像の高さより大きかった場合は不連続点として扱う。(暫定処理)
        bool is_continue = fabs(next_y * MAGNIFICATION - y * MAGNIFICATION) < HEIGHT;
        // 描画用に拡大して座標を保存する。描画時のx座標は拡大率をかけていない状態である必要がある。(あくまで、yの計算時の話だから)
        Point point = {x, y * MAGNIFICATION, is_continue};
        *p = point;
        p++;
    }
    return points;
}

// 与えられた点を表すピクセルを返します。もし存在していなければNULLを返します。
Pixel *get_pixel(Pixel *graph_image, Point point)
{
    // 指針：原点のインデクスを求めてから、引数pointの各座標を加算(yは-)し、それをもとにアドレスを計算する。
    int originXIndex = (WIDTH - 1) / 2 - CENTER_X;
    int originYIndex = (HEIGHT - 1) / 2 + CENTER_Y;
    int x_index = originXIndex + round(point.X);
    int y_index = originYIndex - round(point.Y);
    if (x_index >= WIDTH || y_index >= HEIGHT || x_index < 0 || y_index < 0)
    {
        return NULL;
    }

    return graph_image + x_index + y_index * WIDTH;
}

// 隣接したピクセルを取得する。
Pixel *get_adjacent_pixel(Pixel *graph_image, Pixel *base, Direction direction)
{
    Pixel *result = base;
    switch (direction)
    {
    case top:
        result = base - WIDTH;
        break;
    case bottom:
        result = base + WIDTH;
        break;
    case right:
        result = base + 1;
        break;
    case left:
        result = base - 1;
        break;
    default:
        break;
    }

    // baseが端だった場合取得できない場合がある。
    int diff = base - graph_image;
    // 左端かつ左を選択した場合baseを返す
    // 右端かつ右を選択した場合はbaseを返す... という条件でbaseを返す
    if ((diff % WIDTH == 0 && direction == left) ||
        diff % (WIDTH - 1) == 0 && direction == right ||
        diff / WIDTH == 0 && direction == top ||
        (WIDTH - 1) * HEIGHT <= diff && diff < WIDTH * HEIGHT)
    {
        return base;
    }
    return result;
}

/**
 * ==================================================================
 *
 * 以下、BMP画像関連の関数群(できることなら別ファイルにしたい)
 *
 * ==================================================================
 */

// 与えられた二次元データをもとに画像を出力します。
void export_to_bmp(Pixel *graph_image, char *file_name)
{
    strcat(file_name, ".bmp");
    FILE *fp = fopen(file_name, "wb");
    write_bmp_file_header(fp);
    write_bmp_info_header(fp);
    write_bmp_graph_image(fp, graph_image);
    fclose(fp);
}

// BMP画像のファイルヘッダを書き込みます。
void write_bmp_file_header(FILE *fp)
{
    // ファイルタイプ ("BM" = 0x42, 0x4d)
    char fileType[] = {0x42, 0x4d};
    fwrite(fileType, 1, 2, fp);

    // ファイルサイズ, 予約領域(常に0), 画像データまでのオフセット
    int header[] = {calc_file_size(), 0, FILE_HEADER_SIZE + INFO_HEADER_SIZE};
    fwrite(header, 4, 3, fp);
}

// BMP画像の情報ヘッダを書き込みます。
void write_bmp_info_header(FILE *fp)
{
    // ヘッダサイズ(常に0x28)、幅、高さ[ピクセル]
    int header0[] = {0x28, WIDTH, HEIGHT};

    // プレーン数(チャンネル数)(常に1)、ピクセル毎のビット数(今回はフルカラーなので24 = 0x18ビット)
    short header1[] = {1, 0x18};

    // 圧縮タイプ(無圧縮なので0)、イメージデータサイズ、水平解像度[ppm]、垂直解像度[ppm]、カラーインデックス数、重要インデックス
    int header2[] = {0, calc_image_size(), 1, 1, 0, 0};

    fwrite(header0, 4, 3, fp);
    fwrite(header1, 2, 2, fp);
    fwrite(header2, 4, 6, fp);
}

void write_bmp_graph_image(FILE *fp, Pixel *graph_image)
{
    // BMP画像データは左下の画素から右上の画素に向かって格納されている
    //   ∴  二重ループでポインタ演算をする必要がある
    int i, j;
    for (i = HEIGHT - 1; i >= 0; i--)
    {
        for (j = 0; j < WIDTH; j++)
        {
            // 二次元配列は、メモリ上では一次元のベクトルなので、先頭アドレス + i * 幅 * j
            Pixel *pixel = graph_image + i * WIDTH + j;
            fwrite(&pixel->B, 1, 1, fp);
            fwrite(&pixel->G, 1, 1, fp);
            fwrite(&pixel->R, 1, 1, fp);
        }

        // 不足分を0で埋める。

        int byte = BIT_PER_PIXEL / 8;
        // 4の倍数になるための不足分を計算する。
        // 例) 幅が33、フルカラー画像とする。 33[ピクセル] * 3[バイト] = 99では、99以上の4の倍数(100)になるためには1不足している。
        // この1を求めるには、4から99を4で割った余り(3)を引けば求められる。
        // よって、不足分の計算は、4 - 99 * 3 % 4 = 1
        // 変数で一般化すると、　4 - 幅 * 1ピクセルあたりのバイト数 % 4
        int shortage = 4 - WIDTH * byte % 4;
        if (shortage != 0)
        {
            char pad = 0;
            fwrite(&pad, 1, shortage, fp);
        }
    }
}

// 画像データそのもの(ヘッダなどを除く)のサイズ[バイト]を返します。
int calc_image_size()
{
    // 幅 * 1ピクセルあたりのバイト数が4の倍数でない場合、4の倍数になるように0で埋まるので、サイズの計算が単純でない。
    // よって、幅 * 1ピクセルあたりのバイト数 + 幅 * 1ピクセルあたりのバイト数以上かつ最小の4の倍数までの差 * 高さ * バイト数
    int byte = BIT_PER_PIXEL / 8;
    return (WIDTH * byte + 4 - WIDTH * byte % 4) * HEIGHT * byte;
}

// 画像「ファイル」のサイズを返します。
int calc_file_size()
{
    return FILE_HEADER_SIZE + INFO_HEADER_SIZE + calc_image_size();
}