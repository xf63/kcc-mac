#include "kcc.h"

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    int error_position;
    if (direct_input) {
        error_position = loc - user_input; //when loc is pointer to the specific location of code, position is available by subtraction.
        fprintf(stderr, "%s\n", user_input);
    }
    else {
        char *line_start = loc;
        while (user_input < line_start && line_start[-1] != '\n') {
            line_start--;
        }
        char *line_end = loc;
        while (*line_end != '\n') {
            line_end++;
        }
        int line_num = 1;
        for (char *p = user_input; p < line_start; p++) {
            if (*p == '\n') {
                line_num++;
            }
        }
        int indent = fprintf(stderr, "%s:%d: ", file_name, line_num);
        fprintf(stderr, "%.*s\n", (int)(line_end - line_start), line_start);
        error_position = loc - line_start + indent;
    }
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
        if (*p == ' ' || *p == '\n') {
            p++;
            continue;
        }

        if (memcmp(p, LINE_COMMENT, 2) == 0) {
            p += 2;
            while (*p != '\n') {
                p++;
            }
            continue;
        }

        if (memcmp(p, BLOCK_COMMENT_START, 2) == 0) {
            char *comment_end = strstr(p + 2, BLOCK_COMMENT_END);
            if (comment_end == NULL) {
                error_at(p, "comment block is not properly closed");
            }
            p = comment_end + 2;
            continue;
        }

        if (prefix_match(p, STRING_QUOTE)) {
            char *string_start = p;
            int len = 2;
            p++;
            while (!prefix_match(p, STRING_QUOTE)) {
                p++;
                len++;
            }
            p++;
            current = new_token(TOKEN_STRING, current, string_start, len);
            continue;
        }

        if ((memcmp(p, RETURN, 6) == 0 || memcmp(p, SIZEOF, 6) == 0) && !is_alphabet_or_number(p[6])) {
            current = new_token(TOKEN_RESERVED, current, p, 6);
            p = p + 6;
            continue;
        }

        if (memcmp(p, WHILE, 5) == 0 && !is_alphabet_or_number(p[5])) {
            current = new_token(TOKEN_RESERVED, current, p, 5);
            p = p + 5;
            continue;
        }

        if ((memcmp(p, ELSE, 4) == 0 || memcmp(p, CHAR_KEYWORD, 4)==0) && !is_alphabet_or_number(p[4])) {
            current = new_token(TOKEN_RESERVED, current, p, 4);
            p = p + 4;
            continue;
        }

        if ((memcmp(p, FOR, 3) == 0 || memcmp(p, INT_KEYWORD, 3) == 0) && !is_alphabet_or_number(p[3])) {
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
        || prefix_match(p, DEREFERENCE) || prefix_match(p, ADDRESS_OF)
        || prefix_match(p, BRACKETS_START) || prefix_match(p, BRACKETS_END)
        || prefix_match(p, POINTER)) {
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
            char tokenstr[100];
            strncpy(tokenstr, tok->str, tok->len);
            tokenstr[tok->len] = '\0';
            char *token_category;
            if (tok->category == TOKEN_RESERVED) {
                token_category = "RSVD:";
            }
            else if (tok->category == TOKEN_IDENTIFIER) {
                token_category = "IDNT:";
            }
            else if (tok->category == TOKEN_STRING) {
                token_category = "STRG:";
            }
            fprintf(stderr, "%s'%s' ", token_category, tokenstr);
        }
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "========tokens end=========\n");
}