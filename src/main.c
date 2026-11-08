#include <libgen.h>
#include <stdlib.h>
#include <string.h>

#include "argparse.h"

int main(int argc, const char** argv) {
    char* program = malloc(strlen(argv[0]) + 1);
    memcpy(program, argv[0], strlen(argv[0]));
    program[strlen(argv[0])] = '\0';

    ARGPARSE_OPTIONS* opts = argparse_new_options(basename(program));
    argparse_error ret = argparse_add_option(opts, "help", ARG_NONE);
    if (ret != ARGPARSE_OK) {
        return 1;
    }
    ret = argparse_add_option(opts, "flag", ARG_REQ);
    if (ret != ARGPARSE_OK) {
        return 1;
    }

    ret = argparse_add_positional(opts, "input");
    if (ret != ARGPARSE_OK) {
        return 1;
    }
    ret = argparse_add_positional(opts, "output");
    if (ret != ARGPARSE_OK) {
        return 1;
    }

    ARGPARSE_ARGS* args = argparse_new_args(argc - 1, argv + 1);
    ret = argparse_parse(opts, args);
    if (ret != ARGPARSE_OK) {
        return 1;
    }

    argparse_print_help(opts);

    argparse_free_options(opts);
    argparse_free_args(args);

    free(program);

    return 0;
}
