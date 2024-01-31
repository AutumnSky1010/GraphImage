#ifndef GRAPH_WRITER
#define GRAPH_WRITER
#include "parser.h"

// 画像の1ピクセルあたりの情報を表現する構造体
typedef struct pixel
{
    // 赤成分
    unsigned char R;
    // 緑成分
    unsigned char G;
    // 青成分
    unsigned char B;
} Pixel;

// graph_imageを初期化して返す
Pixel *init_graph_image();
// graph_imageを開放する。
void dispose_image(Pixel *graph_image);

// 与えられた式のグラフを指定色で描画する。
void draw_graph_expression(Pixel *graph_image, Pixel color, char *expression);
// 与えられた関数のグラフを指定色で描画する。
void draw_graph_func(Pixel *graph_image, Pixel color, Node *node, double (*f)(double x, Node *node));
// 座標軸を描画します。
void draw_axis(Pixel *graph_image);

// bmpとしてグラフを出力する。
void export_to_bmp(Pixel *graph_image, char *file_name);

#endif