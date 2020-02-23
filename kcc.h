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
#define WHILE "while"
#define FOR "for"
#define BRACES_START "{"
#define BRACES_END "}"
#define WITH ","
#define DEREFERENCE "*"
#define ADDRESS_OF "&"
#define TYPE_INT "int"
#define POINTER "*"

typedef enum {
    TOKEN_RESERVED,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
} TokenCategory;

typedef struct Token Token;
typedef struct Node Node;
typedef struct LocalVar LocalVar;
typedef struct Function Function;
typedef struct Type Type;

struct Token {
    TokenCategory category;
    Token *next;
    int val;
    char *str;
    int len;
};

Token *token;
typedef enum {
    NODE_VAL,
    NODE_ADD_INTEGER,
    NODE_ADD_POINTER,
    NODE_SUB_INTEGER,
    NODE_SUB_POINTER,
    NODE_DIFF_POINTER,
    NODE_MUL,
    NODE_DIV,
    NODE_EQUAL,
    NODE_NOT_EQUAL,
    NODE_GREATER_THAN,
    NODE_GREATER_EQUAL,
    NODE_LESS_THAN,
    NODE_LESS_EQUAL,
    NODE_ASSIGN,
    NODE_ACCESS_VARIABLE,
    NODE_RETURN,
    NODE_IF,
    NODE_ELSE,
    NODE_WHILE,
    NODE_FOR,
    NODE_BLOCK,
    NODE_CALL_FUNCTION,
    NODE_ARGUMENT,
    NODE_DEFINE_FUNCTION,
    NODE_DEREFERENCE,
    NODE_ADDRESS_OF,
    NODE_DEFINE_VARIABLE,
} NodeCategory;

struct Node {
    NodeCategory category;
    Node *lhs;
    Node *rhs;
    int value;
    LocalVar *var;
    Function *func;
    Type *type;
};

struct LocalVar {
    Type *type;
    LocalVar *next;
    char *name;
    int len;
    int offset;
};

struct Function {
    Type *type;
    char *name;
    int len;
    LocalVar *first;
    LocalVar *last;
    Function *next;
};

typedef enum {
    INTEGER_TYPE,
    POINTER_TYPE,
} TypeCategory;

struct Type {
    TypeCategory category;
    int size;
    Type *point_to;
};

Type *int_type;
Type *pointer_to(Type* base);
bool is_integer(Type *type);
bool is_pointer(Type *type);

Node *top_nodes[100];

char *user_input;

Function *first_function;
Function *current_function;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

Token *tokenize(char *p);
Node **program();
void generate_assembly(Node **top_node);

void show_tokens(Token *head);
void show_node_tree(Node **top_node);