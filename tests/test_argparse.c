#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "argparse.h"

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define ADD_OPT(opts, name, short, type) \
    assert(argparse_add_option(opts, name, short, type) == ARGPARSE_OK)

#define ADD_POS(opts, name) \
    assert(argparse_add_positional(opts, name) == ARGPARSE_OK)

/* "prog"는 argv[0] 자리. 인자는 가변 인자로 넘긴다. */
#define PARSE_OK(opts, ...)                                      \
    do {                                                         \
        const char* av_[] = {"prog", __VA_ARGS__};               \
        ARGPARSE_ARGS* args_ =                                   \
            argparse_new_args((int)ARRAY_LEN(av_) - 1, av_ + 1); \
        argparse_error err_ = argparse_parse(opts, args_);       \
        argparse_free_args(args_);                               \
        assert(err_ == ARGPARSE_OK);                             \
    } while (0)

#define PARSE_ERR(opts, expected, ...)                           \
    do {                                                         \
        const char* av_[] = {"prog", __VA_ARGS__};               \
        ARGPARSE_ARGS* args_ =                                   \
            argparse_new_args((int)ARRAY_LEN(av_) - 1, av_ + 1); \
        argparse_error err_ = argparse_parse(opts, args_);       \
        argparse_free_args(args_);                               \
        assert(err_ == (expected));                              \
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
#define POS_IS(opts, name, expected)                                           \
    do {                                                                       \
        const char* v_ = NULL;                                                 \
        assert(argparse_get_positional_value(opts, name, &v_) == ARGPARSE_OK); \
        assert(v_ && strcmp(v_, expected) == 0);                               \
    } while (0)

#define POS_ERR(opts, name, expected)                                         \
    do {                                                                      \
        const char* v_ = NULL;                                                \
        assert(argparse_get_positional_value(opts, name, &v_) == (expected)); \
    } while (0)
#define HAS_OPT(opts, name)                                          \
    do {                                                             \
        int v_ = 0;                                                  \
        assert(argparse_has_option(opts, name, &v_) == ARGPARSE_OK); \
        assert(v_);                                                  \
    } while (0)

#define HAS_OPT_ABSENT(opts, name)                                   \
    do {                                                             \
        int v_ = 0;                                                  \
        assert(argparse_has_option(opts, name, &v_) == ARGPARSE_OK); \
        assert(!v_);                                                 \
    } while (0)

int main(void) {
    ARGPARSE_OPTIONS* opts = argparse_new_options("test_argparse");
    ADD_OPT(opts, "flag", 'f', ARG_REQ);
    ADD_OPT(opts, "verbose", 'v', ARG_NONE);
    ADD_OPT(opts, "notused", 'n', ARG_NONE);

    ADD_POS(opts, "input");
    ADD_POS(opts, "output");

    /* parse */
    PARSE_OK(opts, "--flag=hello", "--verbose", "testinput", "testoutput");
    VALUE_IS(opts, "flag", "hello");
    HAS_OPT(opts, "verbose");
    HAS_OPT_ABSENT(opts, "notused");
    VALUE_ERR(opts, "missing", ARGPARSE_ERR_UNKNOWN_OPTION);

    POS_IS(opts, "input", "testinput");
    POS_IS(opts, "output", "testoutput");
    POS_ERR(opts, "missing", ARGPARSE_ERR_UNKNOWN_POSITIONAL);

    PARSE_ERR(opts, ARGPARSE_ERR_UNEXPECTED_POSITIONAL, "--flag=hello",
              "--verbose", "testinput", "testoutput", "unexpected");

    PARSE_OK(opts, "--", "--", "output");
    POS_IS(opts, "input", "--");
    POS_IS(opts, "output", "output");

    PARSE_OK(opts, "-vf=hello", "testinput", "testoutput");
    VALUE_IS(opts, "flag", "hello");
    HAS_OPT(opts, "verbose");

    PARSE_OK(opts, "-f", "hello", "input", "output");
    VALUE_IS(opts, "flag", "hello");

    PARSE_OK(opts, "-fhello", "input", "output");
    VALUE_IS(opts, "flag", "hello");

    PARSE_OK(opts, "-f=hello", "input", "output");
    VALUE_IS(opts, "flag", "hello");

    PARSE_OK(opts, "-f=", "input", "output");
    VALUE_IS(opts, "flag", "");

    PARSE_OK(opts, "-vf=", "input", "output");
    VALUE_IS(opts, "flag", "");
    HAS_OPT(opts, "verbose");

    PARSE_ERR(opts, ARGPARSE_ERR_MISSING_VALUE, "-f");
    PARSE_ERR(opts, ARGPARSE_ERR_UNKNOWN_OPTION, "-x");
    PARSE_ERR(opts, ARGPARSE_ERR_UNKNOWN_OPTION, "-vx");

    PARSE_OK(opts, "--", "-v", "output");
    POS_IS(opts, "input", "-v");

    /* -- 종료자 */
    PARSE_OK(opts, "--", "-5", "--flag");
    POS_IS(opts, "input", "-5");
    POS_IS(opts, "output", "--flag");

    /* 필수 위치 인자 부족 */
    PARSE_ERR(opts, ARGPARSE_ERR_MISSING_POSITIONAL, "in");

    /* 위치 인자는 '='를 쪼개지 않음 */
    PARSE_OK(opts, "a=b", "out");
    POS_IS(opts, "input", "a=b");

    /* '-' 단독 */
    PARSE_OK(opts, "-", "out");
    POS_IS(opts, "input", "-");

    PARSE_ERR(opts, ARGPARSE_ERR_UNKNOWN_OPTION, "--falg", "world");
    PARSE_ERR(opts, ARGPARSE_ERR_UNKNOWN_OPTION, "--falg=world");

    /* 공백 형태 */
    ARGPARSE_OPTIONS* opts1 = argparse_new_options("test_argparse");
    ADD_OPT(opts1, "flag", 'f', ARG_REQ);
    ADD_OPT(opts1, "verbose", 'v', ARG_NONE);
    ADD_OPT(opts1, "notused", 'n', ARG_NONE);

    PARSE_OK(opts1, "--flag", "world");
    VALUE_IS(opts1, "flag", "world");

    /* 반복 옵션 */
    PARSE_OK(opts1, "--flag=a", "--flag=b");
    VALUE_IS(opts1, "flag", "b");

    /* 값 누락 */
    PARSE_ERR(opts1, ARGPARSE_ERR_MISSING_VALUE, "--flag");
    VALUE_ERR(opts1, "flag", ARGPARSE_ERR_NOT_SET);
    HAS_OPT_ABSENT(opts1, "flag");

    /* 긴 인자 (이전 스택 오버플로 회귀) */
    PARSE_OK(opts1, "--flag=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    VALUE_IS(opts1, "flag", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    /* 빈 값 */
    PARSE_OK(opts1, "--flag=");
    VALUE_IS(opts1, "flag", "");

    argparse_free_args(NULL);
    argparse_free_options(NULL);
    argparse_free_options(opts);
    argparse_free_options(opts1);
    puts("ok");
    return 0;
}
