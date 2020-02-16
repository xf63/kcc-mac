#include "kcc.h"

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

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