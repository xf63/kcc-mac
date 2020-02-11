#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

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

        if (*p == '+' || *p == '-') {
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

int main(int argc, char **argv) {
    if (argc != 2) {
        error("only 2 argument is acceptable\n");
    }

    user_input = argv[1];
    token = tokenize(user_input);

    // prologue
    printf(".intel_syntax noprefix\n");
    printf(".section	__TEXT,__text,regular,pure_instructions\n");
    printf(".macosx_version_min 10, 10\n");
    printf(".globl	_main\n");
    printf("_main:\n");
    // main
    printf("  mov rax, %d\n", expect_number());

    while (!is_at_eof()) {
        if (consume_reserved('+')) {
            printf("  add rax, %d\n", expect_number());
            continue;
        }
        if (consume_reserved('-')) {
            printf("  sub rax, %d\n", expect_number());
            continue;
        }
    }
    
    printf("  ret\n");
    return 0;
}