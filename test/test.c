int printf(char* str, int a, int b);

int assert(int expected, int actual) {
    if (expected != actual) {
        printf("%d expected but got %d why?\n", expected, actual);
        return 1;
    }
    printf("%d == %d is OK\n", expected, actual);
    return 0;
}

int main() {
    // line comment
    return assert(5, 2+3);
}