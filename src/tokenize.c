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
    tokens.length = 64; // TODO: What is a good starting value for the length of the array?
    tokens.count = 0;
    tokens.values = (Token *)malloc(sizeof(Token) * tokens.length);

    const char *c = src;
    const char *line_start = src;
    size_t line = 1;

    while (true)
    {
        Token t;
        t.kind = INVALID_TOKEN;
        t.str.str = c;
        t.str.len = 1;
        t.line = line;
        t.column = (size_t)(c - line_start) + 1;

        if (*c == '\0')
        {
            t.kind = END_OF_FILE;
            t.str.len = 0;
        }
        else if (*c == ':')
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
            if (*c == '/')
            {
                while (*c != '\n' && *c != '\0')
                    c++;
                continue; // Skip emitting a token
            }
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
            continue; // Skip emitting a token
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

        if (tokens.count == tokens.length)
        {
            size_t new_length = tokens.length * 2;
            tokens.values = (Token *)realloc(tokens.values, sizeof(Token) * new_length);
            tokens.length = new_length;
        }

        tokens.values[tokens.count] = t;
        tokens.count++;

        if (t.kind == END_OF_FILE)
            break;
    }

    return tokens;
}
