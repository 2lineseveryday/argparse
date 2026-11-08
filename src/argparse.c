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
    int end_options;
};

typedef struct argparse_option {
    argparse_argtype type;
    int present;
    char* option_string;
    char* option_value;

} ARGPARSE_OPTION;

typedef struct argparse_positional {
    int present;
    char* option_string;
    char* option_value;

} ARGPARSE_POSITIONAL;

struct argparse_options {
    char* program;

    ARGPARSE_OPTION* options;
    size_t options_count;
    size_t options_capacity;

    ARGPARSE_POSITIONAL* positional;
    size_t positional_count;
    size_t positional_capacity;
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
    if (!s) {
        return NULL;
    }
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

static argparse_error argparse_grow_array(void** data, size_t* capacity,
                                          size_t element_size) {
    size_t new_capacity = *capacity ? *capacity * 2 : OPTIONS_INITIAL_CAPACITY;
    void* p = realloc(*data, element_size * new_capacity);
    if (!p) {
        return ARGPARSE_ERR_ALLOC;
    }
    *data = p;
    *capacity = new_capacity;
    return ARGPARSE_OK;
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

    if (opts->options_count == opts->options_capacity) {
        argparse_error err =
            argparse_grow_array((void**)&opts->options, &opts->options_capacity,
                                sizeof(ARGPARSE_OPTION));
        if (err != ARGPARSE_OK) {
            free(copy);
            return err;
        }
    }
    ARGPARSE_OPTION option = {
        .option_string = copy,
        .option_value = NULL,
        .type = type,
        .present = 0,
    };
    opts->options[opts->options_count++] = option;
    return ARGPARSE_OK;
}
argparse_error argparse_add_positional(ARGPARSE_OPTIONS* opts,
                                       const char* option_string) {
    if (!opts || !option_string) {
        return ARGPARSE_ERR_NULL;
    }
    char* copy = argparse_strdup(option_string);
    if (!copy) {
        return ARGPARSE_ERR_ALLOC;
    }

    if (opts->positional_count == opts->positional_capacity) {
        argparse_error err = argparse_grow_array((void**)&opts->positional,
                                                 &opts->positional_capacity,
                                                 sizeof(ARGPARSE_POSITIONAL));
        if (err != ARGPARSE_OK) {
            free(copy);
            return err;
        }
    }
    ARGPARSE_POSITIONAL option = {
        .option_string = copy,
        .option_value = NULL,
        .present = 0,
    };
    opts->positional[opts->positional_count++] = option;
    return ARGPARSE_OK;
}

static ARGPARSE_POSITIONAL* argparse_get_positional(ARGPARSE_OPTIONS* opts,
                                                    const char* arg) {
    if (!opts || !arg) {
        return NULL;
    }

    for (size_t i = 0; i < opts->positional_count; ++i) {
        if (strcmp(arg, opts->positional[i].option_string) == 0) {
            return &opts->positional[i];
        }
    }

    return NULL;
}

static ARGPARSE_OPTION* argparse_get_option(ARGPARSE_OPTIONS* opts,
                                            const char* arg) {
    if (!opts || !arg) {
        return NULL;
    }
    while (*arg == '-') {
        arg++;
    }
    for (size_t i = 0; i < opts->options_count; ++i) {
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

    if (opt->option_value) {
        free(opt->option_value);
    }
    opt->option_value = copy;
    opt->present = 1;

    return ARGPARSE_OK;
}

static argparse_error argparse_assign_option(ARGPARSE_OPTIONS* opts,
                                             ARGPARSE_ARGS* args,
                                             const char* arg) {
    if (!opts || !args || !arg) {
        return ARGPARSE_ERR_NULL;
    }

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
static argparse_error argparse_assign_positional(ARGPARSE_OPTIONS* opts,
                                                 ARGPARSE_ARGS* args,
                                                 const char* arg) {
    if (!opts || !args || !arg) {
        return ARGPARSE_ERR_NULL;
    }

    char* copy = argparse_strdup(arg);
    if (!copy) {
        return ARGPARSE_ERR_ALLOC;
    }
    size_t i = 0;
    for (i = 0; i < opts->positional_count; ++i) {
        if (opts->positional[i].present == 0) {
            opts->positional[i].option_value = copy;
            opts->positional[i].present = 1;
            break;
        }
    }
    if (i == opts->positional_count) {
        free(copy);
        return ARGPARSE_ERR_UNEXPECTED_POSITIONAL;
    }
    return ARGPARSE_OK;
}
static argparse_error argparse_handle_arg(ARGPARSE_OPTIONS* opts,
                                          ARGPARSE_ARGS* args,
                                          const char* arg) {
    if (!opts || !args || !arg) {
        return ARGPARSE_ERR_NULL;
    }

    if (args->end_options || arg[0] != '-' || arg[1] == '\0') {
        return argparse_assign_positional(opts, args, arg);
    }
    if (strcmp(arg, "--") == 0) {
        args->end_options = 1;
        return ARGPARSE_OK;
    }
    return argparse_assign_option(opts, args, arg);
}

static void argparse_reset_options(ARGPARSE_OPTIONS* opts) {
    for (size_t i = 0; i < opts->options_count; ++i) {
        free(opts->options[i].option_value);
        opts->options[i].option_value = NULL;
        opts->options[i].present = 0;
    }
    for (size_t i = 0; i < opts->positional_count; ++i) {
        free(opts->positional[i].option_value);
        opts->positional[i].option_value = NULL;
        opts->positional[i].present = 0;
    }
}

argparse_error argparse_parse(ARGPARSE_OPTIONS* opts, ARGPARSE_ARGS* args) {
    if (!opts || !args) {
        return ARGPARSE_ERR_NULL;
    }

    argparse_reset_options(opts);

    argparse_error err = ARGPARSE_OK;
    const char* arg = NULL;

    while ((arg = argparse_get_next_arg(args))) {
        err = argparse_handle_arg(opts, args, arg);
        if (err != ARGPARSE_OK) {
            return err;
        }
    }
    for (size_t i = 0; i < opts->positional_count; ++i) {
        if (!opts->positional[i].present) {
            return ARGPARSE_ERR_MISSING_POSITIONAL;
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

argparse_error argparse_get_positional_value(ARGPARSE_OPTIONS* opts,
                                             const char* arg,
                                             const char** out) {
    if (!opts || !arg || !out) {
        return ARGPARSE_ERR_NULL;
    }
    ARGPARSE_POSITIONAL* opt = argparse_get_positional(opts, arg);
    if (!opt) {
        return ARGPARSE_ERR_UNKNOWN_POSITIONAL;
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
    printf("%s", opts->program);

    for (size_t i = 0; i < opts->options_count; ++i) {
        if (opts->options[i].type == ARG_NONE) {
            printf(" [--%s]", opts->options[i].option_string);
        } else if (opts->options[i].type == ARG_REQ) {
            printf(" [--%s=value]", opts->options[i].option_string);
        }
    }
    for (size_t i = 0; i < opts->positional_count; ++i) {
        printf(" <%s>", opts->positional[i].option_string);
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
    args->end_options = 0;

    return args;
}

void argparse_free_args(ARGPARSE_ARGS* args) { free(args); }

ARGPARSE_OPTIONS* argparse_new_options(const char* program) {
    if (!program) {
        return NULL;
    }
    ARGPARSE_OPTIONS* opts = malloc(sizeof(ARGPARSE_OPTIONS));
    if (!opts) {
        return NULL;
    }

    opts->program = argparse_strdup(program);
    if (!opts->program) {
        free(opts);
        return NULL;
    }

    opts->options = NULL;
    opts->options_count = 0;
    opts->options_capacity = 0;

    opts->positional = NULL;
    opts->positional_count = 0;
    opts->positional_capacity = 0;

    return opts;
}

void argparse_free_options(ARGPARSE_OPTIONS* opts) {
    if (opts) {
        for (size_t i = 0; i < opts->options_count; ++i) {
            free(opts->options[i].option_string);
            free(opts->options[i].option_value);
        }
        free(opts->options);

        for (size_t i = 0; i < opts->positional_count; ++i) {
            free(opts->positional[i].option_string);
            free(opts->positional[i].option_value);
        }
        free(opts->positional);

        free(opts->program);
    }
    free(opts);
}
