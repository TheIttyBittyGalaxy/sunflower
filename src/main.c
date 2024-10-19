#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// BOOLEAN

typedef int boolean;
#define TRUE 1
#define FALSE 0

// TOKEN STRUCT //

// TokenKind
typedef enum
{
    NAME,
    COLON,
    LINE,

    OPEN,
    CLOSE,

    END_OF_FILE,
    INVALID_TOKEN
} TokenKind;

inline const char *token_kind_string(TokenKind kind)
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

// Token
typedef struct
{
    TokenKind kind;
    const char *str;
    size_t length;
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

// PRINT TOKENS //

inline void print_token(const Token t)
{
    if (t.kind == LINE || t.kind == END_OF_FILE)
        printf("[%d:%d %s]", t.line, t.column, token_kind_string(t.kind), t.length);
    else
        printf("[%d:%d %s] %.*s", t.line, t.column, token_kind_string(t.kind), t.length, t.str);
}

inline void print_tokens(const TokenArray tokens)
{
    for (size_t i = 0; i < tokens.count; i++)
    {
        printf("%03d ", i);
        print_token(tokens.values[i]);
        printf("\n");
    }
}

// TOKENISE //

inline boolean is_word(const char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

inline boolean is_numeral(const char c)
{
    return (c >= '0' && c <= '9');
}

inline boolean is_word_or_numeral(const char c)
{
    return is_word(c) || is_numeral(c);
}

TokenArray tokenise(const char *const src)
{
    TokenArray tokens;
    initialise_token_array(&tokens, 8); // TODO: What is a good starting value for the length of the array?

    const char *c = src;
    const char *line_start = src;
    size_t line = 1;

    while (*c != '\0')
    {
        Token t = {
            .kind = INVALID_TOKEN,
            .str = c,
            .length = 1,
            .line = line,
            .column = (size_t)(c - line_start) + 1};

        if (*c == ':')
        {
            t.kind = COLON;
            c++;
        }
        else if (*c == '{')
        {
            t.kind = OPEN;
            c++;
        }
        else if (*c == '}')
        {
            t.kind = CLOSE;
            c++;
        }
        else if (*c == '\n')
        {
            t.kind = LINE;
            c++;
            line++;
            line_start = c;
        }
        else if (*c == ' ' || *c == '\t' || *c == '\r')
        {
            c++;
            continue; // Skip this character without generating a token
        }
        else if (is_word(*c))
        {
            t.kind = NAME;
            while (is_word_or_numeral(*(++c)))
                t.length++;
        }
        else
        {
            c++;
        }

        append_token_to_array(&tokens, t);
    }

    Token eof;
    eof.str = c;
    eof.length = 1;
    eof.kind = END_OF_FILE;
    eof.line = line;
    eof.column = c - line_start + 1;
    append_token_to_array(&tokens, eof);

    return tokens;
}

// MAIN //

int main(int argc, char const *argv[])
{
    printf("start\n");

    // TODO: load sunflower program

    // TEMP: Use hardcoded program instead
    const char *source_text = "Node{value:num\n}";

    // TODO: tokenise sunflower program
    printf("tokenise\n");
    TokenArray source_tokens = tokenise(source_text);

    printf("tokens\n");
    print_tokens(source_tokens);

    // TODO: parse sunflower program

    // TODO: generate executable

    printf("complete\n");
    return 0;
}
