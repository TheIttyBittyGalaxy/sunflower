#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
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
void parse_rule(Parser *parser);
Expression *parse_expression(Parser *parser);

// Parse
Program *parse(TokenArray tokens)
{
    Program *program = NEW(Program);
    INIT_ARRAY(program->nodes);
    INIT_ARRAY(program->rules);

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
    while (true)
    {
        if (peek(parser, KEY_DEF))
            parse_node_declaration(parser);
        else if (peek(parser, KEY_FOR))
            parse_rule(parser);
        else
            break;
    }

    eat(parser, END_OF_FILE);
}

// Parse node declaration
void parse_node_declaration(Parser *parser)
{
    Program *program = parser->program;

    Node *node = EXTEND_ARRAY(program->nodes, Node);
    node->name = NULL_SUB_STRING;
    INIT_ARRAY(node->properties);

    eat(parser, KEY_DEF);

    Token name = eat(parser, NAME);
    node->name = name.str;

    eat(parser, CURLY_L);

    while (!peek(parser, CURLY_R))
    {
        Property *property = EXTEND_ARRAY(node->properties, Property);
        property->type.primitive = TYPE_PRIMITIVE__UNRESOLVED;
        property->type.node = NULL;

        Token property_name = eat(parser, NAME);
        property->name = property_name.str;

        eat(parser, COLON);

        Token property_kind = eat(parser, NAME);
        property->type_name = property_kind.str;
    }

    eat(parser, CURLY_R);
}

// Parse rule
void parse_rule(Parser *parser)
{
    Program *program = parser->program;

    Rule *rule = EXTEND_ARRAY(program->rules, Rule);
    INIT_ARRAY(rule->placeholders);

    eat(parser, KEY_FOR);

    while (peek(parser, NAME))
    {
        Placeholder *placeholder = EXTEND_ARRAY(rule->placeholders, Placeholder);
        placeholder->index = rule->placeholders_count - 1;
        placeholder->type.primitive = TYPE_PRIMITIVE__UNRESOLVED;
        placeholder->type.node = NULL;

        Token node_name = eat(parser, NAME);
        placeholder->type_name = node_name.str;

        Token name = eat(parser, NAME);
        placeholder->name = name.str;
    }

    eat(parser, COLON);

    rule->expression = parse_expression(parser);
}

// Parse expression
Expression *parse_precedence(Parser *parser, Precedence precedence)
{
    Expression *lhs = NULL;

    // (expression)
    if (peek(parser, PAREN_L))
    {
        eat(parser, PAREN_L);
        lhs = parse_expression(parser);
        eat(parser, PAREN_R);
    }

    // Unresolved name
    else if (peek(parser, NAME))
    {
        Token t = eat(parser, NAME);
        lhs = NEW(Expression);
        lhs->variant = EXPR_VARIANT__UNRESOLVED_NAME;
        lhs->name = t.str;
    }

    // Number
    else if (peek(parser, NUMBER))
    {
        Token t = eat(parser, NUMBER);

        int num = 0;

        for (size_t i = 0; i < t.str.len; i++)
            num = num * 10 + ((int)(t.str.str[i]) - 48);

        lhs = NEW(Expression);
        lhs->variant = EXPR_VARIANT__LITERAL;
        lhs->literal_value.type_primitive = TYPE_PRIMITIVE__NUMBER;
        lhs->literal_value.number = num;
    }

    // Error if primary value not found
    if (lhs == NULL)
    {
        Token t = parser->tokens.values[parser->current_index];
        fprintf(stderr, "Error at %d:%d, expected expression but got %s\n", t.line, t.column, token_kind_string(t.kind));
        exit(EXIT_FAILURE);
    }

    // Parse infix expression
    while (true)
    {
        Token t = parser->tokens.values[parser->current_index];

        Operation operation;
        if (t.kind == DOT)
            operation = OPERATION__ACCESS;
        else if (t.kind == STAR)
            operation = OPERATION__MUL;
        else if (t.kind == SLASH)
            operation = OPERATION__DIV;
        else if (t.kind == PLUS)
            operation = OPERATION__ADD;
        else if (t.kind == MINUS)
            operation = OPERATION__SUB;
        else if (t.kind == ARROW_L)
            operation = OPERATION__LESS_THAN;
        else if (t.kind == ARROW_R)
            operation = OPERATION__MORE_THAN;
        else if (t.kind == ARROW_L_EQUAL)
            operation = OPERATION__LESS_THAN_OR_EQUAL;
        else if (t.kind == ARROW_R_EQUAL)
            operation = OPERATION__MORE_THAN_OR_EQUAL;
        else if (t.kind == EQUAL_SIGN)
            operation = OPERATION__EQUAL_TO;
        else if (t.kind == EXCLAIM_EQUAL)
            operation = OPERATION__NOT_EQUAL_TO;
        else if (t.kind == KEY_AND)
            operation = OPERATION__LOGICAL_AND;
        else if (t.kind == KEY_OR)
            operation = OPERATION__LOGICAL_OR;
        else
            break; // There is no expression past this point

        size_t operation_precedence = precedence_of(operation);

        if (operation_precedence <= precedence)
            break; // Wrong precedence, return and continue at a different level of precedence

        eat(parser, t.kind);

        Expression *expr = NEW(Expression);
        expr->variant = EXPR_VARIANT__BIN_OP;
        expr->lhs = lhs;
        expr->op = operation;
        expr->rhs = parse_precedence(parser, operation_precedence); // NOTE: If an operation should be right-associative pass `operation_precedence - 1`

        lhs = expr;
    }

    return lhs;
}

Expression *parse_expression(Parser *parser)
{
    return parse_precedence(parser, 0);
}
