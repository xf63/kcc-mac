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
    if (!peek_reserved(INT_KEYWORD) && !peek_reserved(CHAR_KEYWORD)) {
        error_at(token->str, "type expected");
    }
    Type *type;
    if (consume_reserved(INT_KEYWORD)) {
        type = int_type;
    }
    if (consume_reserved(CHAR_KEYWORD)) {
        type = char_type;
    }
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
    if (rhs != NULL) {
        node->type = rhs->type;
        if (lhs != NULL) {
            node->type = lhs->type;
        }
    }
    return node;
}

Node *new_node_number(int number) {
    Node *node = init_node(NODE_VAL);
    node->value = number;
    node->type = int_type;
    return node;
}

Function *check_function(Token *identifier_token) {
    for (Function *func = first_function; func != NULL; func = func->next) {
        if (identifier_token->len == func->len && memcmp(identifier_token->str, func->name, func->len) == 0) {
            return func;
        }
    }
    return NULL;
}

Variable *check_local_variable(Token *identifier_token) {
    for (Variable *var = current_function->first; var != NULL; var = var->next) {
        if (identifier_token->len == var->len && memcmp(identifier_token->str, var->name, var->len) == 0) {
            return var;
        }
    }
    return NULL;
}

Variable *check_global_variable(Token *identifier_token) {
    for (Variable *var = first_global_var; var != NULL; var = var->next) {
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

Node *new_node_variable(Token *identifier_token) {
    Variable *var = check_local_variable(identifier_token);
    if (var == NULL) {
        var = check_global_variable(identifier_token);
    }
    if (var == NULL) {
        char variable_name[10];
        strncpy(variable_name, identifier_token->str, identifier_token->len);
        variable_name[identifier_token->len] = '\0';
        error_at(identifier_token->str, "%s has not been defined", variable_name);
    }
    Node *identifier_node = init_node(NODE_ACCESS_VARIABLE);
    identifier_node->var = var;
    identifier_node->type = var->type;
    return identifier_node;
}

Node *cross_type_addition(Node* lhs, Node*rhs, Token *addition_token) {
    if (lhs->type == NULL) {
        error_at(addition_token->str, "cannot refine type of left hand side");
    }
    if (rhs->type == NULL) {
        error_at(addition_token->str, "cannot refine type of right hand side");
    }
    if (is_character_or_integer(lhs->type) && is_character_or_integer(rhs->type)) {
        return new_node(NODE_ADD_INTEGER, lhs, rhs);
    }
    if (is_pointer_or_array(lhs->type) && is_integer(rhs->type)) {
        return new_node(NODE_ADD_POINTER, lhs, rhs);
    }
    if (is_integer(lhs->type) && is_pointer_or_array(rhs->type)) {
        return new_node(NODE_ADD_POINTER, rhs, lhs);
    }
    error_at(addition_token->str, "addition between these type is not defined");
    return NULL;
}

Node *cross_type_subtraction(Node* lhs, Node*rhs, Token *subtraction_token) {
    if (lhs->type == NULL) {
        error_at(subtraction_token->str, "cannot refine type of left hand side");
    }
    if (rhs->type == NULL) {
        error_at(subtraction_token->str, "cannot refine type of right hand side");
    }
    if (is_character_or_integer(lhs->type) && is_character_or_integer(rhs->type)) {
        return new_node(NODE_SUB_INTEGER, lhs, rhs);
    }
    if (is_pointer_or_array(lhs->type) && is_integer(rhs->type)) {
        return new_node(NODE_SUB_POINTER, lhs, rhs);
    }
    if (is_pointer_or_array(lhs->type) && is_pointer_or_array(rhs->type)) {
        return new_node(NODE_DIFF_POINTER, rhs, lhs);
    }
    error_at(subtraction_token->str, "subtraction between these type is not defined");
    return NULL;
}

Node *expression();

/**
 * primary = [0-9]* | "(" expression ")" |
 *          ident ("(" (expression ("," expression)*)? ")")? |
 *          """ string """
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
            Function *called_func = check_function(identifier_token);
            if (called_func == NULL) {
                error_at(identifier_token->str, "undefined function");
            }
            called_func->name = identifier_token->str;
            called_func->len = identifier_token->len;
            func_call_node->func = called_func;
            func_call_node->type = called_func->type;
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
        return new_node_variable(identifier_token);
    }
    if (token->category == TOKEN_STRING) {
        String *str = calloc(1, sizeof(String));
        str->content = token->str + 1;
        str->len = token->len - 2;
        if (current_string != NULL) {
            current_string->next = str;
            str->num = current_string->num + 1;
        }
        current_string = str;
        if (first_string == NULL) {
            str->num = 0;
            first_string = str;
        }
        Node *str_node = init_node(NODE_STRING);
        str_node->type = array_of(char_type, str->len + 1);
        str_node->str = str;
        token = token->next;
        return str_node;
    }
    return new_node_number(expect_number());
}

/**
 * array = primary ("[" number "]")?
**/
Node *array() {
    Node *node =  primary();
    while (consume_reserved(BRACKETS_START)) {
        node = new_node(NODE_DEREFERENCE, NULL, new_node(NODE_ADD_POINTER, node, new_node_number(expect_number())));
        node->type = node->rhs->type->point_to;
        expect_reserved(BRACKETS_END);
    }
    return node;
}

/**
 * unary = ("+" | "-")? array |
 *          ("*" | "&") unary |
 *          "sizeof" unary
**/
Node *unary() {
    if (consume_reserved(PLUS)) {
        return array();
    }
    if (consume_reserved(MINUS)) {
        return new_node(NODE_SUB_INTEGER, new_node_number(0), array());
    }
    if (consume_reserved(DEREFERENCE)) {
        Node *deref_node = new_node(NODE_DEREFERENCE, NULL, unary());
        deref_node->type = deref_node->rhs->type->point_to;
        return deref_node;
    }
    if (consume_reserved(ADDRESS_OF)) {
        Node *address_node = new_node(NODE_ADDRESS_OF, NULL, unary());
        address_node->type = pointer_to(address_node->rhs->type);
        return address_node;
    }
    if (consume_reserved(SIZEOF)) {
        return new_node_number(unary()->type->size);
    }
    return array();
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
        Token *operator_token = token;
        if (consume_reserved(PLUS)) {
            lhs = cross_type_addition(lhs, multiplicative(), operator_token);
            continue;
        }
        if (consume_reserved(MINUS)) {
            lhs = cross_type_subtraction(lhs, multiplicative(), operator_token);
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
    Node *node = equality();
    Token *assign_token = token;
    if (consume_reserved(ASSIGN)) {
        node = new_node(NODE_ASSIGN, node, assign());
        if (node->lhs->type == NULL || node->rhs->type == NULL) {
            error_at(assign_token->str, "type cannot be refined");
        }
    }
    return node;
}

/**
 * expression = assign
**/
Node *expression() {
    return assign();
}

/**
 * local_variable = type ident ("[" number "]")?
**/
Node *local_variable() {
    Type *type = expect_type();
    Token *identifier_token = expect_identifier();
    Variable *var = check_local_variable(identifier_token);
    if (var != NULL) {
        char variable_name[10];
        strncpy(variable_name, var->name, var->len);
        variable_name[var->len] = '\0';
        error_at(var->name, "%s is already defined", variable_name);
    }
    while (consume_reserved(BRACKETS_START)) {
        type = array_of(type, expect_number());
        expect_reserved(BRACKETS_END);
    }
    Node *variable_definition_node = init_node(NODE_DEFINE_VARIABLE);
    var = calloc(1, sizeof(Variable));
    var->is_local = true;
    var->type = type;
    var->name = identifier_token->str;
    var->len = identifier_token->len;
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
    variable_definition_node->type = var->type;
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
    if (peek_reserved(INT_KEYWORD) || peek_reserved(CHAR_KEYWORD)) {
        lhs = local_variable();
        expect_reserved(END);
        return lhs;
    }
    lhs = expression();
    expect_reserved(END);
    return lhs;
}


/**
 * global_declaration = type ident "(" (definition ("," definition)*)? ")" statement |
 *                      global_variable ";"
**/
Node *global_declaration() {
    Type *type = expect_type();
    Token *identifier_token = expect_identifier();
    if (consume_reserved(PARENTHESES_START)) {
        Node *node = init_node(NODE_DEFINE_FUNCTION);
        if (current_function == NULL) {
            current_function = calloc(1, sizeof(Function));
            first_function = current_function;        
        }
        else {
            current_function->next = calloc(1, sizeof(Function));
            current_function = current_function->next;
        }
        current_function->type = type;
        current_function->name = identifier_token->str;
        current_function->len = identifier_token->len;
        node->func = current_function;
        node->type = type;
        if (!consume_reserved(PARENTHESES_END)) {
            Node *head = init_node(NODE_ARGUMENT);
            head->lhs = local_variable();
            Node *current = head;
            while (consume_reserved(WITH)) {
                current->rhs = init_node(NODE_ARGUMENT);
                current = current->rhs;
                current->lhs = local_variable();
            }
            node->lhs = head;
            expect_reserved(PARENTHESES_END);
        }
        if (consume_reserved(END)) {
            return node;
        }
        node->rhs = statement();
        return node;
    }
    while (consume_reserved(BRACKETS_START)) {
        type = array_of(type, expect_number());
        expect_reserved(BRACKETS_END);
    }
    if (check_global_variable(identifier_token) != NULL) {
        error_at(identifier_token->str, "global variable already defined");
    }
    Variable *var = calloc(1, sizeof(Variable));
    var->is_local = false;
    var->type = type;
    var->name = identifier_token->str;
    var->len = identifier_token->len;
    if (last_global_var != NULL) {
        last_global_var->next = var;
    }
    last_global_var = var;
    if (first_global_var == NULL) {
        first_global_var = last_global_var;
    }
    Node *variable_node = init_node(NODE_DEFINE_VARIABLE);
    variable_node->type = type;
    variable_node->var = var;
    expect_reserved(END);
    return variable_node;
}

/**
 * program = declaration*
**/
Node **program() {
    int num = 0;
    while (!is_at_eof()) {
        top_nodes[num] = global_declaration();
        num++;
    }
    top_nodes[num] = NULL;
    return top_nodes;
}
