#include "kcc.h"

Type *int_type = &(Type){INTEGER_TYPE, 4, NULL};
Type *char_type = &(Type){CHAR_TYPE, 1, NULL};

Type *pointer_to(Type *base) {
    Type *pointer = calloc(1, sizeof(Type));
    pointer->category = POINTER_TYPE;
    pointer->size = 8;
    pointer->point_to = base;
    return pointer;
}

Type *array_of(Type *base, int number) {
    Type *array = calloc(1, sizeof(Type));
    array->category = ARRAY_TYPE;
    array->size = base->size * number;
    array->point_to = base;
    return array;
}

bool is_integer(Type *type) {
    return type->category == INTEGER_TYPE;
}

bool is_character(Type *type) {
    return type->category == CHAR_TYPE;
}

bool is_pointer(Type *type){
    return type->category == POINTER_TYPE;
}

bool is_array(Type *type){
    return type->category == ARRAY_TYPE;
}

bool is_pointer_or_array(Type *type){
    return is_pointer(type) || is_array(type);
}

bool is_character_or_integer(Type *type) {
    return is_integer(type) || is_character(type);
}