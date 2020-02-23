#include "kcc.h"

Type *int_type = &(Type){INTEGER_TYPE, 4, NULL};

Type *pointer_to(Type *base) {
    Type *pointer = calloc(1, sizeof(Type));
    pointer->category = POINTER_TYPE;
    pointer->size = 8;
    pointer->point_to = base;
    return pointer;
}

bool is_integer(Type *type) {
    return type->category == INTEGER_TYPE;
}

bool is_pointer(Type *type){
    return type->category == POINTER_TYPE;
}