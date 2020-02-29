#include "kcc.h"
#include "unistd.h"

char *read_file(char *path);

int main(int argc, char **argv) {
    int input_index = 1;
    bool direct_input = false;
    bool show_token_list = false;
    bool show_parse_tree = false;
    int option;
    while ((option = getopt(argc, argv, "dtp")) != -1) {
        switch (option) {
            case 'd': {
                direct_input = true;
                input_index++;
                break;
            }
            case 't': {
                show_token_list = true;
                input_index++;
                break;
            }
            case 'p': {
                show_parse_tree = true;
                input_index++;
                break;
            }
            default :
                error("Usage: kcc [-d] [-t] [-p] file [file..]");
        }
    }
    if (direct_input) {
        user_input = argv[argc - 1];
    }
    else {
        user_input = read_file(argv[argc - 1]);
    }
    token = tokenize(user_input);
    if (show_token_list) {
        show_tokens(token);
    }
    Node **nodes = program();
    if (show_parse_tree) {
        show_node_tree(nodes);
    }
    generate_assembly(nodes);
    
    return 0;
}

char *read_file(char *path) {
    FILE *fpointer = fopen(path, "r");
    if (fpointer == NULL) {
        error("cannot open %s: %s", path, strerror(errno));
    }

    if (fseek(fpointer, 0, SEEK_END) == -1) {
        error("%s: fseek : %s", path, strerror(errno));
    }
    size_t size = ftell(fpointer);
    if (fseek(fpointer, 0, SEEK_SET) == -1) {
        error("%s: fseek : %s", path, strerror(errno));
    }

    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fpointer);

    if (size == 0 || buf[size - 1] != '\n') {
        buf[size] = '\n';
        size++;
    }
    buf[size] = '\0';
    fclose(fpointer);
    return buf;
}
