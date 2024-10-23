#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>

#include "sub_string.h"

// TokenKind
typedef enum
{
    KEY_DEF,
    KEY_FOR,
    KEY_AND,
    KEY_OR,
    NAME,
    NUMBER,

    COLON,
    DOT,
    EQUAL_SIGN,
    EXCLAIM,
    EXCLAIM_EQUAL,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    LINE,

    PAREN_L,
    PAREN_R,
    CURLY_L,
    CURLY_R,
    ARROW_L,
    ARROW_R,
    ARROW_L_EQUAL,
    ARROW_R_EQUAL,

    END_OF_FILE,
    INVALID_TOKEN
} TokenKind;

// Token
typedef struct
{
    TokenKind kind;
    sub_string str;
    size_t line;
    size_t column;
} Token;

// TokenArray
typedef struct
{
    size_t length;
    union
    {
        Token *values;
        Token *first;
    };
    union
    {
        size_t count;
        size_t next_index;
    };
} TokenArray;

// Token array methods
void initialise_token_array(TokenArray *const array, const size_t size);
void reallocate_token_array(TokenArray *const array, const size_t size);
void append_token_to_array(TokenArray *const array, const Token value);

// Strings & printing
const char *token_kind_string(TokenKind kind);
void print_token(const Token t);
void print_tokens(const TokenArray tokens);

#endif