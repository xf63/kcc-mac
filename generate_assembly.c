#include "kcc.h"

int syntax_number;
char *node2str(Node *node);
char argument_register[10][10] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char func_name[10];

char *get_func_name(Node *node) {
    strncpy(func_name, node->func->name, node->func->len);
    func_name[node->func->len] = '\0';
    return func_name;
}

void generate_local_value(Node *local_value_node) {
    if (local_value_node->category != NODE_LOCAL_VALUE) {
        error("local value expected, but got %s", node2str(local_value_node));
    }
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", local_value_node->var->offset);
    printf("  push rax\n");
}

void generate(Node *node) {
    if (node == NULL) {
        return;
    }
    switch (node->category) {
        case NODE_LOCAL_VALUE:
            generate_local_value(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case NODE_ASSIGN:
            generate_local_value(node->lhs);
            generate(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
        case NODE_RETURN: {
            generate(node->rhs);
            printf("  pop rax\n");
            printf("  jmp Lend\n");
            return;
        }
        case NODE_IF: {
            int if_syntax_number = syntax_number;
            syntax_number++;
            generate(node->lhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            if (node->rhs->category == NODE_ELSE) {
                printf("  je Lelse%03d\n", if_syntax_number);
                generate(node->rhs->lhs);
                printf("  jmp Lend%03d\n", if_syntax_number);
                printf("Lelse%03d:\n", if_syntax_number);
                generate(node->rhs->rhs);
            }
            else {
                printf("  je Lend%03d\n", if_syntax_number);
                generate(node->rhs);
            }
            printf("Lend%03d:\n", if_syntax_number);
            return;
        }
        case NODE_WHILE: {
            int while_syntax_number = syntax_number;
            syntax_number++;
            printf("Lbegin%03d:\n", while_syntax_number);
            generate(node->lhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je Lend%03d\n", while_syntax_number);
            generate(node->rhs);
            printf("  jmp Lbegin%03d\n", while_syntax_number);
            printf("Lend%03d:\n", while_syntax_number);
            return;
        }
        case NODE_FOR: {
            int for_syntax_number = syntax_number;
            syntax_number++;
            generate(node->lhs->lhs);
            printf("Lbegin%03d:\n", for_syntax_number);
            generate(node->lhs->rhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je Lend%03d\n", for_syntax_number);
            generate(node->rhs->rhs);
            generate(node->rhs->lhs);
            printf("  jmp Lbegin%03d\n", for_syntax_number);
            printf("Lend%03d:\n", for_syntax_number);
            return;
        }
        case NODE_BLOCK: {
            for (Node *stmt = node; stmt->lhs != NULL; stmt = stmt->rhs) {
                generate(stmt->lhs);
            }
            return;
        }
        case NODE_CALL_FUNCTION: {
            int arg_num = 0;
            for (Node *arg_node = node->rhs; arg_node != NULL; arg_node = arg_node->rhs) {
                generate(arg_node->lhs);
                printf("  pop %s\n", argument_register[arg_num]);
                arg_num++;
            }
            printf("  call _%s\n", get_func_name(node));
            printf("  push rax\n");
            return;
        }
        default:
            break;
    }

    generate(node->lhs);
    generate(node->rhs);

    switch (node->category) {
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
        case NODE_EQUAL: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzx rax, al\n");
            printf("  push rax\n");
            return;
        }
        case NODE_NOT_EQUAL: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzx rax, al\n");
            printf("  push rax\n");
            return;
        }
        case NODE_GREATER_THAN: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  cmp rax, rdi\n");
            printf("  setg al\n");
            printf("  movzx rax, al\n");
            printf("  push rax\n");
            return;
        }
        case NODE_GREATER_EQUAL: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  cmp rax, rdi\n");
            printf("  setge al\n");
            printf("  movzx rax, al\n");
            printf("  push rax\n");
            return;
        }
        case NODE_LESS_THAN: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzx rax, al\n");
            printf("  push rax\n");
            return;
        }
        case NODE_LESS_EQUAL: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzx rax, al\n");
            printf("  push rax\n");
            return;
        }
        default:
            error("unknown node category: %d", node->category);
    }
}

void generate_assembly(Node **top_nodes) {
    syntax_number = 0;
    // prologue
    printf(".intel_syntax noprefix\n");
    printf(".section	__TEXT,__text,regular,pure_instructions\n");
    printf(".macosx_version_min 10, 10\n");
    printf(".globl	_main\n");
    printf("_main:\n");
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    // local value stack allocation
    if (last != NULL) {
        printf("  sub rsp, %d\n", last->offset);
    }
    // main
    for (int i = 0; top_nodes[i] != NULL; i++) {
        generate(top_nodes[i]);
        printf("  pop rax\n");
    }
    printf("Lend:\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

char nodestr[100];

char *node2str(Node *node) {
    if (node == NULL) {
        return "-";
    }
    if (node->category == NODE_VAL) {
        sprintf(nodestr, "val: %d", node->value);
        return nodestr;
    }
    switch (node->category) {
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
        case NODE_EQUAL: {
            strcpy(nodestr, "(==)");
            return nodestr;
        }
        case NODE_NOT_EQUAL: {
            strcpy(nodestr, "(!=)");
            return nodestr;
        }
        case NODE_GREATER_THAN: {
            strcpy(nodestr, "(>)");
            return nodestr;
        }
        case NODE_GREATER_EQUAL: {
            strcpy(nodestr, "(>=)");
            return nodestr;
        }
        case NODE_LESS_THAN: {
            strcpy(nodestr, "(<)");
            return nodestr;
        }
        case NODE_LESS_EQUAL: {
            strcpy(nodestr, "(<=)");
            return nodestr;
        }
        case NODE_LOCAL_VALUE: {
            sprintf(nodestr, "local value offset : %d", node->var->offset);
            return nodestr;
        }
        case NODE_ASSIGN: {
            strcpy(nodestr, "assign");
            return nodestr;
        }
        case NODE_RETURN: {
            strcpy(nodestr, "return");
            return nodestr;
        }
        case NODE_IF: {
            strcpy(nodestr, "if (left) right");
            return nodestr;
        }
        case NODE_ELSE: {
            strcpy(nodestr, "if (left) right");
            return nodestr;
        }
        case NODE_WHILE: {
            strcpy(nodestr, "while (left) right");
            return nodestr;
        }
        case NODE_FOR: {
            strcpy(nodestr, "for (left; right; left) right");
            return nodestr;
        }
        case NODE_BLOCK: {
            strcpy(nodestr, "block");
            return nodestr;
        }
        case NODE_CALL_FUNCTION: {
            char func_name[10];
            strncpy(func_name, node->func->name, node->func->len);
            func_name[node->func->len] = '\0';
            sprintf(nodestr, "call function: %s", func_name);
            return nodestr;
        }
        case NODE_ARGUMENT: {
            strcpy(nodestr, "argument");
            return nodestr;
        }
        default: {
            sprintf(nodestr, "category: %d", node->category);
            return nodestr;
        }
    }
}


void show_children(Node *node, int depth) {
    if (node == NULL) {
        fprintf(stderr, "-");
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

void show_node_tree(Node **top_nodes) {
    fprintf(stderr, "========node tree========\n");
    for (int i = 0; top_nodes[i] != NULL; i++) {
        show_children(top_nodes[i], 0);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "=========================\n");
}