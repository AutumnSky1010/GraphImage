#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "lexer.h"

// 与えた文字列とトークンの種類でトークンを生成する。
Token *create_token(StringInfo *info, TokenType type);
void dispose_token(Token *token);

// 演算子以外の文字かを判定する
bool is_other_char(char current);
// 演算子以外の文字列からトークンの種類を取得する
TokenType get_token_type_for_other(char *literal);

// 字句解析を行う。
Token *lexical(char *expression)
{
    char *current = expression;
    // 文字列生成用変数
    StringInfo string_info = {"", 0};
    // トークンのリスト(ダミーノードで初期化)
    Token *token_list = (Token *)calloc(1, sizeof(Token));
    Token *token_start = token_list;
    token_list->data = NULL;
    token_list->next = NULL;
    token_list->prev = NULL;
    bool is_during_other_str = false;
    bool is_diring_num = false;

    while (*current != '\0')
    {
        Token *token = NULL;

        // その他の文字列の入力中だった場合 かつ 現在の文字がその他の文字じゃない時はトークンにする。
        if (is_during_other_str && !is_other_char(*current))
        {
            token = create_token(&string_info, variable);
            TokenType token_type = get_token_type_for_other(token->data);
            token->type = token_type;
            token->prev = token_list;
            token_list->next = token;
            token_list = token_list->next;

            is_during_other_str = false;
        }
        // 数字の入力中だった場合 かつ 現在の文字が数字じゃない時はトークンにする。
        else if (is_diring_num && !isdigit(*current))
        {
            token = create_token(&string_info, num);
            token->prev = token_list;
            token_list->next = token;
            token_list = token_list->next;

            is_diring_num = false;
        }

        // 括弧などの演算子
        if (*current == '(' || *current == ')')
        {
            string_info.start = current;
            string_info.length = 1;
            TokenType token_type = *current == '(' ? left_parenthesis : right_parenthesis;
            token = create_token(&string_info, token_type);
        }
        // べき乗
        else if (*current == '^')
        {
            string_info.start = current;
            string_info.length = 1;
            token = create_token(&string_info, func);
        }
        else if (*current == '-' || *current == '+')
        {
            string_info.start = current;
            string_info.length = 1;
            // 前回がない場合と前回が二項演算子の場合は単項演算子として扱う。
            // 例) -1 + 1, 1 + -1
            if ((token_list->data == "" || token_list->type == bin_ope_plus_minus || token_list->type == bin_ope_times_div) &&
                *current == '-')
            {
                token = create_token(&string_info, unary_ope);
            }
            else
            {
                token = create_token(&string_info, bin_ope_plus_minus);
            }
        }
        else if (*current == '*' || *current == '/')
        {
            string_info.start = current;
            string_info.length = 1;
            token = create_token(&string_info, bin_ope_times_div);
        }
        // 数字
        else if (isdigit(*current))
        {
            if (!is_diring_num)
            {
                string_info.start = current;
                string_info.length = 1;
                is_diring_num = true;
            }
            else
            {
                ++string_info.length;
            }
            current++;
            continue;
        }
        // 円周率の場合
        else if (*current == 'p' && *(current + 1) == 'i')
        {
            string_info.start = current;
            string_info.length = 2;
            token = create_token(&string_info, pi);
            token->prev = token_list;
            token_list->next = token;
            token_list = token_list->next;
            current += 2;
            continue;
        }
        // ネイピア数の場合
        else if (*current == 'e')
        {
            string_info.start = current;
            string_info.length = 1;
            token = create_token(&string_info, e);
        }
        // それ以外の文字
        else
        {
            if (!is_during_other_str)
            {
                string_info.start = current;
                string_info.length = 1;
                is_during_other_str = true;
            }
            else
            {
                ++string_info.length;
            }
            current++;
            continue;
        }
        token->prev = token_list;
        token_list->next = token;
        token_list = token_list->next;
        current++;
    }

    // その他の文字列が入力中の場合はnextに追加する。
    if (is_during_other_str)
    {
        token_list->next = create_token(&string_info, variable);
        token_list->next->prev = token_list; // 次の前は現在
    }
    // 数字の入力中だった場合はnextに追加する。
    else if (is_diring_num && !isdigit(*current))
    {
        token_list->next = create_token(&string_info, num);
        token_list->next->prev = token_list;
    }
    // リストの先頭
    Token *start = token_start->next;
    start->prev = NULL;
    dispose_token(token_start);
    return start;
}

bool is_other_char(char current)
{
    bool is_operand = current == '*' || current == '/' || current == '-' || current == '+' || current == '^';
    bool is_parenthesis = current == '(' || current == ')';
    bool is_number = isdigit(current);
    return !is_operand && !is_number && !is_parenthesis;
}

TokenType get_token_type_for_other(char *literal)
{
    if (strcmp(literal, "sin") == 0)
    {
        return func;
    }
    else if (strcmp(literal, "cos") == 0)
    {
        return func;
    }
    else if (strcmp(literal, "tan") == 0)
    {
        return func;
    }
    else if (strcmp(literal, "log") == 0)
    {
        return func;
    }
    else if (strcmp(literal, "exp") == 0)
    {
        return func;
    }
    else
    {
        return variable;
    }
}

Token *create_token(StringInfo *info, TokenType type)
{
    char *current = info->start;
    int i;
    char *other_str = (char *)calloc(info->length + 1, sizeof(char));

    Token *token = (Token *)calloc(1, sizeof(Token));
    if (other_str == NULL || token == NULL)
    {
        perror("メモリ確保エラー");
        exit(-1);
    }
    for (i = 0; i < info->length; ++i)
    {
        other_str[i] = *current;
        current++;
    }
    // null文字
    other_str[info->length] = '\0';
    token->data = other_str;
    token->type = type;
    token->next = NULL;
    return token;
}

void dispose_all_tokens(Token *root)
{
    Token *current = root;
    while (current != NULL)
    {
        Token *tmp = current->next;
        dispose_token(current);
        current = tmp;
    }
}
void dispose_token(Token *token)
{
    if (token->data != NULL)
    {
        free(token->data);
    }
    free(token);
}

// 指定したトークンを削除して先頭を返す。
Token *remove_token(Token *root, Token *target)
{
    // リストが空の場合
    if (target == NULL || root == NULL)
    {
        return NULL;
    }
    // リストの要素が1個の場合
    if (target->next == NULL && target == root)
    {
        dispose_token(target);
        return NULL;
    }

    // リストの先頭だった場合
    if (target == root)
    {
        Token *new_root = target->next;
        new_root->prev = NULL;
        root = new_root;
    }
    // リストの終点だった場合
    else if (target->next == NULL)
    {
        Token *new_end_token = target->prev;
        new_end_token->next = NULL;
    }
    // リストの途中の要素だった場合
    else
    {
        Token *prev = target->prev;
        Token *next = target->next;
        prev->next = next;
        next->prev = prev;
    }
    dispose_token(target);
    return root;
}