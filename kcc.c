#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#define PLUS '+'
#define MINUS '-'
#define TIMES '*'
#define DIVIDE '/'
#define PARENTHESES_START '('
#define PARENTHESES_END ')'
typedef enum {
    TOKEN_RESERVED,
    TOKEN_NUMBER,
    TOKEN_EOF,
} TokenType;

typedef struct Token Token;

struct Token {
    TokenType type;
    Token *next;
    int val;
    char *str;
};

Token *token;
typedef enum {
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_VAL,
} NodeType;

typedef struct Node Node;
struct Node {
    NodeType type;
    Node *lhs;
    Node *rhs;
    int value;
};

Node *top_node;

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

char *user_input;

void error_at(char *loc, char *fmt, ...) {
    int error_position = loc - user_input; //when loc is pointer to the specific location of code, position is available by subtraction.
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", error_position, "");
    fprintf(stderr, "^ ");

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

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

Token *new_token(TokenType type, Token *current, char *str) {
    Token *token = calloc(1, sizeof(Token));
    token->type = type;
    token->str = str;
    current->next = token;
    return token;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *current = &head;

    while (*p) {
        if (*p == ' ') {
            p++;
            continue;
        }

        if (*p == PLUS || *p == MINUS || *p == TIMES || *p == DIVIDE ||
        *p == PARENTHESES_START || *p == PARENTHESES_END) {
            current = new_token(TOKEN_RESERVED, current, p);
            p++;
            continue;
        }

        if ('0' <= *p && *p <= '9') {
            current = new_token(TOKEN_NUMBER, current, p);
            current->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "unexpected character: %c\n", *p);
    }

    new_token(TOKEN_EOF, current, p);
    return head.next;
}

void show_tokens(Token *head) {
    fprintf(stderr, "========tokens start=======\n");
    for (Token *tok = head; tok->type != TOKEN_EOF; tok = tok->next) {
        fprintf(stderr, "_%c_ ", tok->str[0]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "========tokens end=========\n");
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

void generate(Node *node) {
    if (node == NULL) {
        return;
    }
    generate(node->lhs);
    generate(node->rhs);

    switch (node->type) {
        case NODE_VAL: {
            printf("  push %d\n", node->value);
            return;
        }
        case NODE_ADD: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  add rax, rdi\n");
            printf("  push rax\n");
            return;
        }
        case NODE_SUB: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  sub rax, rdi\n");
            printf("  push rax\n");
            return;
        }
        case NODE_MUL: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mul rdi\n");
            printf("  push rax\n");
            return;
        }
        case NODE_DIV: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  cqo\n");
            printf("  idiv rdi\n");
            printf("  push rax\n");
            return;
        }
    }
}


char nodestr[100];

char *node2str(Node *node) {
    if (node == NULL) {
        return "NULL";
    }
    if (node->type == NODE_VAL) {
        sprintf(nodestr, "val: %d", node->value);
        return nodestr;
    }
    switch (node->type) {
        case NODE_ADD: {
            sprintf(nodestr, "(+)", node->type);
            return nodestr;
        }
        case NODE_SUB: {
            sprintf(nodestr, "(-)", node->type);
            return nodestr;
        }
        case NODE_MUL: {
            sprintf(nodestr, "(*)", node->type);
            return nodestr;
        }
        case NODE_DIV: {
            sprintf(nodestr, "(/)", node->type);
            return nodestr;
        }
        default: {
            sprintf(nodestr, "type: %d", node->type);
            return nodestr;
        }
    }
}

void show_children(Node *node, int depth) {
    if (node == NULL) return;
    fprintf(stderr, "%s\n", node2str(node));
    fprintf(stderr, "%*s", (depth + 1) * 2, "");
    fprintf(stderr, "left : ");
    show_children(node->lhs, depth + 1);
    fprintf(stderr, "\n");
    fprintf(stderr, "%*s", (depth + 1) * 2, "");
    fprintf(stderr, "right: ");
    show_children(node->rhs, depth + 1);
    fprintf(stderr, "\n");
}

void show_node_tree(Node *top_node) {
    fprintf(stderr, "========node tree========\n");
    show_children(top_node, 0);
    fprintf(stderr, "=========================\n");
}

void generate_assembly(Node *top_node) {
    // prologue
    printf(".intel_syntax noprefix\n");
    printf(".section	__TEXT,__text,regular,pure_instructions\n");
    printf(".macosx_version_min 10, 10\n");
    printf(".globl	_main\n");
    printf("_main:\n");
    // main
    generate(top_node);
    printf("  pop rax\n");
    printf("  ret\n");
}

int main(int argc, char **argv) {
    int input_index = 1;
    bool show_debug_info = false;
    if (argc == 3) {
        show_debug_info = true;
        input_index++;
    }

    user_input = argv[input_index];
    token = tokenize(user_input);
    if (show_debug_info) {
        show_tokens(token);
    }
    top_node = expression();
    if (show_debug_info) {
        show_node_tree(top_node);
    }
    generate_assembly(top_node);
    
    return 0;
}