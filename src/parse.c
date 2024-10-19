#include <stdio.h>
#include <stdbool.h>

#include "parse.h"

// Parser
typedef struct
{
    Program *program;
    TokenArray tokens;
    size_t current_index;
} Parser;

// Parser methods
bool peek(Parser *parser, TokenKind expectation)
{
    size_t i = parser->current_index;
    Token t = parser->tokens.values[i];

    if (expectation != LINE)
        while (t.kind == LINE)
            t = parser->tokens.values[++i];

    return t.kind == expectation;
}

Token eat(Parser *parser, TokenKind expectation)
{
    Token t = parser->tokens.values[parser->current_index];

    if (expectation != LINE)
        while (t.kind == LINE)
            t = parser->tokens.values[++(parser->current_index)];

    // TODO: Create a language error, rather than terminating the entire program
    if (t.kind != expectation)
    {
        fprintf(stderr, "Error at %d:%d, expected %s but got %s\n", t.line, t.column, token_kind_string(expectation), token_kind_string(t.kind));
        exit(EXIT_FAILURE);
    }

    parser->current_index++;
    return t;
}

// Parse methods
void parse_program(Parser *parser);
void parse_node_declaration(Parser *parser);

// Parse
Program *parse(TokenArray tokens)
{
    Program *program = (Program *)malloc(sizeof(Program));

    Parser parser;
    parser.program = program;
    parser.tokens = tokens;
    parser.current_index = 0;

    parse_program(&parser);

    return program;
}

// Parse program
void parse_program(Parser *parser)
{
    parse_node_declaration(parser);
}

// Parse node declaration
void parse_node_declaration(Parser *parser)
{
    Node *node = &parser->program->node;
    node->name = NULL_SUB_STRING;
    node->properties = NULL;
    node->property_count = 0;

    Token name = eat(parser, NAME);
    node->name = name.str;

    eat(parser, OPEN);

    while (!peek(parser, CLOSE))
    {
        node->property_count++;
        if (node->property_count == 1)
            node->properties = (Property *)malloc(sizeof(Property));
        else
            node->properties = (Property *)realloc(node->properties, sizeof(Property) * node->property_count);

        Property *property = node->properties + (node->property_count - 1);
        property->name = NULL_SUB_STRING;
        property->kind_name = NULL_SUB_STRING;

        Token property_name = eat(parser, NAME);
        property->name = property_name.str;

        eat(parser, COLON);

        Token property_kind = eat(parser, NAME);
        property->kind_name = property_kind.str;
    }

    eat(parser, CLOSE);
}
