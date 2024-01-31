#ifndef PARSER
#define PARSER
#include "lexer.h"

// 構文木を表現するノード
typedef struct node
{
    // 演算子or変数or定数のトークンが来る。
    Token *token;
    struct node *left;
    struct node *right;
} Node;

// 構文木を生成する。
Node *parse(Token *tokens);
// 構文木をメモリ開放する
void dispose_tree(Node *node);
#endif