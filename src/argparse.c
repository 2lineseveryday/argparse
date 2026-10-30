#include "argparse.h"

#include <stdlib.h>
#include <string.h>

struct ARGS {
    int argc;
    int index;
    const char** argv;
};

struct OPTION {
    const char* option_string;
    void (*handler)(struct ARGS* args);
};

struct OPTIONS {
    struct OPTION* options;
    int count;
    int capacity;
};

argparse_error argparse_add_option(struct OPTIONS* opts,
                                   const char* option_string,
                                   HANDLER_TYPE handler) {
    if (!opts || !option_string || !handler) {
        return ARGPARSE_ERR_NULL;
    }
    struct OPTION option = {.option_string = option_string, .handler = handler};

    if (opts->count == opts->capacity) {
        int new_capacity = opts->capacity ? opts->capacity * 2 : 4;
        struct OPTION* p =
            realloc(opts->options, sizeof(struct OPTION) * new_capacity);
        if (!p) {
            return ARGPARSE_ERR_REALLOC;
        }
        opts->options = p;
        opts->capacity = new_capacity;
    }
    opts->options[opts->count++] = option;
    return ARGPARSE_OK;
}

const char* argparse_get_next_arg(struct ARGS* args) {
    if (!args) {
        return NULL;
    }

    if (args->index < args->argc) {
        return args->argv[args->index++];
    }
    return NULL;
}

struct ARGS* argparse_new_args(int argc, const char** argv) {
    struct ARGS* args = malloc(sizeof(struct ARGS));
    if (!args) {
        return NULL;
    }

    args->argc = argc;
    args->argv = argv;
    args->index = 0;

    return args;
}

void argparse_free_args(struct ARGS* args) { free(args); }

struct OPTIONS* argparse_new_options(void) {
    struct OPTIONS* opts = malloc(sizeof(struct OPTIONS));
    if (!opts) {
        return NULL;
    }

    opts->options = NULL;
    opts->count = 0;
    opts->capacity = 0;

    return opts;
}

void argparse_free_options(struct OPTIONS* opts) {
    if (opts) {
        free(opts->options);
    }

    free(opts);
}

HANDLER_TYPE argparse_get_handler(struct OPTIONS* opts, const char* arg) {
    if (!opts || !arg) {
        return NULL;
    }
    for (int i = 0; i < opts->count; ++i) {
        if (strncmp(arg, opts->options[i].option_string,
                    strlen(opts->options[i].option_string) + 1) == 0) {
            return opts->options[i].handler;
        }
    }
    return NULL;
}
