#include "kcc.h"

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
            strcpy(nodestr, "(+)");
            return nodestr;
        }
        case NODE_SUB: {
            strcpy(nodestr, "(-)");
            return nodestr;
        }
        case NODE_MUL: {
            strcpy(nodestr, "(*)");
            return nodestr;
        }
        case NODE_DIV: {
            strcpy(nodestr, "(/)");
            return nodestr;
        }
        default: {
            sprintf(nodestr, "type: %d", node->type);
            return nodestr;
        }
    }
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

void show_children(Node *node, int depth) {
    if (node == NULL) {
        fprintf(stderr, "NULL");
        return;
    }
    fprintf(stderr, "%s\n", node2str(node));
    fprintf(stderr, "%*s", (depth + 1) * 2, "");
    fprintf(stderr, "left : ");
    show_children(node->lhs, depth + 1);
    fprintf(stderr, "\n");
    fprintf(stderr, "%*s", (depth + 1) * 2, "");
    fprintf(stderr, "right: ");
    show_children(node->rhs, depth + 1);
}

void show_node_tree(Node *top_node) {
    fprintf(stderr, "========node tree========\n");
    show_children(top_node, 0);
    fprintf(stderr, "\n");
    fprintf(stderr, "=========================\n");
}