#ifndef LEXER
#define LEXER

typedef enum token_type
{
    // 数字
    num,

    // 定数
    e,  // ネイピア数
    pi, // 円周率

    // 演算子
    bin_ope_times_div,  // 二項演算子乗除算
    bin_ope_plus_minus, // 二項演算子加減算
    unary_ope,          // 単項演算子
    func,               // 関数(1変数なので実質単項演算子)

    // 括弧
    left_parenthesis,
    right_parenthesis,

    // その他の文字列
    variable,
} TokenType;

// 双方向連結リスト
typedef struct token
{
    char *data;
    TokenType type;
    struct token *next;
    struct token *prev;
} Token;

typedef struct
{
    char *start;
    int length;
} StringInfo;

Token *lexical(char *expression);
void dispose_all_tokens(Token *root);
void dispose_token(Token *token);
Token *remove_token(Token *root, Token *target);

#endif