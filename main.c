/**
 *
 *                グラフ描画ソフトウェア Ver 1.0.0-alpha
 *
 * 【概要】
 * 与えられた関数のグラフをBMP画像として出力します。
 *
 * 【プロ2の範囲を超える内容】
 * - ポインタ
 *      - 関数ポインタなど
 *      - 動的メモリ確保・開放(ヒープ領域・スタック領域・静的領域など)
 * - 構造体
 * - 各標準ライブラリ
 * - プリプロセッサ
 * - ファイル操作
 *      - バイナリファイルの扱いなど
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
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdbool.h>
#include <string.h>

// 画像の幅[ピクセル] 制約: 奇数
#define WIDTH 1001
// 画像の高さ[ピクセル] 制約: 奇数
#define HEIGHT 1001
// 最大のy座標
#define TOP (HEIGHT - 1) / 2
// 最大のx座標
#define RIGHT (WIDTH - 1) / 2
// ファイルヘッダのサイズ
#define FILE_HEADER_SIZE 0x0e
#define INFO_HEADER_SIZE 0x28
// ピクセル毎のビット数(今回はフルカラーなので24 = 0x18ビット)
// 制約: 8の倍数
#define BIT_PER_PIXEL 0x18
// グラフの拡大率
#define MAGNIFICATION 50
// サンプリング数
#define SAMPLING_RATE 1000

// 画像の1ピクセルあたりの情報を表現する構造体
typedef struct pixel
{
    // 赤成分
    unsigned char R;
    // 緑成分
    unsigned char G;
    // 青成分
    unsigned char B;
} PIXEL;

// 点を表現する構造体
typedef struct point
{
    // x座標
    double X;
    // y座標
    double Y;
    // 次の点と連続かを表す真偽値
    bool IsContinue;
} POINT;

// 数学的な関数の結果を表現する構造体
typedef struct funcResult
{
    // y座標
    double Y;
    // 次の点と連続かを表す真偽値
    bool IsContinue;
} FUNC_RESULT;
/* アプリケーションのライフサイクルに関する関数郡 */

// 画像データを初期化する。
void InitializeImageData(PIXEL *);

/* 画像データ生成関連の関数郡 */
// 点を描画する。
void Plot(PIXEL *imageData, POINT, PIXEL);
// 2点間を結ぶ直線を指定したピクセルで描画する。
void DrawLine(PIXEL *imageData, POINT, POINT, PIXEL pixel);
// 与えられた関数のグラフを描画する。
void DrawGraph(PIXEL *imageData, FUNC_RESULT(func)(double x, double nextX, int isInitial));
// 座標軸を描画する。
void DrawAxis(PIXEL *imageData);
// 与えられた関数を用いて、点の集合をつくり、その先頭アドレスを返す。
POINT *GetPoints(FUNC_RESULT(func)(double x, double nextX, int isInitial));
// 座標に対応する画像データのピクセルのポインタを返す。
PIXEL *GetPixel(PIXEL *imageData, POINT);

/* 数学的な関数を計算する関数郡 */
// 実数a, bを初期化する。
void InitializeAAndBAndC(double *a, double *b, double *c); // ユーティリティ関数
// 多項式関数
FUNC_RESULT Polynomial(double x, double nextX, int isInitial);
// 正接
FUNC_RESULT Tan(double x, double nextX, int isInitial);
// 余弦
FUNC_RESULT Cos(double x, double nextX, int isInitial);
// 正弦
FUNC_RESULT Sin(double x, double nextX, int isInitial);
// tan(x)における、直線を結べるかの条件を判定する関数
bool CanDrawLineTan(double x1, double x2, double b);

/* BMP画像関連の関数群 */
// 画像データをBMP画像として出力する関数。
void ExportToBMP(PIXEL *imageData, char *fileName);
// BMP画像のファイルヘッダをファイルに書き込む。
void WriteBMPFileHeader(FILE *);
// BMP画像の情報ヘッダをファイルに書き込む。
void WriteBMPInfoHeader(FILE *);
// BMP画像の画像データをファイルに書き込む。
void WriteBMPImageData(FILE *, PIXEL *imageData);
// 画像データのサイズ[バイト]を計算する関数。
int CalcImageSize();
// ファイルのサイズを計算する関数。
int CalcFileSize();

int main(void)
{
    // 数学的な関数の配列
    FUNC_RESULT(*funcs[4])
    (double x, double nextX, int isInitial) = {
        Polynomial,
        Sin,
        Cos,
        Tan};
    // ヒープ領域上に画像データを生成する。(auto変数はスタック領域に生成されるため)
    PIXEL *imageData = (PIXEL *)calloc(HEIGHT * WIDTH, sizeof(PIXEL));

    printf("グラフ画像出力ソフトウェア\n");
    bool isAppEnd = false;
    do
    {
        InitializeImageData(imageData);
        DrawAxis(imageData);
        
        char fileName[51];
        do 
        {
            printf("ファイル名を入力してください。(20字以内)\n");
            scanf("%s", fileName);
        } while(strlen(fileName) > 20);

        int input;
        bool isInputFuncsEnd = false;
        do
        {
            printf("描画したい関数を選び、番号を入力してください。\n存在しない番号を入力すると、終了します。\n");
            printf("多項式関数: 0\nsin x: 1\ncos x: 2\ntan x: 3\n");
            scanf("%d", &input);
            if (0 <= input && input <= 3)
            {
                DrawGraph(imageData, funcs[input]);
            }
            else
            {
                isInputFuncsEnd = true;
            }
            printf("\n");
        } while (!isInputFuncsEnd);
        ExportToBMP(imageData, fileName);
        printf("%sを出力しました。\n", fileName);
        printf("終了する場合: 0\n別の画像を出力する場合: 0以外\nを入力してください。\n");
        scanf("%d", &input);
        if (input == 0)
        {
            isAppEnd = true;
        }

    } while (!isAppEnd);

    free(imageData);
    return 0;
}

// 与えられた二次元配列をすべて0で埋めます。
void InitializeImageData(PIXEL *imageData)
{
    int i;

    for (i = 0; i < HEIGHT * WIDTH; i++)
    {
        PIXEL pixel;
        pixel.R = 0;
        pixel.G = 0;
        pixel.B = 0;
        *imageData = pixel;
        imageData++;
    }
}

/**
 * ==================================================================
 *
 * 以下、グラフデータ生成関連の関数群(できることなら別ファイルにしたい)
 * 画像データをを直接書き込む。
 *
 * ==================================================================
 */

// 座標軸を描画します。
void DrawAxis(PIXEL *imageData)
{
    PIXEL pixel = {255, 0, 0};
    double xMax = (WIDTH - 1) / 2;
    double yMax = (HEIGHT - 1) / 2;

    POINT west = {-xMax, 0};
    POINT east = {xMax, 0};
    DrawLine(imageData, west, east, pixel);

    POINT south = {0, -yMax};
    POINT north = {0, yMax};
    DrawLine(imageData, south, north, pixel);
}

// 与えられた2点p1, p2間の直線を描画します。
void DrawLine(PIXEL *imageData, POINT p1, POINT p2, PIXEL pixel)
{
    // 差を求める
    int dx = p2.X - p1.X;
    int dy = p2.Y - p1.Y;

    // 繰り返しの回数を決めるために、差の絶対値を取って比較する。
    int dxAbs = abs(dx);
    int dyAbs = abs(dy);
    int count;
    // 値を代入していく最小の座標。2点で値が小さい方を代入する。
    int x = p1.X > p2.X ? p2.X : p1.X;
    int y = p1.Y > p2.Y ? p2.Y : p1.Y;
    // xの増加量とyの増加量を比較し、増加量が多い方を繰り返し回数とする。
    if (dxAbs > dyAbs)
    {
        count = dxAbs;
        // もし、始点のx座標が負の画面外の場合、始点を画面内にする。
        if (x < -RIGHT)
        {
            count -= abs(x  + RIGHT);
            x = -RIGHT;
        }
    }
    else
    {
        count = dyAbs;
        // もし、始点のy座標が負の画面外の場合、始点を画面内にする。
        if (y < -TOP)
        {
            count -= abs(y  + TOP);
            y = -TOP;
        }
    }

    int i;
    for (i = 0; i < count; i++)
    {
        POINT point;
        if (dxAbs > dyAbs)
        {
            point.X = x + i;
            point.Y = (double)dy / (double)dx * (point.X - p1.X) + p1.Y;
            // 線が画面外に達した場合は処理を打ち切る
            if (point.X > RIGHT || point.X < -RIGHT)
            {
                break;
            }
        }
        else
        {
            point.Y = y + i;
            point.X = (double)dx / (double)dy * (point.Y - p1.Y) + p1.X;
            // 線が画面外に達した場合は処理を打ち切る
            if (point.Y > TOP || point.Y < -TOP)
            {
                break;
            }
        }

        Plot(imageData, point, pixel);
    }
}

// 点を描画します。
void Plot(PIXEL *imageData, POINT point, PIXEL color)
{
    PIXEL *pixel = GetPixel(imageData, point);
    // 与えられた座標に対応するピクセルが存在場合はリターンする。
    if (pixel == NULL)
    {
        return;
    }
    *pixel = color;
}

// 数学的な関数の役割をする関数と画像データを受け取り、描画します。
void DrawGraph(PIXEL *imageData, FUNC_RESULT(func)(double x, double nextX, int isInitial))
{
    POINT *points = GetPoints(func);
    // メモリ解放用に、先頭アドレスを記憶する。
    POINT *start = points;
    int i;
    // サンプリング数 + 左右両側(画面外)の点の数
    int count = SAMPLING_RATE + 2;
    PIXEL pointPixel = {255, 255, 0};
    PIXEL linePixel = {0, 255, 0};
    for (i = 0; i < count; i++)
    {

        if (i < count - 1 && points->IsContinue)
        {
            DrawLine(imageData, *points, *(points + 1), linePixel);
        }
        Plot(imageData, *points, pointPixel);
        points++;
    }
    // メモリを開放する。
    free(start);
}

// 与えられた関数を用いて値を計算し、サンプリングレート+2個の座標配列を返します。(グラフが左右両側で途切れないようにするために、範囲外の点が２つ必要)
// 注意：ヒープ領域上に配列を生成するので、使用後は必ずfree()でメモリを開放すること。
POINT *GetPoints(FUNC_RESULT(func)(double x, double nextX, int isInitial))
{
    POINT *points = (POINT *)calloc(SAMPLING_RATE + 2, sizeof(POINT));
    // 戻り値は先頭アドレスにしたいので、作業用ポインタ変数を生成している。
    POINT *p = points;
    // 幅をレートで分割する
    double rate = WIDTH / (double)SAMPLING_RATE;
    // 外の点も計算したいので、レートを加算している。
    double XMax = RIGHT + rate;
    double XMin = -XMax;
    int i;
    int isInitial = 1;
    double x, nextX;
    for (i = 0; i < SAMPLING_RATE + 2; i++)
    {
        // x = Xの最小値 + 分割後一つ一つの幅 * i
        x = (XMin + rate * i);
        nextX = x + rate;

        // xには、拡大率^-1を乗ずる必要がある。
        // y = f(x)をx軸方向、y軸方向にn倍拡大するには、
        // y = nf(x/n)として計算する必要があるから。
        FUNC_RESULT result = func(x / MAGNIFICATION, nextX / MAGNIFICATION, isInitial);
        // 描画用に拡大して座標を保存する。描画時のx座標は拡大率をかけていない状態である必要がある。(あくまで、yの計算時の話だから)
        POINT point = {x, result.Y * MAGNIFICATION, result.IsContinue};
        *p = point;

        // i = 0のとき、関数を初期化済みなので、isInitialを0にする。
        if (i == 0)
        {
            isInitial = 0;
        }
        p++;
    }
    return points;
}

// 与えられた点を表すピクセルを返します。もし存在していなければNULLを返します。
PIXEL *GetPixel(PIXEL *imageData, POINT point)
{
    // 指針：原点のインデクスを求めてから、引数pointの各座標を加算(yは-)し、それをもとにアドレスを計算する。
    int originXIndex = (WIDTH - 1) / 2;
    int originYIndex = (HEIGHT - 1) / 2;
    int xIndex = originXIndex + round(point.X);
    int yIndex = originYIndex - round(point.Y);
    if (xIndex >= WIDTH || yIndex >= HEIGHT || xIndex < 0 || yIndex < 0)
    {
        return NULL;
    }

    return imageData + xIndex + yIndex * WIDTH;
}

/**
 * ==================================================================
 *
 * 以下、数学的な関数を計算する関数群(できることなら別ファイルにしたい)
 *
 * ==================================================================
 */

// 多項式関数
FUNC_RESULT Polynomial(double x, double nextX, int isInitial)
{
    // 昇べきの順で各項の係数を持つ配列。ax^0 + bx^1 + ...... + tx^20
    // とりあえず20次式まで対応している。
    static double coefficients[21];
    // n次式のn
    static int n;
    int i;
    if (isInitial)
    {
        printf("n次の多項式 (0 <= n <= 20)\n");
        printf("n = ");
        scanf("%d", &n);

        for (i = 0; i < n + 1; i++)
        {
            printf("x^%dの係数: ", i);
            scanf("%lf", &coefficients[i]);
        }
    }
    // x^0の係数(定数項)を代入
    double y = coefficients[0];
    // 定数だけじゃないかつ、xが0じゃない場合
    if (n != 0 && x != 0)
    {
        int j;
        for (i = 1; i < n + 1; i++)
        {
            // 単項の値を保持する。
            double unary = coefficients[i];
            if (unary == 0)
            {
                continue;
            }
            for (j = 0; j < i; j++)
            {
                unary *= x;
            }
            y += unary;
        }
    }
    FUNC_RESULT result = {y, true};
    return result;
}

// 正弦
FUNC_RESULT Sin(double x, double nextX, int isInitial)
{
    static double a, b, c;
    if (isInitial)
    {
        printf("asin(bx) + c\n");
        InitializeAAndBAndC(&a, &b, &c);
    }
    double y = a * sin(b * x) + c;
    FUNC_RESULT result = {y, true};
    return result;
}

// 余弦
FUNC_RESULT Cos(double x, double nextX, int isInitial)
{
    static double a, b, c;
    if (isInitial)
    {
        printf("acos(bx) + c\n");
        InitializeAAndBAndC(&a, &b, &c);
    }
    double y = a * cos(b * x) + c;
    FUNC_RESULT result = {y, true};
    return result;
}

// 正接
FUNC_RESULT Tan(double x, double nextX, int isInitial)
{
    static double a, b, c;
    if (isInitial)
    {
        printf("atan(bx) + c\n");
        InitializeAAndBAndC(&a, &b, &c);
    }

    double y = a * tan(b * x) + c;
    int isContinue = CanDrawLineTan(x, nextX, b);
    FUNC_RESULT result = {y, isContinue};
    return result;
}

// tan(x)用の、2点間で直線を引けるかを判定する関数。
bool CanDrawLineTan(double x1, double x2, double b)
{
    // 絶対値をとる
    x1 = fabs(x1);
    x2 = fabs(x2);

    double maxX, minX;
    if (x1 > x2)
    {
        maxX = x1;
        minX = x2;
    }
    else
    {
        maxX = x2;
        minX = x1;
    }
    // minX <= d <= maxX のとき、dが pi/2bで割り切れる かつ pi/b で割り切れないとき、直線を結べない。
    // よって、閉区間[minX, maxX]内に、pi/2bで割り切れる かつ pi/b で割り切れない実数dがあるかを調べる必要がある。
    double pi2Mod = fmod(minX, M_PI_2 / b);
    double piMod = fmod(minX, M_PI / b);
    // 最小のxがpi/2bで割り切れ、pi/bで割り切れない時は戻す。
    if (pi2Mod == 0 && piMod != 0)
    {
        return 0;
    }
    // 最小のx以上のpi/2の倍数、piの倍数を求める。
    double d1 = minX + M_PI_2 / b - pi2Mod;
    double d2 = minX + M_PI / b - piMod;
    return d1 == d2 || d1 > maxX;
}

// 実数の定数a,bを初期化する。
void InitializeAAndBAndC(double *a, double *b, double *c)
{
    printf("実数の定数a, b, cを初期化します。\na = ");
    scanf("%lf", a);
    printf("b = ");
    scanf("%lf", b);
    printf("c = ");
    scanf("%lf", c);
}

/**
 * ==================================================================
 *
 * 以下、BMP画像関連の関数群(できることなら別ファイルにしたい)
 *
 * ==================================================================
 */

// 与えられた二次元データをもとに画像を出力します。
void ExportToBMP(PIXEL *imageData, char *fileName)
{
    strcat(fileName, ".bmp");
    FILE *fp = fopen(fileName, "wb");
    WriteBMPFileHeader(fp);
    WriteBMPInfoHeader(fp);
    WriteBMPImageData(fp, imageData);
    fclose(fp);
}

// BMP画像のファイルヘッダを書き込みます。
void WriteBMPFileHeader(FILE *fp)
{
    // ファイルタイプ ("BM" = 0x42, 0x4d)
    char fileType[] = {0x42, 0x4d};
    fwrite(fileType, 1, 2, fp);

    // ファイルサイズ, 予約領域(常に0), 画像データまでのオフセット
    int header[] = {CalcFileSize(), 0, FILE_HEADER_SIZE + INFO_HEADER_SIZE};
    fwrite(header, 4, 3, fp);
}

// BMP画像の情報ヘッダを書き込みます。
void WriteBMPInfoHeader(FILE *fp)
{
    // ヘッダサイズ(常に0x28)、幅、高さ[ピクセル]
    int header0[] = {0x28, WIDTH, HEIGHT};

    // プレーン数(チャンネル数)(常に1)、ピクセル毎のビット数(今回はフルカラーなので24 = 0x18ビット)
    short header1[] = {1, 0x18};

    // 圧縮タイプ(無圧縮なので0)、イメージデータサイズ、水平解像度[ppm]、垂直解像度[ppm]、カラーインデックス数、重要インデックス
    int header2[] = {0, CalcImageSize(), 1, 1, 0, 0};

    fwrite(header0, 4, 3, fp);
    fwrite(header1, 2, 2, fp);
    fwrite(header2, 4, 6, fp);
}

void WriteBMPImageData(FILE *fp, PIXEL *imageData)
{
    // BMP画像データは左下の画素から右上の画素に向かって格納されている
    //   ∴  二重ループでポインタ演算をする必要がある
    int i, j;
    for (i = HEIGHT - 1; i >= 0; i--)
    {
        for (j = 0; j < WIDTH; j++)
        {
            // 二次元配列は、メモリ上では一次元のベクトルなので、先頭アドレス + i * 幅 * j
            PIXEL *pixel = imageData + i * WIDTH + j;
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
int CalcImageSize()
{
    // 幅 * 1ピクセルあたりのバイト数が4の倍数でない場合、4の倍数になるように0で埋まるので、サイズの計算が単純でない。
    // よって、幅 * 1ピクセルあたりのバイト数 + 幅 * 1ピクセルあたりのバイト数以上かつ最小の4の倍数までの差 * 高さ * バイト数
    int byte = BIT_PER_PIXEL / 8;
    return (WIDTH * byte + 4 - WIDTH * byte % 4) * HEIGHT * byte;
}

// 画像「ファイル」のサイズを返します。
int CalcFileSize()
{
    return FILE_HEADER_SIZE + INFO_HEADER_SIZE + CalcImageSize();
}