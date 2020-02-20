#include "kcc.h"

bool consume_reserved(char *operator) {
    if (token->category != TOKEN_RESERVED || strlen(operator) != token->len
    || memcmp(token->str, operator, token->len) != 0) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char *operator) {
    if (token->category != TOKEN_RESERVED || strlen(operator) != token->len
    || memcmp(token->str, operator, token->len) != 0) {
        error_at(token->str, "%s is expected", operator);
    }
    token = token->next;
}

int expect_number() {
    if (token->category != TOKEN_NUMBER) {
        error_at(token->str, "number is expected");
    }
    int value = token->val;
    token = token->next;
    return value;
}

bool is_at_eof() {
    return token->category == TOKEN_EOF;
}

Node *init_node(NodeCategory category) {
    Node *node = calloc(1, sizeof(Node));
    node->category = category;
    node->lhs = NULL;
    node->rhs = NULL;
    node->var = NULL;
    node->func = NULL;
    return node;
}

Node *new_node(NodeCategory category, Node *lhs, Node *rhs) {
    Node *node = init_node(category);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_number(int number) {
    Node *node = init_node(NODE_VAL);
    node->value = number;
    return node;
}

LocalVar *checkAlreadyAllocated(Token *identifier_token) {
    for (LocalVar *var = first; var != NULL; var = var->next) {
        if (identifier_token->len == var->len && memcmp(identifier_token->str, var->name, var->len) == 0) {
            return var;
        }
    }
    return NULL;
}

Token *consume_identifier() {
    if (token->category != TOKEN_IDENTIFIER) {
        return NULL;
    }
    Token *identifier_token = token;
    token = token->next;
    return identifier_token;
}

Node *new_node_local_value(Token *identifier_token) {
    LocalVar *var = checkAlreadyAllocated(identifier_token);
    Node *identifier_node = init_node(NODE_LOCAL_VALUE);
    if (var != NULL) {
        identifier_node->var = var;
        return identifier_node;
    }
    var = calloc(1, sizeof(LocalVar));
    var->name = identifier_token->str;
    var->len = identifier_token->len;

    int default_offset = 0;
    if (last != NULL) {
        default_offset = last->offset;
        last->next = var;
    }
    var->offset = default_offset + 8;
    last = var;
    if (first == NULL) {
        first = var;
    }
    identifier_node->var = var;
    return identifier_node;
}

Node *expression();

/**
 * primary = [0-9]* | "(" expression ")" |
 *          ident ("(" (expression ("," expression)*)? ")")?
**/
Node *primary() {
    if (consume_reserved(PARENTHESES_START)) {
        Node *inside_parentheses = expression();
        expect(PARENTHESES_END);
        return inside_parentheses;
    }
    Token *identifier_token = consume_identifier();
    if (identifier_token != NULL) {
        if (consume_reserved(PARENTHESES_START)) {
            Node *func_call_node = init_node(NODE_CALL_FUNCTION);
            Function *called_func = calloc(1, sizeof(Function));
            called_func->name = identifier_token->str;
            called_func->len = identifier_token->len;
            func_call_node->func = called_func;
            Node *head = init_node(NODE_ARGUMENT);
            Node *current = head;
            if (!consume_reserved(PARENTHESES_END)) {
                head->lhs = expression();
                while (consume_reserved(WITH)) {
                    current->rhs = init_node(NODE_ARGUMENT);
                    current = current->rhs;
                    current->lhs = expression();
                }
                func_call_node->rhs = head;
                expect(PARENTHESES_END);
            }
            return func_call_node;
        }
        return new_node_local_value(identifier_token);
    }
    return new_node_number(expect_number());
}

/**
 * unary = ("+" | "-")? primary
**/
Node *unary() {
    if (consume_reserved(PLUS)) {
        return primary();
    }
    if (consume_reserved(MINUS)) {
        return new_node(NODE_SUB, new_node_number(0), primary());
    }
    return primary();
}

/**
 * multiplicative = unary ("*" unary | "/" unary)*
**/
Node *multiplicative() {
    Node *lhs = unary();
    while (true) {
        if (consume_reserved(TIMES)) {
            lhs = new_node(NODE_MUL, lhs, unary());
            continue;
        }
        if (consume_reserved(DIVIDE)) {
            lhs = new_node(NODE_DIV, lhs, unary());
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

/**
 * relational = additive (">" additive | ">=" additive | "<" additive | "<=" additive)*
**/
Node *relational() {
    Node *lhs = additive();
    while (true) {
        if (consume_reserved(GREATER_THAN)) {
            lhs = new_node(NODE_GREATER_THAN, lhs, additive());
            continue;
        }
        if (consume_reserved(GREATER_EQUAL)) {
            lhs = new_node(NODE_GREATER_EQUAL, lhs, additive());
            continue;
        }
        if (consume_reserved(LESS_THAN)) {
            lhs = new_node(NODE_LESS_THAN, lhs, additive());
            continue;
        }
        if (consume_reserved(LESS_EQUAL)) {
            lhs = new_node(NODE_LESS_EQUAL, lhs, additive());
            continue;
        }
        return lhs;
    }
}

/**
 * equality = relational ("==" relational | "!=" relational)*
**/
Node *equality() {
    Node *lhs = relational();
    while (true) {
        if (consume_reserved(EQUAL)) {
            lhs = new_node(NODE_EQUAL, lhs, relational());
            continue;
        }
        if (consume_reserved(NOT_EQUAL)) {
            lhs = new_node(NODE_NOT_EQUAL, lhs, relational());
            continue;
        }
        return lhs;
    }
}

/**
 * assign = equality ("=" assign)?
**/
Node *assign() {
    Node *lhs = equality();
    if (consume_reserved(ASSIGN)) {
        lhs = new_node(NODE_ASSIGN, lhs, assign());
    }
    return lhs;
}

/**
 * expression = assign
**/
Node *expression() {
    return assign();
}

/**
 * statement = expression ";" |
 *              "return" expression ";" |
 *              "if" "(" equality ")" statement ("else" statement)? |
 *              "while" "(" equality ")" statement |
 *              "for" "(" (assign)? ";" (equality)? ";" (assign)? ")" statement
 *              "{" statement* "}"
**/
Node *statement() {
    Node *lhs;
    if (consume_reserved(RETURN)) {
        lhs = new_node(NODE_RETURN, NULL, expression());
        expect(END);
        return lhs;
    }
    if (consume_reserved(IF)) {
        expect(PARENTHESES_START);
        lhs = equality();
        expect(PARENTHESES_END);
        Node *rhs = statement();
        if (!consume_reserved(ELSE)) {
            return new_node(NODE_IF, lhs, rhs);
        }
        Node *else_node = init_node(NODE_ELSE);
        else_node->lhs = rhs;
        else_node->rhs = statement();
        return new_node(NODE_IF, lhs, else_node);
    }
    if (consume_reserved(WHILE)) {
        expect(PARENTHESES_START);
        lhs = equality();
        expect(PARENTHESES_END);
        return new_node(NODE_WHILE, lhs, statement());
    }
    if (consume_reserved(FOR)) {
        expect(PARENTHESES_START);
        Node *init_check_node = init_node(NODE_FOR);
        Node *increment_execute_node = init_node(NODE_FOR);
        if (!consume_reserved(END)) {
            init_check_node->lhs = assign();
            expect(END);
        }
        if (!consume_reserved(END)) {
            init_check_node->rhs = equality();
            expect(END);
        }
        if (!consume_reserved(PARENTHESES_END)) {
            increment_execute_node->lhs = assign();
            expect(PARENTHESES_END);
        }
        increment_execute_node->rhs = statement();
        return new_node(NODE_FOR, init_check_node, increment_execute_node);
    }
    if (consume_reserved(BRACES_START)) {
        Node *brace_top_node = init_node(NODE_BLOCK);
        lhs = brace_top_node;
        while (!consume_reserved(BRACES_END)) {
            lhs->lhs = statement();
            lhs->rhs = init_node(NODE_BLOCK);
            lhs = lhs->rhs;
        }
        return brace_top_node;
    }
    lhs = expression();
    expect(END);
    return lhs;
}

/**
 * program = statement*
**/
Node **program() {
    first = NULL;
    last = NULL;
    int statement_num = 0;
    while (!is_at_eof()) {
        top_nodes[statement_num] = statement();
        statement_num++;
    }
    top_nodes[statement_num] = NULL;
    return top_nodes;
}
