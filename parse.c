#include "kcc.h"

bool consume_reserved(char operator) {
    if (token->type != TOKEN_RESERVED || token->str[0] != operator) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char operator) {
    if (token->type != TOKEN_RESERVED || token->str[0] != operator) {
        error_at(token->str, "%c is expected", operator);
    }
    token = token->next;
}

int expect_number() {
    if (token->type != TOKEN_NUMBER) {
        error_at(token->str, "number is expected");
    }
    int value = token->val;
    token = token->next;
    return value;
}

bool is_at_eof() {
    return token->type == TOKEN_EOF;
}

Node *init_node() {
    Node *node = calloc(1, sizeof(Node));
    node->lhs = NULL;
    node->rhs = NULL;
    return node;
}

Node *new_node(NodeType type, Node *lhs, Node *rhs) {
    Node *node = init_node();
    node->type = type;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_number(int number) {
    Node *node = init_node();
    node->type = NODE_VAL;
    node->value = number;
    return node;
}

Node *expression();

/**
 * primary = [0-9]* | "(" expression ")"
**/
Node *primary() {
    if (consume_reserved(PARENTHESES_START)) {
        Node *inside_parentheses = expression();
        expect(PARENTHESES_END);
        return inside_parentheses;
    }
    return new_node_number(expect_number());
}

/**
 * multiplicative = primary ("*" primary | "/" primary)*
**/
Node *multiplicative() {
    Node *lhs = primary();
    while (true) {
        if (consume_reserved(TIMES)) {
            lhs = new_node(NODE_MUL, lhs, primary());
            continue;
        }
        if (consume_reserved(DIVIDE)) {
            lhs = new_node(NODE_DIV, lhs, primary());
            continue;
        }
        return lhs;
    }
    
}

/**
 * additive = multiplicative ("+" multiplicative | "-" multiplicative)*
**/
Node *additive() {
    Node *lhs = multiplicative();
    while (true) {
        if (consume_reserved(PLUS)) {
            lhs = new_node(NODE_ADD, lhs, multiplicative());
            continue;
        }
        if (consume_reserved(MINUS)) {
            lhs = new_node(NODE_SUB, lhs, multiplicative());
            continue;
        }
        return lhs;
    }
}


Node *expression() {
    return additive();
}