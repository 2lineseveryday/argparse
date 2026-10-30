#include "argparse.h"

#include <assert.h>
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

void argparse_add_option(struct OPTIONS* opts, const char* option_string,
                         HANDLER_TYPE handler) {
    assert(opts != NULL);
    struct OPTION option = {.option_string = option_string, .handler = handler};

    if (opts->count == opts->capacity) {
        if (opts->capacity == 0) {
            opts->capacity = 1;
        } else {
            opts->capacity++;
        }
        opts->options =
            realloc(opts->options, sizeof(struct OPTION) * opts->capacity);
        assert(opts->options != NULL);
    }
    opts->options[opts->count++] = option;
}

const char* argparse_get_next_arg(struct ARGS* args) {
    assert(args != NULL);
    if (args->index < args->argc) {
        return args->argv[args->index++];
    }
    return NULL;
}

struct ARGS* argparse_new_args(int argc, const char** argv) {
    struct ARGS* args = malloc(sizeof(struct ARGS));
    assert(args != NULL);

    args->argc = argc;
    args->argv = argv;
    args->index = 0;

    return args;
}

void argparse_free_args(struct ARGS* args) {
    assert(args != NULL);
    free(args);
}

struct OPTIONS* argparse_new_options(void) {
    struct OPTIONS* opts = malloc(sizeof(struct OPTIONS));
    assert(opts != NULL);

    opts->options = NULL;
    opts->count = 0;
    opts->capacity = 0;

    return opts;
}

void argparse_free_options(struct OPTIONS* opts) {
    assert(opts != NULL);

    if (opts->options) {
        free(opts->options);
    }

    free(opts);
}

HANDLER_TYPE argparse_get_handler(struct OPTIONS* opts, const char* arg) {
    for (int i = 0; i < opts->count; ++i) {
        if (strncmp(arg, opts->options[i].option_string,
                    strlen(opts->options[i].option_string) + 1) == 0) {
            return opts->options[i].handler;
        }
    }
    return NULL;
}
