#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "argparse.h"

static int calls = 0;

static void on_flag(ARGPARSE_ARGS* args) {
    (void)args;
    calls++;
}

int main(void) {
    ARGPARSE_OPTIONS* opts = argparse_new_options();
    assert(argparse_add_option(opts, "--flag", on_flag) == ARGPARSE_OK);

    argparse_handler_type handler = argparse_get_handler(opts, "--flag");
    assert(handler == on_flag);
    assert(argparse_get_handler(opts, "--missing") == NULL);

    handler(NULL);
    assert(calls == 1);

    assert(argparse_add_option(NULL, "--x", on_flag) == ARGPARSE_ERR_NULL);
    assert(argparse_add_option(opts, NULL, on_flag) == ARGPARSE_ERR_NULL);
    assert(argparse_get_handler(NULL, "--x") == NULL);
    assert(argparse_get_handler(opts, NULL) == NULL);

    char buf[16];
    strcpy(buf, "--copy");
    assert(argparse_add_option(opts, buf, on_flag) == ARGPARSE_OK);
    strcpy(buf, "--changed"); /* 원본 변경 */
    assert(argparse_get_handler(opts, "--copy") == on_flag);
    assert(argparse_get_handler(opts, "--changed") == NULL);

    argparse_free_options(NULL);
    argparse_free_args(NULL);

    argparse_free_options(opts);
    puts("ok");
    return 0;
}
