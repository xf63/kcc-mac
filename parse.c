#include "kcc.h"
bool peek_reserved(char *operator) {
    if (token->category != TOKEN_RESERVED || strlen(operator) != token->len
    || memcmp(token->str, operator, token->len) != 0) {
        return false;
    }
    return true;
}

bool consume_reserved(char *operator) {
    if (token->category != TOKEN_RESERVED || strlen(operator) != token->len
    || memcmp(token->str, operator, token->len) != 0) {
        return false;
    }
    token = token->next;
    return true;
}

void expect_reserved(char *operator) {
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

Type *expect_type() {
    expect_reserved(TYPE_INT);
    Type *type = int_type;
    while (consume_reserved(POINTER)) {
        type = pointer_to(type);
    }
    return type;
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
    node->type = NULL;
    return node;
}

Node *new_node(NodeCategory category, Node *lhs, Node *rhs) {
    Node *node = init_node(category);
    node->lhs = lhs;
    node->rhs = rhs;
    if (lhs != NULL && lhs->type != NULL) {
        node->type = lhs->type;
    }
    return node;
}

Node *new_node_number(int number) {
    Node *node = init_node(NODE_VAL);
    node->value = number;
    return node;
}

LocalVar *checkAlreadyAllocated(Token *identifier_token) {
    for (LocalVar *var = current_function->first; var != NULL; var = var->next) {
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

Token *expect_identifier() {
    if (token->category != TOKEN_IDENTIFIER) {
        error_at(token->str, "alphabet character is expected");
    }
    Token *identifier_token = token;
    token = token->next;
    return identifier_token;
}

Node *new_node_local_value(Token *identifier_token) {
    LocalVar *var = checkAlreadyAllocated(identifier_token);
    if (var == NULL) {
        char variable_name[10];
        strncpy(variable_name, identifier_token->str, identifier_token->len);
        variable_name[identifier_token->len] = '\0';
        error_at(identifier_token->str, "%s has not been defined", variable_name);
    }
    Node *identifier_node = init_node(NODE_LOCAL_VALUE);
    identifier_node->var = var;
    identifier_node->type = var->type;
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
        expect_reserved(PARENTHESES_END);
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
                func_call_node->lhs = head;
                expect_reserved(PARENTHESES_END);
            }
            return func_call_node;
        }
        return new_node_local_value(identifier_token);
    }
    return new_node_number(expect_number());
}

/**
 * unary = ("+" | "-")? primary |
 *          ("*" | "&") unary
**/
Node *unary() {
    if (consume_reserved(PLUS)) {
        return primary();
    }
    if (consume_reserved(MINUS)) {
        return new_node(NODE_SUB, new_node_number(0), primary());
    }
    if (consume_reserved(DEREFERENCE)) {
        return new_node(NODE_DEREFERENCE, NULL, unary());
    }
    if (consume_reserved(ADDRESS_OF)) {
        return new_node(NODE_ADDRESS_OF, NULL, unary());
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
 * definition = type ident
**/
Node *definition() {
    Type *type = expect_type();
    Token *identifier_token = expect_identifier();
    LocalVar *var = checkAlreadyAllocated(identifier_token);
    if (var != NULL) {
        char variable_name[10];
        strncpy(variable_name, var->name, var->len);
        variable_name[var->len] = '\0';
        error_at(var->name, "%s is already defined", variable_name);
    }
    Node *variable_definition_node = init_node(NODE_DEFINE_VARIABLE);
    var = calloc(1, sizeof(LocalVar));
    var->name = identifier_token->str;
    var->len = identifier_token->len;
    var->type = type;
    int default_offset = 0;
    if (current_function->last != NULL) {
        default_offset = current_function->last->offset;
        current_function->last->next = var;
    }
    var->offset = default_offset + var->type->size;
    current_function->last = var;
    if (current_function->first == NULL) {
        current_function->first = var;
    }
    variable_definition_node->var = var;
    return variable_definition_node;
}

/**
 * statement = expression ";" |
 *              "return" expression ";" |
 *              "if" "(" equality ")" statement ("else" statement)? |
 *              "while" "(" equality ")" statement |
 *              "for" "(" (assign)? ";" (equality)? ";" (assign)? ")" statement |
 *              "{" statement* "}" |
 *              definition ";"
**/
Node *statement() {
    Node *lhs;
    if (consume_reserved(RETURN)) {
        lhs = new_node(NODE_RETURN, NULL, expression());
        expect_reserved(END);
        lhs->func = current_function;
        return lhs;
    }
    if (consume_reserved(IF)) {
        expect_reserved(PARENTHESES_START);
        lhs = equality();
        expect_reserved(PARENTHESES_END);
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
        expect_reserved(PARENTHESES_START);
        lhs = equality();
        expect_reserved(PARENTHESES_END);
        return new_node(NODE_WHILE, lhs, statement());
    }
    if (consume_reserved(FOR)) {
        expect_reserved(PARENTHESES_START);
        Node *init_check_node = init_node(NODE_FOR);
        Node *increment_execute_node = init_node(NODE_FOR);
        if (!consume_reserved(END)) {
            init_check_node->lhs = assign();
            expect_reserved(END);
        }
        if (!consume_reserved(END)) {
            init_check_node->rhs = equality();
            expect_reserved(END);
        }
        if (!consume_reserved(PARENTHESES_END)) {
            increment_execute_node->lhs = assign();
            expect_reserved(PARENTHESES_END);
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
    if (peek_reserved(TYPE_INT)) {
        lhs = definition();
        expect_reserved(END);
        return lhs;
    }
    lhs = expression();
    expect_reserved(END);
    return lhs;
}


/**
 * declaration = type ident "(" (definition ("," definition)*)? ")" statement
**/
Node *declaration() {
    Type *type = expect_type();
    Node *function_node = init_node(NODE_DEFINE_FUNCTION);
    current_function = calloc(1, sizeof(Function));
    Token *function_token = expect_identifier();
    current_function->name = function_token->str;
    current_function->len = function_token->len;
    function_node->func = current_function;
    function_node->type = type;
    expect_reserved(PARENTHESES_START);
    if (!consume_reserved(PARENTHESES_END)) {
        Node *head = init_node(NODE_ARGUMENT);
        head->lhs = definition();
        Node *current = head;
        while (consume_reserved(WITH)) {
            current->rhs = init_node(NODE_ARGUMENT);
            current = current->rhs;
            current->lhs = definition();
        }
        function_node->lhs = head;
        expect_reserved(PARENTHESES_END);
    }
    function_node->rhs = statement();
    return function_node;
}

/**
 * program = declaration*
**/
Node **program() {
    int num = 0;
    while (!is_at_eof()) {
        top_nodes[num] = declaration();
        num++;
    }
    top_nodes[num] = NULL;
    return top_nodes;
}
