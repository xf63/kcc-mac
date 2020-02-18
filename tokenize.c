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

Token *new_token(TokenType type, Token *current, char *str, int len) {
    Token *token = calloc(1, sizeof(Token));
    token->type = type;
    token->str = str;
    token->len = len;
    current->next = token;
    return token;
}

bool prefix_match(char *target, char *pattern) {
    return memcmp(target, pattern, strlen(pattern)) == 0;
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

        if (prefix_match(p, EQUAL) || prefix_match(p, NOT_EQUAL) || prefix_match(p, GREATER_EQUAL) || prefix_match(p, LESS_EQUAL)) {
            current = new_token(TOKEN_RESERVED, current, p, 2);
            p = p + 2;
            continue;
        }

        if (prefix_match(p, PLUS) || prefix_match(p, MINUS) || prefix_match(p, TIMES) || prefix_match(p, DIVIDE)
        || prefix_match(p, PARENTHESES_START) || prefix_match(p, PARENTHESES_END)
        || prefix_match(p, GREATER_THAN) || prefix_match(p, LESS_THAN) || prefix_match(p, END)) {
            current = new_token(TOKEN_RESERVED, current, p, 1);
            p++;
            continue;
        }

        if ('0' <= *p && *p <= '9') {
            current = new_token(TOKEN_NUMBER, current, p, -1);
            current->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "unexpected character: %c\n", *p);
    }

    new_token(TOKEN_EOF, current, p, -1);
    return head.next;
}

void show_tokens(Token *head) {
    fprintf(stderr, "========tokens start=======\n");
    for (Token *tok = head; tok->type != TOKEN_EOF; tok = tok->next) {
        if (tok->type == TOKEN_NUMBER) {
            fprintf(stderr, "%d ", tok->val);
        }
        else {
            char output[10];
            strncpy(output, tok->str, tok->len);
            output[tok->len] = '\0';
            fprintf(stderr, "%s ", output);
        }
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "========tokens end=========\n");
}