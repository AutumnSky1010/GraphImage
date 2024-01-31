#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "calclator.h"

// 三角関数
double sin_calclator(double left, double right);
double cos_calclator(double left, double right);
double tan_calclator(double left, double right);

// 指数対数関数
double exp_calclator(double left, double right);
double log_calclator(double left, double right);

// 符号逆にする計算機
double minus_mono_calclator(double left, double right);

// 四則演算計算機
double times_calclator(double left, double right);
double div_calclator(double left, double right);
double plus_calclator(double left, double right);
double minus_calclator(double left, double right);

// トークンに合わせて計算機を取得する
double (*get_calclator(Token *token))(double left, double right);
// 何の関数かを調べて計算機を返す。
double (*get_func_calclator(char *function_name))(double left, double right);

double calclate(double x, Node *node)
{
    // 定数の場合
    if (node->token->type == num)
    {
        return atoi(node->token->data);
    }
    // ネイピア数の場合
    if (node->token->type == e)
    {
        return 2.7182818284590452;
    }
    if (node->token->type == pi)
    {
        return 3.1415926535897932;
    }
    // 変数の場合
    else if (node->token->type == variable)
    {
        return x;
    }

    // 演算子に対応した計算機を取得する
    double (*calclator)(double left, double right) = get_calclator(node->token);
    if (calclator == NULL)
    {
        return 0;
    }

    double left = 0, right = 0;
    if (node->left != NULL)
    {
        left = calclate(x, node->left);
    }
    if (node->right != NULL)
    {
        right = calclate(x, node->right);
    }
    return calclator(left, right);
}

double (*get_calclator(Token *token))(double left, double right)
{
    if (token->type == func)
    {
        return get_func_calclator(token->data);
    }
    else if (token->type == unary_ope)
    {
        // 単項演算子はマイナスしかない。
        return minus_mono_calclator;
    }
    else if (token->type == bin_ope_times_div)
    {
        if (*(token->data) == '*')
        {
            return times_calclator;
        }
        else if (*(token->data) == '/')
        {
            return div_calclator;
        }
    }
    else if (token->type == bin_ope_plus_minus)
    {
        if (*(token->data) == '+')
        {
            return plus_calclator;
        }
        else if (*(token->data) == '-')
        {
            return minus_calclator;
        }
    }
    return NULL;
}
double (*get_func_calclator(char *function_name))(double left, double right)
{
    if (strcmp(function_name, "sin") == 0)
    {
        return sin_calclator;
    }
    else if (strcmp(function_name, "cos") == 0)
    {
        return cos_calclator;
    }
    else if (strcmp(function_name, "tan") == 0)
    {
        return tan_calclator;
    }
    else if (strcmp(function_name, "log") == 0)
    {
        return log_calclator;
    }
    else if (strcmp(function_name, "exp") == 0 || *function_name == '^')
    {
        return exp_calclator;
    }
    else
    {
        return NULL;
    }
}

double sin_calclator(double left, double right)
{
    return sin(right);
}
double cos_calclator(double left, double right)
{
    return cos(right);
}
double tan_calclator(double left, double right)
{
    return tan(right);
}
double exp_calclator(double left, double right)
{
    return pow(left, right);
}
double log_calclator(double left, double right)
{
    // 2変数に対応するのは後回し。(log_a(b))
    return log(right);
}
double minus_mono_calclator(double left, double right)
{
    return -right;
}

double times_calclator(double left, double right)
{
    return left * right;
}
double div_calclator(double left, double right)
{
    return left / right;
}
double plus_calclator(double left, double right)
{
    return left + right;
}
double minus_calclator(double left, double right)
{
    return left - right;
}