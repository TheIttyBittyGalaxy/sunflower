#include <stdio.h>
#include <stdlib.h>

#include "token.h"

// Token array methods
void initialise_token_array(TokenArray *const array, const size_t size)
{
    array->values = size > 0
                        ? (Token *)malloc(sizeof(Token) * size)
                        : NULL;

    array->length = size;
    array->count = 0;
};

void reallocate_token_array(TokenArray *const array, const size_t size)
{
    if (size > 0)
    {
        array->values = (array->length == 0)
                            ? (Token *)malloc(sizeof(Token) * size)
                            : (Token *)realloc(array->values, sizeof(Token) * size);
    }
    else if (array->length > 0)
    {
        free(array->values);
        array->values = NULL;
    }

    array->length = size;
    if (array->count > array->length)
        array->count = array->length;
}

void append_token_to_array(TokenArray *const array, const Token value)
{
    if (array->count == array->length)
        reallocate_token_array(array, array->length > 0 ? array->length * 2 : 1);

    array->values[array->next_index] = value;
    array->next_index++;
}

// Strings & printing
const char *token_kind_string(TokenKind kind)
{
    if (kind == NAME)
        return "NAME";
    if (kind == COLON)
        return "COLON";
    if (kind == LINE)
        return "LINE";
    if (kind == OPEN)
        return "OPEN";
    if (kind == CLOSE)
        return "CLOSE";
    if (kind == END_OF_FILE)
        return "END_OF_FILE";
    if (kind == INVALID_TOKEN)
        return "INVALID_TOKEN";
    return "INVALID_TOKEN_KIND";
}

void print_token(const Token t)
{
    if (t.kind == LINE || t.kind == END_OF_FILE)
        printf("[%d:%d %s]", t.line, t.column, token_kind_string(t.kind), t.length);
    else
        printf("[%d:%d %s] %.*s", t.line, t.column, token_kind_string(t.kind), t.length, t.str);
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