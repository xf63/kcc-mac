#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "only 2 argument is acceptable\n");
        return 1;
    }
    
    // prologue
    printf(".intel_syntax noprefix\n");
    printf(".section	__TEXT,__text,regular,pure_instructions\n");
    printf(".macosx_version_min 10, 10\n");
    printf(".globl	_main\n");
    printf("_main:\n");
    // main
    printf("  mov rax, %d\n", atoi(argv[1]));
    printf("  ret\n");
    return 0;
}