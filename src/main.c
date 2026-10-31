#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "argparse.h"

void print_help(ARGPARSE_ARGS* /*ags*/) { printf("argparse help\n"); }

int main(int argc, const char** argv) {
    ARGPARSE_ARGS* args = argparse_new_args(argc, argv);
    ARGPARSE_OPTIONS* opts = argparse_new_options();
    argparse_error ret = argparse_add_option(opts, "--help", print_help);
    if (ret != ARGPARSE_OK) {
        return 1;
    }
    const char* arg = NULL;
    while ((arg = argparse_get_next_arg(args)) != NULL) {
        printf("%s\n", arg);
        argparse_handler_type handler = argparse_get_handler(opts, arg);
        if (handler != NULL) {
            handler(args);
        }
    }

    argparse_free_options(opts);
    argparse_free_args(args);

    return 0;
}
