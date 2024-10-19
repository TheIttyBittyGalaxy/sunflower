#include "parse.h"

// Parser
typedef struct
{
    Program *program;
    TokenArray tokens;
    size_t current_index;
} Parser;

// Parser methods
Token eat(Parser *parser, TokenKind token_kind)
{
    Token t = parser->tokens.values[parser->current_index];
    // TODO: Error if the token kind is incorrect!

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
