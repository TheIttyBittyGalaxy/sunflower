#include <stdio.h>
#include <stdlib.h>

#include "token.h"

// Strings & printing
const char *token_kind_string(TokenKind kind)
{
    if (kind == KEY_DEF)
        return "KEY_DEF";
    if (kind == KEY_FOR)
        return "KEY_FOR";
    if (kind == KEY_AND)
        return "KEY_AND";
    if (kind == KEY_OR)
        return "KEY_OR";
    if (kind == NAME)
        return "NAME";
    if (kind == NUMBER)
        return "NUMBER";

    if (kind == COLON)
        return "COLON";
    if (kind == DOT)
        return "DOT";
    if (kind == EQUAL_SIGN)
        return "EQUAL_SIGN";
    if (kind == EXCLAIM)
        return "EXCLAIM";
    if (kind == EXCLAIM_EQUAL)
        return "EXCLAIM_EQUAL";
    if (kind == PLUS)
        return "PLUS";
    if (kind == MINUS)
        return "MINUS";
    if (kind == STAR)
        return "STAR";
    if (kind == SLASH)
        return "SLASH";
    if (kind == LINE)
        return "LINE";

    if (kind == PAREN_L)
        return "PAREN_L";
    if (kind == PAREN_R)
        return "PAREN_R";
    if (kind == CURLY_L)
        return "CURLY_L";
    if (kind == CURLY_R)
        return "CURLY_R";
    if (kind == ARROW_L)
        return "ARROW_L";
    if (kind == ARROW_R)
        return "ARROW_R";
    if (kind == ARROW_L_EQUAL)
        return "ARROW_L_EQUAL";
    if (kind == ARROW_R_EQUAL)
        return "ARROW_R_EQUAL";

    if (kind == END_OF_FILE)
        return "END_OF_FILE";
    if (kind == INVALID_TOKEN)
        return "INVALID_TOKEN";

    return "INVALID_TOKEN_KIND";
}

void print_token(const Token t)
{
    if (t.kind == LINE || t.kind == END_OF_FILE)
        printf("[%d:%d %s]", t.line, t.column, token_kind_string(t.kind));
    else
        printf("[%d:%d %s] %.*s", t.line, t.column, token_kind_string(t.kind), t.str.len, t.str.str);
}

void print_tokens(const TokenArray tokens)
{
    for (size_t i = 0; i < tokens.count; i++)
    {
        printf("%03d ", i);
        print_token(tokens.values[i]);
        printf("\n");
    }
}
