#include <stdio.h>
#include <string.h>

#include "argparse.h"

void print_help(void) { printf("argparse help\n"); }

int main(int argc, const char** argv) {
    ARGPARSE_OPTIONS* opts = argparse_new_options(argv[0]);
    argparse_error ret = argparse_add_option(opts, "help", ARG_NONE);
    if (ret != ARGPARSE_OK) {
        return 1;
    }
    ret = argparse_add_option(opts, "flag", ARG_REQ);
    if (ret != ARGPARSE_OK) {
        return 1;
    }

    ARGPARSE_ARGS* args = argparse_new_args(argc, argv);
    ret = argparse_parse(opts, args);
    if (ret != ARGPARSE_OK) {
        return 1;
    }

    argparse_print_help(opts);


    argparse_free_options(opts);
    argparse_free_args(args);

    return 0;
}
