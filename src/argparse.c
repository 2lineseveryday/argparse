#include "argparse.h"

#include <stdio.h>
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

typedef struct argparse_option {
    argparse_argtype type;
    char* option_string;
    char* option_value;
    int present;
} ARGPARSE_OPTION;

struct argparse_options {
    const char* program;
    ARGPARSE_OPTION* options;
    size_t count;
    size_t capacity;
};

static char* argparse_str_split(char* s) {
    char* pos = strchr(s, '=');
    if (pos) {
        *pos = '\0';
        return (pos + 1);
    }
    return NULL;
}

static char* argparse_strdup(const char* s) {
    size_t n = strlen(s) + 1;
    char* p = malloc(n);
    if (p) {
        memcpy(p, s, n);
    }
    return p;
}

static const char* argparse_get_next_arg(ARGPARSE_ARGS* args) {
    if (!args) {
        return NULL;
    }

    if (args->index < args->argc) {
        return args->argv[args->index++];
    }
    return NULL;
}

argparse_error argparse_add_option(ARGPARSE_OPTIONS* opts,
                                   const char* option_string,
                                   argparse_argtype type) {
    if (!opts || !option_string) {
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
    ARGPARSE_OPTION option = {
        .option_string = copy,
        .option_value = NULL,
        .type = type,
        .present = 0,
    };
    opts->options[opts->count++] = option;
    return ARGPARSE_OK;
}

static ARGPARSE_OPTION* argparse_get_option(ARGPARSE_OPTIONS* opts,
                                            const char* arg) {
    if (!opts || !arg) {
        return NULL;
    }
    while (*arg == '-') {
        arg++;
    }
    for (size_t i = 0; i < opts->count; ++i) {
        if (strcmp(arg, opts->options[i].option_string) == 0) {
            return &opts->options[i];
        }
    }
    return NULL;
}

static argparse_error argparse_set_option_value(ARGPARSE_OPTION* opt,
                                                const char* value) {
    char* copy = argparse_strdup(value);
    if (!copy) {
        return ARGPARSE_ERR_ALLOC;
    }

    free(opt->option_value);
    opt->option_value = copy;
    opt->present = 1;

    return ARGPARSE_OK;
}

static argparse_error argparse_handle_arg(ARGPARSE_OPTIONS* opts,
                                          ARGPARSE_ARGS* args,
                                          const char* arg) {
    argparse_error err = ARGPARSE_OK;
    char* copy = argparse_strdup(arg);
    if (!copy) {
        return ARGPARSE_ERR_ALLOC;
    }

    char* key = copy;
    char* value = argparse_str_split(copy);

    ARGPARSE_OPTION* opt = argparse_get_option(opts, key);
    if (!opt) {
        if (key[0] == '-') {
            err = ARGPARSE_ERR_UNKNOWN_OPTION;
        }
    } else if (opt->type == ARG_REQ) {
        if (!value) {
            const char* next = argparse_get_next_arg(args);
            if (!next || (next[0] == '-' && argparse_get_option(opts, next))) {
                err = ARGPARSE_ERR_MISSING_VALUE;
            } else {
                value = (char*)next;
            }
        }
        if (err == ARGPARSE_OK) {
            err = argparse_set_option_value(opt, value);
        }
        if (err != ARGPARSE_OK) {
            free(opt->option_value);
            opt->option_value = NULL;
            opt->present = 0;
        }
    } else {
        opt->present = 1;
    }

    free(copy);
    return err;
}

argparse_error argparse_parse(ARGPARSE_OPTIONS* opts, ARGPARSE_ARGS* args) {
    if (!opts || !args) {
        return ARGPARSE_ERR_NULL;
    }

    argparse_error err = ARGPARSE_OK;
    const char* arg = NULL;

    while ((arg = argparse_get_next_arg(args))) {
        err = argparse_handle_arg(opts, args, arg);
        if (err != ARGPARSE_OK) {
            return err;
        }
    }

    return ARGPARSE_OK;
}

argparse_error argparse_has_option(ARGPARSE_OPTIONS* opts, const char* arg,
                                   int* out) {
    if (!opts || !arg || !out) {
        return ARGPARSE_ERR_NULL;
    }
    ARGPARSE_OPTION* opt = argparse_get_option(opts, arg);
    if (!opt) {
        return ARGPARSE_ERR_UNKNOWN_OPTION;
    }
    *out = opt->present;
    return ARGPARSE_OK;
}

argparse_error argparse_get_option_value(ARGPARSE_OPTIONS* opts,
                                         const char* arg, const char** out) {
    if (!opts || !arg || !out) {
        return ARGPARSE_ERR_NULL;
    }
    ARGPARSE_OPTION* opt = argparse_get_option(opts, arg);
    if (!opt) {
        return ARGPARSE_ERR_UNKNOWN_OPTION;
    }
    if (opt->type != ARG_REQ) {
        return ARGPARSE_ERR_WRONG_TYPE;
    }
    if (!opt->present) {
        return ARGPARSE_ERR_NOT_SET;
    }
    *out = opt->option_value;
    return ARGPARSE_OK;
}

void argparse_print_help(ARGPARSE_OPTIONS* opts) {
    if (!opts) {
        return;
    }
    printf("%s ", opts->program);

    for (size_t i = 0; i < opts->count; ++i) {
        if (opts->options[i].type == ARG_NONE) {
            printf("[--%s]", opts->options[i].option_string);
        } else if (opts->options[i].type == ARG_REQ) {
            printf("[--%s=value]", opts->options[i].option_string);
        }
    }
    printf("\n");
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

ARGPARSE_OPTIONS* argparse_new_options(const char* program) {
    ARGPARSE_OPTIONS* opts = malloc(sizeof(ARGPARSE_OPTIONS));
    if (!opts) {
        return NULL;
    }

    opts->program = program;

    opts->options = NULL;
    opts->count = 0;
    opts->capacity = 0;

    return opts;
}

void argparse_free_options(ARGPARSE_OPTIONS* opts) {
    if (opts) {
        for (size_t i = 0; i < opts->count; ++i) {
            free(opts->options[i].option_string);
            free(opts->options[i].option_value);
        }
        free(opts->options);
    }
    free(opts);
}
