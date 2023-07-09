/*

【参考文献】
BMPファイルの構造
http://coconut.sys.eng.shizuoka.ac.jp/bmp/
https://qiita.com/spc_ehara/items/03d179f4901faeadb184
*/

#include <stdio.h>
#include <stdlib.h>

// 画像の幅[ピクセル]
#define WIDTH 400
// 画像の高さ[ピクセル]
#define HEIGHT 400
// ファイルヘッダのサイズ
#define FILE_HEADER_SIZE 0x0e
#define INFO_HEADER_SIZE 0x28
// ピクセル毎のビット数(今回はフルカラーなので24 = 0x18ビット)
#define BIT_PER_PIXEL 0x18

typedef struct pixel
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
} PIXEL;



/*画像データ関連の関数群*/
void InitializeImageData(PIXEL *);

/*BMP画像関連の関数群*/
void ExportToBMP(PIXEL *);
void WriteBMPFileHeader();
void WriteBMPInfoHeader();
int CalcImageSize();
int CalcFileSize();

int main(void) 
{
    PIXEL imageData[HEIGHT][WIDTH];
    PIXEL *imageDataFirst = &imageData[0][0];
    InitializeImageData(imageDataFirst);
    ExportToBMP(imageDataFirst);
    return 0;
}

// 与えられた二次元配列をすべて0で埋めます。
void InitializeImageData(PIXEL *imageData) 
{
    int i;
    for (i = 0; i < HEIGHT * WIDTH; i++) {
        PIXEL *pixel = (PIXEL *)malloc(sizeof(PIXEL));
        pixel->R = 255;
        pixel->G = 0;
        pixel->B = 0;
        *imageData = *pixel;
        imageData++;
    }
}

// 与えられた二次元データをもとに画像を出力します。
void ExportToBMP(PIXEL *imageData) 
{
    FILE *fp = fopen("test.bmp", "wb");
    WriteBMPFileHeader(fp);
    WriteBMPInfoHeader(fp);

    int i;
    for (i = 0; i < HEIGHT * WIDTH; i++) {
        fwrite(&imageData->B, 1, 1, fp);
        fwrite(&imageData->G, 1, 1, fp);
        fwrite(&imageData->R, 1, 1, fp);
        imageData++;
    }
    fclose(fp);
}

// BMP画像のファイルヘッダを書き込みます。
void WriteBMPFileHeader(FILE *fp) 
{
    // ファイルタイプ ("BM" = 0x42, 0x4d)
    char fileType[] = { 0x42, 0x4d };
    fwrite(fileType, 1, 2, fp);

    // ファイルサイズ, 予約領域(常に0), 画像データまでのオフセット
    int header[] = { CalcFileSize(), 0, 0x36 };
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

// 画像データそのもの(ヘッダなどを除く)のサイズ[バイト]を返します。
int CalcImageSize() 
{
    return WIDTH * HEIGHT * BIT_PER_PIXEL;
}

// 画像「ファイル」のサイズを返します。
int CalcFileSize() 
{
    return FILE_HEADER_SIZE + INFO_HEADER_SIZE + CalcImageSize();
}
