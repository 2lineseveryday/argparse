#include <libgen.h>
#include <stdlib.h>
#include <string.h>

#include "argparse.h"

int main(int argc, const char** argv) {
    int rc = 1;
    char* program = NULL;
    ARGPARSE_OPTIONS* opts = NULL;
    ARGPARSE_ARGS* args = NULL;

    size_t program_size = strlen(argv[0]) + 1;
    program = malloc(program_size);
    if (!program) {
        goto cleanup;
    }
    memcpy(program, argv[0], program_size);

    opts = argparse_new_options(basename(program));
    if (!opts) {
        goto cleanup;
    }
    if (argparse_add_option(opts, "help", ARG_NONE) != ARGPARSE_OK ||
        argparse_add_option(opts, "flag", ARG_REQ) != ARGPARSE_OK ||
        argparse_add_positional(opts, "input") != ARGPARSE_OK ||
        argparse_add_positional(opts, "output") != ARGPARSE_OK) {
        goto cleanup;
    }

    args = argparse_new_args(argc - 1, argv + 1);
    if (!args) {
        goto cleanup;
    }
    if (argparse_parse(opts, args) != ARGPARSE_OK) {
        goto cleanup;
    }

    argparse_print_help(opts);
    rc = 0;

cleanup:
    argparse_free_args(args);
    argparse_free_options(opts);
    free(program);
    return rc;
}
