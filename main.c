#include "kcc.h"

int main(int argc, char **argv) {
    int input_index = 1;
    bool show_debug_info = false;
    if (argc == 3) {
        show_debug_info = true;
        input_index++;
    }

    user_input = argv[input_index];
    token = tokenize(user_input);
    if (show_debug_info) {
        show_tokens(token);
    }
    top_node = expression();
    if (show_debug_info) {
        show_node_tree(top_node);
    }
    generate_assembly(top_node);
    
    return 0;
}