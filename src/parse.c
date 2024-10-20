#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
Expression *parse_expression_prefix(Parser *parser, size_t precedence);
Expression *parse_expression_infix(Parser *parser, Expression *lhs, size_t precedence);

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
        property->name = NULL_SUB_STRING;
        property->kind_name = NULL_SUB_STRING;

        Token property_name = eat(parser, NAME);
        property->name = property_name.str;

        eat(parser, COLON);

        Token property_kind = eat(parser, NAME);
        property->kind_name = property_kind.str;
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

        Token node_name = eat(parser, NAME);
        placeholder->node_name = node_name.str;

        Token name = eat(parser, NAME);
        placeholder->name = name.str;
    }

    eat(parser, COLON);

    rule->expression = parse_expression(parser);
}

// Parse expression
Expression *parse_expression(Parser *parser)
{
    return parse_expression_prefix(parser, MIN_PRECEDENCE);
}

Expression *parse_expression_prefix(Parser *parser, size_t max_precedence)
{
    Expression *expr = NULL;

    // (expression)
    if (peek(parser, PAREN_L))
    {
        eat(parser, PAREN_L);
        expr = parse_expression(parser);
        eat(parser, PAREN_R);
    }

    // Unresolved name
    else if (peek(parser, NAME))
    {
        Token t = eat(parser, NAME);
        expr = NEW(Expression);
        expr->kind = UNRESOLVED_NAME;
        expr->name = t.str;
    }

    // Number
    else if (peek(parser, NUMBER))
    {
        Token t = eat(parser, NUMBER);

        // CLEANUP: Surely there's a better way of doing this?
        size_t len = (t.str.len + 1);
        char *temp = (char *)malloc(sizeof(char) * len);
        strncpy(temp, t.str.str, t.str.len);
        temp[len] = '\0';

        expr = NEW(Expression);
        expr->kind = NUMBER_LITERAL;
        expr->number = atoi(temp);

        free(temp);
    }

    // Error if prefix not found
    if (expr == NULL)
    {
        Token t = parser->tokens.values[parser->current_index];
        fprintf(stderr, "Error at %d:%d, expected expression but got %s\n", t.line, t.column, token_kind_string(t.kind));
        exit(EXIT_FAILURE);
    }

    // Parse expression with infix
    while (true)
    {
        Expression *full_expr = parse_expression_infix(parser, expr, max_precedence);
        if (expr == full_expr)
            break;
        expr = full_expr;
    }

    return expr;
}

Expression *parse_expression_infix(Parser *parser, Expression *lhs, size_t max_precedence)
{
    Operation infix_op;
    Token t = parser->tokens.values[parser->current_index];

    if (strncmp(t.str.str, ".", 1) == 0)
        infix_op = INDEX;
    else if (strncmp(t.str.str, "=", 1) == 0)
        infix_op = EQUAL_TO;
    else if (strncmp(t.str.str, "<", 1) == 0)
        infix_op = LESS_THAN;
    else if (strncmp(t.str.str, ">", 1) == 0)
        infix_op = MORE_THAN;
    else if (strncmp(t.str.str, "<=", 2) == 0)
        infix_op = LESS_THAN_OR_EQUAL;
    else if (strncmp(t.str.str, ">=", 2) == 0)
        infix_op = MORE_THAN_OR_EQUAL;

    else
        return lhs;

    size_t infix_op_precedence = precedence_of(infix_op);

    if (infix_op_precedence <= max_precedence)
    {
        eat(parser, t.kind);

        Expression *expr = NEW(Expression);
        expr->kind = BIN_OP;
        expr->lhs = lhs;
        expr->op = infix_op;
        expr->rhs = parse_expression_prefix(parser, infix_op_precedence);

        return expr;
    }
    else
    {
        return lhs;
    }
}