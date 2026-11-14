#ifndef ARG_PARSE_H
#define ARG_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _argparse_args argparse_args;
typedef struct _argparse_options argparse_options;

typedef enum {
    ARGPARSE_OK = 0,
    ARGPARSE_ERR_NULL,
    ARGPARSE_ERR_ALLOC,
    ARGPARSE_ERR_MISSING_VALUE,
    ARGPARSE_ERR_UNKNOWN_OPTION,
    ARGPARSE_ERR_NOT_SET,
    ARGPARSE_ERR_WRONG_TYPE,
    ARGPARSE_ERR_UNEXPECTED_POSITIONAL,
    ARGPARSE_ERR_MISSING_POSITIONAL,
    ARGPARSE_ERR_UNKNOWN_POSITIONAL,
} argparse_error;

typedef enum {
    ARG_NONE,
    ARG_REQ,
} argparse_argtype;

argparse_error argparse_add_option(argparse_options* opts,
                                   const char* option_string, char short_option,
                                   argparse_argtype type);

argparse_error argparse_add_positional(argparse_options* opts,
                                       const char* name);

argparse_error argparse_parse(argparse_options* opts, argparse_args* args);
argparse_error argparse_has_option(argparse_options* opts, const char* arg,
                                   int* out);
argparse_error argparse_get_option_value(argparse_options* opts,
                                         const char* arg, const char** out);
argparse_error argparse_get_positional_value(argparse_options* opts,
                                             const char* arg, const char** out);

void argparse_print_help(argparse_options* opts);

argparse_args* argparse_new_args(int argc, const char** argv);
void argparse_free_args(argparse_args* args);

argparse_options* argparse_new_options(const char* program);
void argparse_free_options(argparse_options* opts);

#ifdef __cplusplus
}
#endif  // extern "C"
#endif  // !ARG_PARSE_H
