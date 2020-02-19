#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#define PLUS "+"
#define MINUS "-"
#define TIMES "*"
#define DIVIDE "/"
#define PARENTHESES_START "("
#define PARENTHESES_END ")"
#define EQUAL "=="
#define NOT_EQUAL "!="
#define GREATER_THAN ">"
#define GREATER_EQUAL ">="
#define LESS_THAN "<"
#define LESS_EQUAL "<="
#define ASSIGN "="
#define END ";"
#define RETURN "return"
#define IF "if"
#define ELSE "else"

typedef enum {
    TOKEN_RESERVED,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
} TokenType;

typedef struct Token Token;
typedef struct Node Node;
typedef struct LocalVar LocalVar;

struct Token {
    TokenType type;
    Token *next;
    int val;
    char *str;
    int len;
};

Token *token;
typedef enum {
    NODE_VAL,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_EQUAL,
    NODE_NOT_EQUAL,
    NODE_GREATER_THAN,
    NODE_GREATER_EQUAL,
    NODE_LESS_THAN,
    NODE_LESS_EQUAL,
    NODE_ASSIGN,
    NODE_LOCAL_VALUE,
    NODE_RETURN,
    NODE_IF,
    NODE_ELSE,
} NodeType;

struct Node {
    NodeType type;
    Node *lhs;
    Node *rhs;
    int value;
    LocalVar *var;
};

struct LocalVar {
    LocalVar *next;
    char *name;
    int len;
    int offset;
};


Node *top_nodes[100];

LocalVar *first;
LocalVar *last;

char *user_input;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

Token *tokenize(char *p);
Node **program();
void generate_assembly(Node **top_node);

void show_tokens(Token *head);
void show_node_tree(Node **top_node);