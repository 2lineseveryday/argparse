#ifndef ARG_PARSE_H
#define ARG_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

struct ARGS;
struct OPTION;
struct OPTIONS;

typedef void (*HANDLER_TYPE)(struct ARGS*);

void argparse_add_option(struct OPTIONS* opts, const char* option_string,
                         HANDLER_TYPE handler);
const char* argparse_get_next_arg(struct ARGS* args);

struct ARGS* argparse_new_args(int argc, const char** argv);
void argparse_free_args(struct ARGS* args);

struct OPTIONS* argparse_new_options(void);
void argparse_free_options(struct OPTIONS* opts);

HANDLER_TYPE argparse_get_handler(struct OPTIONS* opts, const char* arg);

#ifdef __cplusplus
}
#endif  // extern "C"
#endif  // !ARG_PARSE_H
