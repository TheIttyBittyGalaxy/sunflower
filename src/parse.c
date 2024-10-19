#include <stdio.h>

#include "parse.h"

// Parser
typedef struct
{
    Program *program;
    TokenArray tokens;
    size_t current_index;
} Parser;

// Parser methods
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
void parse(Program *program, TokenArray tokens)
{
    Parser parser;
    parser.program = program;
    parser.tokens = tokens;
    parser.current_index = 0;

    parse_program(&parser);
}

// Parse program
void parse_program(Parser *parser)
{
    parse_node_declaration(parser);
}

// Parse node declaration
void parse_node_declaration(Parser *parser)
{
    Token name = eat(parser, NAME);
    parser->program->node.name = name.str;
    parser->program->node.name_len = name.length;

    eat(parser, OPEN);

    Token property_name = eat(parser, NAME);
    parser->program->node.property_name = property_name.str;
    parser->program->node.property_name_len = property_name.length;

    eat(parser, COLON);

    Token property_kind = eat(parser, NAME);
    parser->program->node.property_kind_name = property_kind.str;
    parser->program->node.property_kind_name_len = property_kind.length;

    eat(parser, CLOSE);
}
