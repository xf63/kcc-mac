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

Token *new_token(TokenCategory category, Token *current, char *str, int len) {
    Token *token = calloc(1, sizeof(Token));
    token->category = category;
    token->str = str;
    token->len = len;
    current->next = token;
    return token;
}

bool prefix_match(char *target, char *pattern) {
    return memcmp(target, pattern, strlen(pattern)) == 0;
}

bool is_alphabet_or_number(char c) {
    return ('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || c == '_';
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

        if (memcmp(p, RETURN, 6) == 0 && !is_alphabet_or_number(p[6])) {
            current = new_token(TOKEN_RESERVED, current, p, 6);
            p = p + 6;
            continue;
        }

        if (memcmp(p, WHILE, 5) == 0 && !is_alphabet_or_number(p[5])) {
            current = new_token(TOKEN_RESERVED, current, p, 5);
            p = p + 5;
            continue;
        }

        if (memcmp(p, ELSE, 4) == 0 && !is_alphabet_or_number(p[4])) {
            current = new_token(TOKEN_RESERVED, current, p, 4);
            p = p + 4;
            continue;
        }

        if (memcmp(p, FOR, 3) == 0 && !is_alphabet_or_number(p[3])) {
            current = new_token(TOKEN_RESERVED, current, p, 3);
            p = p + 3;
            continue;
        }

        if (memcmp(p, IF, 2) == 0 && !is_alphabet_or_number(p[2])) {
            current = new_token(TOKEN_RESERVED, current, p, 2);
            p = p + 2;
            continue;
        }

        if (prefix_match(p, EQUAL) || prefix_match(p, NOT_EQUAL) || prefix_match(p, GREATER_EQUAL) || prefix_match(p, LESS_EQUAL)) {
            current = new_token(TOKEN_RESERVED, current, p, 2);
            p = p + 2;
            continue;
        }

        if (prefix_match(p, PLUS) || prefix_match(p, MINUS) || prefix_match(p, TIMES) || prefix_match(p, DIVIDE)
        || prefix_match(p, PARENTHESES_START) || prefix_match(p, PARENTHESES_END)
        || prefix_match(p, GREATER_THAN) || prefix_match(p, LESS_THAN) 
        || prefix_match(p, ASSIGN) || prefix_match(p, END) || prefix_match(p, WITH)
        || prefix_match(p, BRACES_START) || prefix_match(p, BRACES_END)
        || prefix_match(p, DEREFERENCE) || prefix_match(p, ADDRESS_OF)) {
            current = new_token(TOKEN_RESERVED, current, p, 1);
            p++;
            continue;
        }

        if ('0' <= *p && *p <= '9') {
            current = new_token(TOKEN_NUMBER, current, p, -1);
            current->val = strtol(p, &p, 10);
            continue;
        }

        if ('a' <= *p && *p <= 'z') {
            char *identifier_start = p;
            int i = 0;
            while (is_alphabet_or_number(*p)) {
                i++;
                p++;
            }
            current = new_token(TOKEN_IDENTIFIER, current, identifier_start, i);
            continue;
        }

        error_at(p, "unexpected character: %c\n", *p);
    }

    new_token(TOKEN_EOF, current, p, -1);
    return head.next;
}

void show_tokens(Token *head) {
    fprintf(stderr, "========tokens start=======\n");
    for (Token *tok = head; tok->category != TOKEN_EOF; tok = tok->next) {
        if (tok->category == TOKEN_NUMBER) {
            fprintf(stderr, "num:'%d' ", tok->val);
        }
        else {
            char tokenstr[10];
            strncpy(tokenstr, tok->str, tok->len);
            tokenstr[tok->len] = '\0';
            char *token_category;
            if (tok->category == TOKEN_RESERVED) {
                token_category = "RSVD:";
            }
            else if (tok->category == TOKEN_IDENTIFIER) {
                token_category = "IDNT:";
            }
            fprintf(stderr, "%s'%s' ", token_category, tokenstr);
        }
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "========tokens end=========\n");
}