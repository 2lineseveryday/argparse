#ifndef ARG_PARSE_H
#define ARG_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct argparse_args ARGPARSE_ARGS;
typedef struct argparse_option ARGPARSE_OPTION;
typedef struct argparse_options ARGPARSE_OPTIONS;

typedef enum {
    ARGPARSE_OK = 0,
    ARGPARSE_ERR_NULL,
    ARGPARSE_ERR_REALLOC,
} argparse_error;

typedef void (*argparse_handler_type)(ARGPARSE_ARGS*);

argparse_error argparse_add_option(ARGPARSE_OPTIONS* opts,
                                   const char* option_string,
                                   argparse_handler_type handler);
const char* argparse_get_next_arg(ARGPARSE_ARGS* args);

ARGPARSE_ARGS* argparse_new_args(int argc, const char** argv);
void argparse_free_args(ARGPARSE_ARGS* args);

ARGPARSE_OPTIONS* argparse_new_options(void);
void argparse_free_options(ARGPARSE_OPTIONS* opts);

argparse_handler_type argparse_get_handler(ARGPARSE_OPTIONS* opts, const char* arg);

#ifdef __cplusplus
}
#endif  // extern "C"
#endif  // !ARG_PARSE_H
