#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "tokenise.h"

// Utility functions
bool is_word(const char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool is_digit(const char c)
{
    return (c >= '0' && c <= '9');
}

bool is_word_or_digit(const char c)
{
    return is_word(c) || is_digit(c);
}

// Tokenise
TokenArray tokenise(const char *const src)
{
    TokenArray tokens;
    initialise_token_array(&tokens, 8); // TODO: What is a good starting value for the length of the array?

    const char *c = src;
    const char *line_start = src;
    size_t line = 1;

    while (*c != '\0')
    {
        Token t;
        t.kind = INVALID_TOKEN;
        t.str.str = c;
        t.str.len = 1;
        t.line = line;
        t.column = (size_t)(c - line_start) + 1;

        if (*c == ':')
        {
            t.kind = COLON;
            c++;
        }
        else if (*c == '.')
        {
            t.kind = DOT;
            c++;
        }
        else if (*c == '=')
        {
            t.kind = EQUAL_SIGN;
            c++;
        }
        else if (*c == '!')
        {
            t.kind = EXCLAIM;
            c++;
            if (*c == '=')
            {
                t.kind = EXCLAIM_EQUAL;
                t.str.len++;
                c++;
            }
        }
        else if (*c == '(')
        {
            t.kind = PAREN_L;
            c++;
        }
        else if (*c == ')')
        {
            t.kind = PAREN_R;
            c++;
        }
        else if (*c == '{')
        {
            t.kind = CURLY_L;
            c++;
        }
        else if (*c == '}')
        {
            t.kind = CURLY_R;
            c++;
        }
        else if (*c == '<')
        {
            t.kind = ARROW_L;
            c++;
            if (*c == '=')
            {
                t.kind = ARROW_L_EQUAL;
                t.str.len++;
                c++;
            }
        }
        else if (*c == '>')
        {
            t.kind = ARROW_R;
            c++;
            if (*c == '=')
            {
                t.kind = ARROW_R_EQUAL;
                t.str.len++;
                c++;
            }
        }
        else if (*c == '+')
        {
            t.kind = PLUS;
            c++;
        }
        else if (*c == '-')
        {
            t.kind = MINUS;
            c++;
        }
        else if (*c == '*')
        {
            t.kind = STAR;
            c++;
        }
        else if (*c == '/')
        {
            t.kind = SLASH;
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
        else if (is_digit(*c))
        {
            t.kind = NUMBER;
            while (is_digit(*(++c)))
                t.str.len++;
        }
        else if (is_word(*c))
        {
            t.kind = NAME;
            while (is_word_or_digit(*(++c)))
                t.str.len++;

            if (strncmp(t.str.str, "DEF", 3) == 0)
                t.kind = KEY_DEF;
            else if (strncmp(t.str.str, "FOR", 3) == 0)
                t.kind = KEY_FOR;
            else if (strncmp(t.str.str, "AND", 3) == 0)
                t.kind = KEY_AND;
            if (strncmp(t.str.str, "OR", 2) == 0)
                t.kind = KEY_OR;
        }
        else
        {
            c++;
        }

        append_token_to_array(&tokens, t);
    }

    Token eof;
    eof.str.str = c;
    eof.str.len = 1;
    eof.kind = END_OF_FILE;
    eof.line = line;
    eof.column = c - line_start + 1;
    append_token_to_array(&tokens, eof);

    return tokens;
}
