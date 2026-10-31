#include "argparse.h"

#include <stdlib.h>
#include <string.h>

enum {
    OPTIONS_INITIAL_CAPACITY = 4,
};

struct argparse_args {
    int argc;
    int index;
    const char** argv;
};

struct argparse_option {
    char* option_string;
    argparse_handler_type handler;
};

struct argparse_options {
    ARGPARSE_OPTION* options;
    size_t count;
    size_t capacity;
};

static char* argparse_strdup(const char* s) {
    size_t n = strlen(s) + 1;
    char* p = malloc(n);
    if (p) {
        memcpy(p, s, n);
    }
    return p;
}

argparse_error argparse_add_option(ARGPARSE_OPTIONS* opts,
                                   const char* option_string,
                                   argparse_handler_type handler) {
    if (!opts || !option_string || !handler) {
        return ARGPARSE_ERR_NULL;
    }
    char* copy = argparse_strdup(option_string);
    if (!copy) {
        return ARGPARSE_ERR_ALLOC;
    }

    if (opts->count == opts->capacity) {
        size_t new_capacity =
            opts->capacity ? opts->capacity * 2 : OPTIONS_INITIAL_CAPACITY;
        ARGPARSE_OPTION* p =
            realloc(opts->options, sizeof(ARGPARSE_OPTION) * new_capacity);
        if (!p) {
            free(copy);
            return ARGPARSE_ERR_ALLOC;
        }
        opts->options = p;
        opts->capacity = new_capacity;
    }
    ARGPARSE_OPTION option = {.option_string = copy, .handler = handler};
    opts->options[opts->count++] = option;
    return ARGPARSE_OK;
}

const char* argparse_get_next_arg(ARGPARSE_ARGS* args) {
    if (!args) {
        return NULL;
    }

    if (args->index < args->argc) {
        return args->argv[args->index++];
    }
    return NULL;
}

ARGPARSE_ARGS* argparse_new_args(int argc, const char** argv) {
    ARGPARSE_ARGS* args = malloc(sizeof(ARGPARSE_ARGS));
    if (!args) {
        return NULL;
    }

    args->argc = argc;
    args->argv = argv;
    args->index = 0;

    return args;
}

void argparse_free_args(ARGPARSE_ARGS* args) { free(args); }

ARGPARSE_OPTIONS* argparse_new_options(void) {
    ARGPARSE_OPTIONS* opts = malloc(sizeof(ARGPARSE_OPTIONS));
    if (!opts) {
        return NULL;
    }

    opts->options = NULL;
    opts->count = 0;
    opts->capacity = 0;

    return opts;
}

void argparse_free_options(ARGPARSE_OPTIONS* opts) {
    if (opts) {
        for (size_t i = 0; i < opts->count; ++i) {
            free(opts->options[i].option_string);
        }
        free(opts->options);
    }
    free(opts);
}

argparse_handler_type argparse_get_handler(ARGPARSE_OPTIONS* opts,
                                           const char* arg) {
    if (!opts || !arg) {
        return NULL;
    }
    for (size_t i = 0; i < opts->count; ++i) {
        if (strcmp(arg, opts->options[i].option_string) == 0) {
            return opts->options[i].handler;
        }
    }
    return NULL;
}
