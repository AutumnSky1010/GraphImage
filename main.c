#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define MAX_ITER_COUNT 100
#include "graph_writer.h"
#include "parser.h"
#include "lexer.h"
#include "calclator.h"

typedef enum mode
{
    newton,
    draw_graph_mode,
} Mode;
// テキストファイルに記述した関数のグラフを描画する
void draw_graph();

// ニュートン法を実行してグラフを出力する
double newton_method();
// 関数
double f(double x, Node *node);
// 微分係数を計算する
double dxdy(double x, Node *node);
// 接線の式
double tangent_line(double x, Node *node);

int main(void)
{
    Mode mode;
    printf("グラフ描画&ニュートン法シミュレータ\n");
    printf("モードを選んでください。\n%d: ニュートン法シミュレータ\n%d: 関数グラフ描画\n", newton, draw_graph_mode);
    scanf("%d", &mode);
    switch (mode)
    {
    case newton:
        newton_method();
        break;
    case draw_graph_mode:
        draw_graph();
        break;
    default:
        break;
    }
    return 0;
}

// 接線の方程式を計算するためににグローバル変数にしている。
double xk;

double newton_method()
{
    // ファイルから初期値、関数、導関数を読み込む
    char *function_file_name = "newton_funcs.txt";
    FILE *fp = fopen(function_file_name, "r");
    if (fp == NULL)
    {
        perror("ファイルを開けませんでした。\n");
        printf("ファイル名: %s\n", function_file_name);
        exit(-1);
    }

    char expression[255];
    Token *tokens;
    Node *dxdy_node, *f_node;
    // 初期値
    double x0;
    // 初期値を読み込む
    fscanf(fp, "%lf", &x0);
    // 関数の式を読み込む
    fscanf(fp, "%s", expression);
    tokens = lexical(expression);
    f_node = parse(tokens);
    // 導関数の式を読み込む
    fscanf(fp, "%s", expression);
    tokens = lexical(expression);
    dxdy_node = parse(tokens);
    // 接線描画用に構造体をコピーして配列にする(引数がNodeのポインタに制限されているので無理やり渡すには配列にするしかない)
    Node nodes_for_tangent_line[2];
    nodes_for_tangent_line[0] = *f_node;
    nodes_for_tangent_line[1] = *dxdy_node;

    // 許容誤差
    double eps = 1.0e-10;
    int i;
    xk = x0;
    Pixel *graph_image = init_graph_image();
    draw_axis(graph_image);
    Pixel color = {0, 0, 0};
    draw_graph_func(graph_image, color, f_node, f);
    srand(time(NULL));
    for (i = 0; i < MAX_ITER_COUNT; i++)
    {
        color.R = rand() % 256;
        color.G = rand() % 256;
        color.B = rand() % 256;

        xk = xk - f(xk, f_node) / dxdy(xk, dxdy_node);
        draw_graph_func(graph_image, color, nodes_for_tangent_line, tangent_line);
        printf("%d反復: %.16f\n", i + 1, xk);
        char fileName[51];
        sprintf(fileName, "%s/%d-%s", "newton_method_images", i + 1, "newton_method");
        export_to_bmp(graph_image, fileName);
        if (fabs(f(xk, f_node)) < eps)
        {
            printf("%d反復で近似解: %.16f\n", i + 1, xk);
            return xk;
        }
    }
    printf("%d反復では収束しませんでした。\n", MAX_ITER_COUNT);
}

double dxdy(double x, Node *node)
{
    return calclate(x, node);
}

double f(double x, Node *node)
{
    return calclate(x, node);
}

// f(x)計算用nodeとf'(x)計算用nodeを渡す
// f(x) -> f'(x)という順でノードを渡す
double tangent_line(double x, Node *node)
{
    return dxdy(xk, node + 1) * (x - xk) + f(xk, node);
}

void draw_graph()
{
    char *function_file_name = "graphs.txt";
    FILE *fp = fopen(function_file_name, "r");
    if (fp == NULL)
    {
        perror("ファイルを開けませんでした。\n");
        printf("ファイル名: %s\n", function_file_name);
        exit(-1);
    }

    char *file_name = (char *)calloc(51, sizeof(char));
    fscanf(fp, "%s", file_name);
    if (strlen(file_name) > 20)
    {
        printf("出力ファイル名は20文字までにしてください。\n");
        exit(-1);
    }
    printf("%s.bmpにグラフを書き込みます。\n", file_name);
    Pixel color;
    char expression[255];

    Pixel *graph_image = init_graph_image();
    draw_axis(graph_image);
    while (fscanf(fp, "%s %d %d %d", expression, &color.R, &color.G, &color.B) != EOF)
    {
        draw_graph_expression(graph_image, color, expression);
    }

    export_to_bmp(graph_image, file_name);

    printf("%sを出力しました。\n", file_name);
    getchar();
    getchar();
    dispose_image(graph_image);
    free(file_name);

    fclose(fp);
}