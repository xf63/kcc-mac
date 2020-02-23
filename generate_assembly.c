#include "kcc.h"

int syntax_number;
char *node2str(Node *node);
void generate(Node *node);
char argument_register_8byte[10][10] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char argument_register_4byte[10][10] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char func_name[10];

char *get_func_name(Node *node) {
    strncpy(func_name, node->func->name, node->func->len);
    func_name[node->func->len] = '\0';
    return func_name;
}

void load(Type *type) {
    printf("  pop rax\n");
    if (type->size == 4) {
        printf("  mov eax, [rax]\n");
    }
    else if (type->size == 8) {
        printf("  mov rax, [rax]\n");
    }
    else {
        error("undefined type");
    }
    printf("  push rax\n");
}

void store(Type *type) {
    printf("  pop rdi\n");
    printf("  pop rax\n");
    if (type->size == 4) {
        printf("  mov [rax], edi\n");
    }
    else if (type->size == 8) {
        printf("  mov [rax], rdi\n");
    }
    else {
        error("undefined type");
    }
    printf("  push rdi\n");
}

void generate_stack_value(Node *node) {
    if (node->category == NODE_DEREFERENCE) {
        generate(node->rhs);
        return;
    }
    if (node->category == NODE_ACCESS_VARIABLE || node->category == NODE_DEFINE_VARIABLE) {
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", node->var->offset);
        printf("  push rax\n");
        return;
    }
    error("local variable or address expected, but got %s", node2str(node));
}

void generate(Node *node) {
    if (node == NULL) {
        return;
    }
    switch (node->category) {
        case NODE_ACCESS_VARIABLE:
            generate_stack_value(node);
            load(node->type);
            return;
        case NODE_ASSIGN:
            generate_stack_value(node->lhs);
            generate(node->rhs);
            store(node->type);
            return;
        case NODE_RETURN: {
            generate(node->rhs);
            printf("  pop rax\n");
            printf("  jmp Lend%s\n", get_func_name(node));
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
            for (Node *arg_node = node->lhs; arg_node != NULL; arg_node = arg_node->rhs) {
                generate(arg_node->lhs);
                printf("  pop %s\n", argument_register_8byte[arg_num]);
                arg_num++;
            }
            printf("  call _%s\n", get_func_name(node));
            printf("  push rax\n");
            return;
        }
        case NODE_DEFINE_FUNCTION: {
            if (node->rhs == NULL) {
                return;
            }
            printf("_%s:\n", get_func_name(node));
            printf("  push rbp\n");
            printf("  mov rbp, rsp\n");
            // local value stack allocation
            if (node->func->last != NULL) {
                printf("  sub rsp, %d\n", node->func->last->offset);
            }
            int arg_num = 0;
            for (Node *arg_node = node->lhs; arg_node != NULL; arg_node = arg_node->rhs) {
                generate_stack_value(arg_node->lhs);
                printf("  push %s\n", argument_register_8byte[arg_num]);
                store(arg_node->lhs->type);
                arg_num++;
            }
            generate(node->rhs);
            printf("  pop rax\n");
            printf("Lend%s:\n", get_func_name(node));
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;
        }
        case NODE_DEREFERENCE: {
            generate(node->rhs);
            load(node->type);
            return;
        }
        case NODE_ADDRESS_OF: {
            generate_stack_value(node->rhs);
            return;
        }
        case NODE_DEFINE_VARIABLE: {
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
        case NODE_ADD_INTEGER: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  add rax, rdi\n");
            printf("  push rax\n");
            return;
        }
        case NODE_ADD_POINTER: {
            printf("  pop rax\n");
            printf("  mov rdi, %d\n", node->lhs->type->point_to->size); // rdi = size of left hand side
            printf("  mul rdi\n"); // rax = rax * rdi
            printf("  pop rdi\n");
            printf("  add rdi, rax\n");
            printf("  push rdi\n");
            return;
        }
        case NODE_SUB_INTEGER: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  sub rax, rdi\n");
            printf("  push rax\n");
            return;
        }
        case NODE_SUB_POINTER: {
            printf("  pop rax\n");
            printf("  mov rdi, %d\n", node->lhs->type->point_to->size); // rdi = size of left hand side
            printf("  mul rdi\n"); // rax = rax * rdi
            printf("  pop rdi\n");
            printf("  sub rdi, rax\n");
            printf("  push rdi\n");
            return;
        }
        case NODE_DIFF_POINTER: {
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  sub rax, rdi\n");
            printf("  mov rdi, %d\n", node->lhs->type->size); // rdi = size of left hand side
            printf("  cqo\n");
            printf("  idiv rdi\n");
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
    // main
    for (int i = 0; top_nodes[i] != NULL; i++) {
        generate(top_nodes[i]);
    }
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
        case NODE_ADD_INTEGER: {
            strcpy(nodestr, "(+)");
            return nodestr;
        }
        case NODE_ADD_POINTER: {
            sprintf(nodestr, "(+) pointer + int * %d", node->lhs->type->point_to->size);
            return nodestr;
        }
        case NODE_SUB_INTEGER: {
            strcpy(nodestr, "(-)");
            return nodestr;
        }
        case NODE_SUB_POINTER: {
            sprintf(nodestr, "(-) pointer - int * %d", node->lhs->type->point_to->size);
            return nodestr;
        }
        case NODE_DIFF_POINTER: {
            strcpy(nodestr, "(-) between pointer");
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
        case NODE_ACCESS_VARIABLE: {
            sprintf(nodestr, "local variable offset: %d, size: %d", node->var->offset, node->type->size);
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
            sprintf(nodestr, "call function: %s", get_func_name(node));
            return nodestr;
        }
        case NODE_DEFINE_FUNCTION: {
            sprintf(nodestr, "define function: %s", get_func_name(node));
            return nodestr;
        }
        case NODE_ARGUMENT: {
            strcpy(nodestr, "argument");
            return nodestr;
        }
        case NODE_DEREFERENCE: {
            strcpy(nodestr, "dereference");
            return nodestr;
        }
        case NODE_ADDRESS_OF: {
            strcpy(nodestr, "address of");
            return nodestr;
        }
        case NODE_DEFINE_VARIABLE: {
            sprintf(nodestr, "define variable offset: %d, size: %d", node->var->offset, node->type->size);
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