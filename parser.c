#include <stdlib.h>
#include "lexer.h"
#include <math.h>
#include <limits.h>
#include "parser.h"
// get_priority関数で演算子以外が渡された時の戻り値
#define NOT_OPERAND -1
// 優先順位の基本値の最大(括弧の深さを考慮しない時の最大)
#define MAX_PRIORITY 3

Node *create_node(Token *tokens);
// トークンの種類から優先順位を計算する。
// 例)
// 2*(1+10)
// *: MAX_PRIORITY - 1
// +: MAX_PRIORITY + (MAX_PRIORITY - 2)
int get_priority(TokenType token_type, int nest_deps);

// メモ：単項演算子の場合は左辺=0の二項演算として考える
// 関数は単項演算子として考え、演算する。
// -1 = 0 - 1
// sin(x) = 0 sin x
// sin(x**2 + 1) = 0 sin (x**2 + 1)

Node *parse(Token *tokens)
{
    // 括弧の深さを考慮する。(括弧が深いほど優先順位が上がるため)
    int nest_deps = 0;

    // 優先順位が一番低い演算子
    Token *min_priority_operand = NULL;
    int min_priority = INT_MAX;

    Token *current = tokens;
    while (current != NULL)
    {
        // 括弧はじめだったら優先順位の最大値を足してスキップ
        if (current->type == left_parenthesis)
        {
            nest_deps += MAX_PRIORITY;
            Token *parenthesis_token = current;
            current = current->next;
            continue;
        }
        // とじだったら優先順位の最大値を引く
        else if (current->type == right_parenthesis)
        {
            nest_deps -= MAX_PRIORITY;
            Token *parenthesis_token = current;
            current = current->next;
            continue;
        }

        int current_operand_priorirty = get_priority(current->type, nest_deps);
        // 優先順位が同じときは左を優先するので条件は「<=」になる
        if (current_operand_priorirty <= min_priority && current_operand_priorirty != NOT_OPERAND)
        {
            min_priority_operand = current;
            min_priority = current_operand_priorirty;
        }
        current = current->next;
    }

    // 演算子がない場合は値として扱う。
    if (min_priority_operand == NULL)
    {
        // 括弧は不要なので削除する。
        current = tokens;
        while (current != NULL)
        {
            Token *next = current->next;
            if (current->type == left_parenthesis || current->type == right_parenthesis)
            {
                tokens = remove_token(tokens, current);
            }
            current = next;
        }
        // すべて括弧だった場合はNULLを返す
        if (tokens == NULL)
        {
            return NULL;
        }
        tokens->prev == NULL;
        Node *node = create_node(tokens);
        return node;
    }

    // 以降、演算子がある場合のみ
    // 演算子をトークンとして持つノード
    Node *node = create_node(min_priority_operand);
    Token *right_start_token = min_priority_operand->next;
    min_priority_operand->next = NULL;
    if (right_start_token != NULL)
    {
        node->right = parse(right_start_token);
    }
    // 優先順位最小の演算子とtokensが同じではなかったら左側の子は存在する(「-1」など)
    if (min_priority_operand != tokens)
    {
        // 左の子の最後のトークン
        Token *left_end_token = min_priority_operand->prev;
        left_end_token->next = NULL;
        node->left = parse(tokens);
    }
    // 演算子ノードのトークンの前後を切断
    node->token->next = NULL;
    node->token->prev = NULL;
    return node;
}

Node *create_node(Token *tokens)
{
    Node *node = (Node *)calloc(1, sizeof(Node));
    node->left = NULL;
    node->right = NULL;
    node->token = tokens;
    return node;
}

int get_priority(TokenType token_type, int nest_deps)
{
    switch (token_type)
    {
    case func:
    case unary_ope:
        return MAX_PRIORITY + nest_deps;
    case bin_ope_times_div:
        return MAX_PRIORITY - 1 + nest_deps;
    case bin_ope_plus_minus:
        return MAX_PRIORITY - 2 + nest_deps;
    default:
        // 演算子じゃない場合
        return NOT_OPERAND;
    }
}

void dispose_tree(Node *node)
{
    dispose_all_tokens(node->token);
    if (node->left != NULL)
    {
        dispose_tree(node->left);
    }
    if (node->right != NULL)
    {
        dispose_tree(node->right);
    }
    free(node);
}