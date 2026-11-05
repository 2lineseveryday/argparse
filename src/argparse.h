#ifndef ARG_PARSE_H
#define ARG_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct argparse_args ARGPARSE_ARGS;
typedef struct argparse_options ARGPARSE_OPTIONS;

typedef enum {
    ARGPARSE_OK = 0,
    ARGPARSE_ERR_NULL,
    ARGPARSE_ERR_ALLOC,
    ARGPARSE_ERR_MISSING_VALUE,
    ARGPARSE_ERR_UNKNOWN_OPTION,
    ARGPARSE_ERR_NOT_SET,
} argparse_error;

typedef enum {
    ARG_NONE,
    ARG_REQ,
} argparse_argtype;

argparse_error argparse_add_option(ARGPARSE_OPTIONS* opts,
                                   const char* option_string,
                                   argparse_argtype type);

argparse_error argparse_parse(ARGPARSE_OPTIONS* opts, ARGPARSE_ARGS* args);
argparse_error argparse_get_option_value(ARGPARSE_OPTIONS* opts,
                                         const char* option_string,
                                         const char** out);

void argparse_print_help(ARGPARSE_OPTIONS* opts);

ARGPARSE_ARGS* argparse_new_args(int argc, const char** argv);
void argparse_free_args(ARGPARSE_ARGS* args);

ARGPARSE_OPTIONS* argparse_new_options(const char* program);
void argparse_free_options(ARGPARSE_OPTIONS* opts);

#ifdef __cplusplus
}
#endif  // extern "C"
#endif  // !ARG_PARSE_H
