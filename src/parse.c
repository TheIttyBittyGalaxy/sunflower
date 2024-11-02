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
        property->type = TYPE_PRIMITIVE__INVALID;
        property->type_name = NULL_SUB_STRING;
        property->node_type = NULL;

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
        placeholder->node_type = NULL;

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
        expr->variant = EXPR_VARIANT__UNRESOLVED_NAME;
        expr->name = t.str;
    }

    // Number
    else if (peek(parser, NUMBER))
    {
        Token t = eat(parser, NUMBER);

        int num = 0;

        for (size_t i = 0; i < t.str.len; i++)
            num = num * 10 + ((int)(t.str.str[i]) - 48);

        expr = NEW(Expression);
        expr->variant = EXPR_VARIANT__NUMBER_LITERAL;
        expr->number = num;
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

    if (t.kind == DOT)
        infix_op = OPERATION__INDEX;

    else if (t.kind == STAR)
        infix_op = OPERATION__MUL;
    else if (t.kind == SLASH)
        infix_op = OPERATION__DIV;
    else if (t.kind == PLUS)
        infix_op = OPERATION__ADD;
    else if (t.kind == MINUS)
        infix_op = OPERATION__SUB;

    else if (t.kind == ARROW_L)
        infix_op = OPERATION__LESS_THAN;
    else if (t.kind == ARROW_R)
        infix_op = OPERATION__MORE_THAN;
    else if (t.kind == ARROW_L_EQUAL)
        infix_op = OPERATION__LESS_THAN_OR_EQUAL;
    else if (t.kind == ARROW_R_EQUAL)
        infix_op = OPERATION__MORE_THAN_OR_EQUAL;

    else if (t.kind == EQUAL_SIGN)
        infix_op = OPERATION__EQUAL_TO;
    else if (t.kind == EXCLAIM_EQUAL)
        infix_op = OPERATION__NOT_EQUAL_TO;

    else if (t.kind == KEY_AND)
        infix_op = OPERATION__LOGICAL_AND;
    else if (t.kind == KEY_OR)
        infix_op = OPERATION__LOGICAL_OR;

    else
        return lhs;

    size_t infix_op_precedence = precedence_of(infix_op);

    if (infix_op_precedence <= max_precedence)
    {
        eat(parser, t.kind);

        Expression *expr = NEW(Expression);
        expr->variant = EXPR_VARIANT__BIN_OP;
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