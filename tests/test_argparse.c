#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "argparse.h"

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define ADD_OPT(opts, name, type) \
    assert(argparse_add_option(opts, name, type) == ARGPARSE_OK)

/* "prog"는 argv[0] 자리. 인자는 가변 인자로 넘긴다. */
#define PARSE_OK(opts, ...)                                                 \
    do {                                                                    \
        const char* av_[] = {"prog", __VA_ARGS__};                          \
        ARGPARSE_ARGS* args_ = argparse_new_args((int)ARRAY_LEN(av_), av_); \
        argparse_error err_ = argparse_parse(opts, args_);                  \
        argparse_free_args(args_);                                          \
        assert(err_ == ARGPARSE_OK);                                        \
    } while (0)

#define PARSE_ERR(opts, expected, ...)                                      \
    do {                                                                    \
        const char* av_[] = {"prog", __VA_ARGS__};                          \
        ARGPARSE_ARGS* args_ = argparse_new_args((int)ARRAY_LEN(av_), av_); \
        argparse_error err_ = argparse_parse(opts, args_);                  \
        argparse_free_args(args_);                                          \
        assert(err_ == (expected));                                         \
    } while (0)

#define VALUE_IS(opts, name, expected)                                     \
    do {                                                                   \
        const char* v_ = NULL;                                             \
        assert(argparse_get_option_value(opts, name, &v_) == ARGPARSE_OK); \
        assert(v_ && strcmp(v_, expected) == 0);                           \
    } while (0)

#define VALUE_ERR(opts, name, expected)                                   \
    do {                                                                  \
        const char* v_ = NULL;                                            \
        assert(argparse_get_option_value(opts, name, &v_) == (expected)); \
    } while (0)

int main(void) {
    ARGPARSE_OPTIONS* opts = argparse_new_options("test_argparse");
    ADD_OPT(opts, "flag", ARG_REQ);
    ADD_OPT(opts, "verbose", ARG_NONE);

    /* parse */
    PARSE_OK(opts, "--flag=hello", "--verbose");
    VALUE_IS(opts, "flag", "hello");
    VALUE_IS(opts, "verbose", "1");
    VALUE_ERR(opts, "missing", ARGPARSE_ERR_UNKNOWN_OPTION);

    PARSE_ERR(opts, ARGPARSE_ERR_UNKNOWN_OPTION, "--falg", "world");
    PARSE_ERR(opts, ARGPARSE_ERR_UNKNOWN_OPTION, "--falg=world");

    /* 공백 형태 */
    PARSE_OK(opts, "--flag", "world");
    VALUE_IS(opts, "flag", "world");

    /* 반복 옵션 */
    PARSE_OK(opts, "--flag=a", "--flag=b");
    VALUE_IS(opts, "flag", "b");

    /* 값 누락 */
    PARSE_ERR(opts, ARGPARSE_ERR_MISSING_VALUE, "--flag");

    /* 긴 인자 (이전 스택 오버플로 회귀) */
    PARSE_OK(opts, "--flag=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    VALUE_IS(opts, "flag", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    /* 빈 값 */
    PARSE_OK(opts, "--flag=");
    VALUE_IS(opts, "flag", "");

    argparse_free_args(NULL);
    argparse_free_options(NULL);
    argparse_free_options(opts);
    puts("ok");
    return 0;
}
