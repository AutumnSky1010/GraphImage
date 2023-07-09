/*

【参考文献】
BMPファイルの構造
http://coconut.sys.eng.shizuoka.ac.jp/bmp/
https://qiita.com/spc_ehara/items/03d179f4901faeadb184
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// 画像の幅[ピクセル] 制約: 奇数
#define WIDTH 1001
// 画像の高さ[ピクセル] 制約: 奇数
#define HEIGHT 1001
// ファイルヘッダのサイズ
#define FILE_HEADER_SIZE 0x0e
#define INFO_HEADER_SIZE 0x28
// ピクセル毎のビット数(今回はフルカラーなので24 = 0x18ビット)
// 制約: 8の倍数
#define BIT_PER_PIXEL 0x18

typedef struct pixel
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
} PIXEL;

typedef struct point
{
    double X;
    double Y;
} POINT;


/*画像データ関連の関数群*/
void InitializeImageData(PIXEL *);

/*グラフ生成関連の関数郡*/
void Plot(PIXEL *);
void DrawLine(PIXEL *, POINT, POINT);
void DrawAxis(PIXEL *);
PIXEL *GetPixel(PIXEL *, POINT);

/*BMP画像関連の関数群*/
void ExportToBMP(PIXEL *);
void WriteBMPFileHeader(FILE *);
void WriteBMPInfoHeader(FILE *);
void WriteBMPImageData(FILE *, PIXEL *);
int CalcImageSize();
int CalcFileSize();

int main(void) 
{
    PIXEL *imageData = (PIXEL *)calloc(HEIGHT * WIDTH, sizeof(PIXEL));
    InitializeImageData(imageData);
    DrawAxis(imageData);
    Plot(imageData);
    ExportToBMP(imageData);
    free(imageData);
    return 0;
}

// 与えられた二次元配列をすべて0で埋めます。
void InitializeImageData(PIXEL *imageData) 
{
    int i;
    
    for (i = 0; i < HEIGHT * WIDTH; i++) {
        PIXEL pixel;
        pixel.R = 0;
        pixel.G = 0;
        pixel.B = 0;
        *imageData = pixel;
        imageData++;
    }
}

void DrawAxis(PIXEL *imageData) 
{
    int i;
    PIXEL pixel;
    pixel.R = 255;
    pixel.G = 0;
    pixel.B = 0;
    for (i = 0; i < WIDTH; i++) {
        *(imageData + (HEIGHT - 1)/2 * WIDTH + i) = pixel;
    }
    for (i = 0; i < HEIGHT; i++) {
        *(imageData + (WIDTH - 1)/2 + HEIGHT * i) = pixel;
    }
}

void DrawLine(PIXEL *imageData, POINT p1, POINT p2) 
{

}

void Plot(PIXEL *imageData)
{
    double x, y;
    for (x = -(WIDTH - 1)/2; x < (WIDTH - 1)/2; x+=0.01) {
        y = 100*sin(x/100);
        POINT point;
        point.X = x;
        point.Y = y;
        PIXEL *pixel = GetPixel(imageData, point);
        if (pixel == NULL) {
            continue;
        }
        pixel->R = 255;
        pixel->G = 255;
        pixel->B = 0;
    }
}

// 与えられた点を表すピクセルを返します。もし存在していなければNULLを返します。
PIXEL *GetPixel(PIXEL *imageData, POINT point) 
{
    // 指針：原点のインデクスを求めてから、引数pointの各座標を加算(yは-)し、それをもとにアドレスを計算する。
    int originXIndex = (WIDTH - 1) / 2;
    int originYIndex = (HEIGHT - 1) / 2;
    int xIndex = round(originXIndex + point.X);
    int yIndex = round(originYIndex - point.Y);
    if (xIndex >= WIDTH || yIndex >= HEIGHT || xIndex < 0 || yIndex < 0) {
        return NULL;
    }

    return imageData + xIndex + yIndex * WIDTH;
}

// 与えられた二次元データをもとに画像を出力します。
void ExportToBMP(PIXEL *imageData) 
{
    FILE *fp = fopen("test.bmp", "wb");
    WriteBMPFileHeader(fp);
    WriteBMPInfoHeader(fp);
    WriteBMPImageData(fp, imageData);
    fclose(fp);
}

// BMP画像のファイルヘッダを書き込みます。
void WriteBMPFileHeader(FILE *fp) 
{
    // ファイルタイプ ("BM" = 0x42, 0x4d)
    char fileType[] = { 0x42, 0x4d };
    fwrite(fileType, 1, 2, fp);

    // ファイルサイズ, 予約領域(常に0), 画像データまでのオフセット
    int header[] = { CalcFileSize(), 0, FILE_HEADER_SIZE + INFO_HEADER_SIZE };
    fwrite(header, 4, 3, fp);
}

// BMP画像の情報ヘッダを書き込みます。
void WriteBMPInfoHeader(FILE *fp) 
{
    // ヘッダサイズ(常に0x28)、幅、高さ[ピクセル]
    int header0[] = { 0x28, WIDTH, HEIGHT };

    // プレーン数(チャンネル数)(常に1)、ピクセル毎のビット数(今回はフルカラーなので24 = 0x18ビット)
    short header1[] = { 1, 0x18 };

    // 圧縮タイプ(無圧縮なので0)、イメージデータサイズ、水平解像度[ppm]、垂直解像度[ppm]、カラーインデックス数、重要インデックス
    int header2[] = { 0, CalcImageSize(), 1, 1, 0, 0 };

    fwrite(header0, 4, 3, fp);
    fwrite(header1, 2, 2, fp);
    fwrite(header2, 4, 6, fp);
}

void WriteBMPImageData(FILE *fp, PIXEL *imageData)
{
    // BMP画像データは左下の画素から右上の画素に向かって格納されている
    //   ∴  二重ループでポインタ演算をする必要がある
    int i, j;
    for (i = HEIGHT - 1; i >= 0; i--) {
        for (j = 0; j < WIDTH; j++) {
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
        if (shortage != 0) {
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