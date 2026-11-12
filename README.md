# argparse

A small C11 command-line argument parser with no dependencies.

## Features

- Short options: `-v`, `-o value`, `-ovalue`, `-o=value`
- Combined short flags: `-abc` is `-a -b -c`
- One option can have both names: `-v` and `--verbose`
- Long options: `--flag`, `--name=value`, `--name value`
- Flags (`ARG_NONE`) and options that require a value (`ARG_REQ`)
- Positional arguments, filled in registration order
- `--` terminates option parsing; a lone `-` is treated as a positional
- Unknown options, missing values, and missing/extra positionals return error codes
- No global state: everything lives in an `ARGPARSE_OPTIONS` handle

## Build

```sh
make                    # build/libargparse.a and build/main
make test               # run the tests
make SANITIZE=1 test    # tests with ASan/UBSan
make clean
```

## Example

```c
#include <stdio.h>

#include "argparse.h"

int main(int argc, const char** argv) {
    ARGPARSE_OPTIONS* opts = argparse_new_options(argv[0]);
    if (!opts) {
        return 1;
    }

    if (argparse_add_option(opts, "verbose", 'v', ARG_NONE) != ARGPARSE_OK ||
        argparse_add_option(opts, "output", 'o', ARG_REQ) != ARGPARSE_OK ||
        argparse_add_positional(opts, "input") != ARGPARSE_OK) {
        argparse_free_options(opts);
        return 1;
    }

    /* The program name is not part of the arguments. */
    ARGPARSE_ARGS* args = argparse_new_args(argc - 1, argv + 1);
    if (!args) {
        argparse_free_options(opts);
        return 1;
    }

    argparse_error err = argparse_parse(opts, args);
    if (err != ARGPARSE_OK) {
        fprintf(stderr, "parse error: %d\n", err);
        argparse_free_args(args);
        argparse_free_options(opts);
        return 1;
    }

    int verbose = 0;
    argparse_has_option(opts, "verbose", &verbose);

    const char* output = NULL;
    argparse_get_option_value(opts, "output", &output);

    const char* input = NULL;
    argparse_get_positional_value(opts, "input", &input);

    if (verbose) {
        printf("verbose\n");
    }
    printf("input: %s\n", input);
    if (output) {
        printf("output: %s\n", output);
    }

    argparse_free_args(args);
    argparse_free_options(opts);
    return 0;
}
```

```sh
$ ./prog -v -o out.txt in.txt
verbose
input: in.txt
output: out.txt
```

## Syntax

| Input | Result |
|---|---|
| `prog in.txt` | positional `input` = `in.txt` |
| `prog -v in.txt` | short flag set |
| `prog --verbose in.txt` | long flag set |
| `prog -o out.txt in.txt` | short option value from the next argument |
| `prog -oout.txt in.txt` | short option value attached |
| `prog -o=out.txt in.txt` | short option value after `=` |
| `prog -o= in.txt` | empty short option value |
| `prog -abc in.txt` | `-a -b -c` combined |
| `prog --output=out.txt in.txt` | long option value from `=` |
| `prog --output out.txt in.txt` | long option value from the next argument |
| `prog -- --verbose` | `--verbose` becomes a positional |
| `prog -` | `-` becomes a positional |
| `prog -5` | unknown option; use `--` for values starting with `-` |

## API

```c
ARGPARSE_OPTIONS* argparse_new_options(const char* program);
void argparse_free_options(ARGPARSE_OPTIONS* opts);

argparse_error argparse_add_option(ARGPARSE_OPTIONS* opts,
                                   const char* option_string,
                                   char short_option,
                                   argparse_argtype type);
argparse_error argparse_add_positional(ARGPARSE_OPTIONS* opts,
                                       const char* name);

ARGPARSE_ARGS* argparse_new_args(int argc, const char** argv);
void argparse_free_args(ARGPARSE_ARGS* args);

argparse_error argparse_parse(ARGPARSE_OPTIONS* opts, ARGPARSE_ARGS* args);

argparse_error argparse_has_option(ARGPARSE_OPTIONS* opts, const char* name,
                                   int* out);
argparse_error argparse_get_option_value(ARGPARSE_OPTIONS* opts,
                                         const char* name, const char** out);
argparse_error argparse_get_positional_value(ARGPARSE_OPTIONS* opts,
                                             const char* name,
                                             const char** out);

void argparse_print_help(ARGPARSE_OPTIONS* opts);
```

### Ownership

- `argparse_new_options` copies `program`.
- `argparse_add_option` and `argparse_add_positional` copy the name.
- `short_option` is a single `char`; pass `'\0'` for a long-only option.
- Values returned by the getters are owned by the options object and remain valid until `argparse_free_options`.

### Behavior

- `argparse_parse` resets option and positional state before parsing, so the same `opts` can be reused for another call.
- `argparse_get_option_value` returns `ARGPARSE_ERR_WRONG_TYPE` for `ARG_NONE` options; use `argparse_has_option` for flags.
- A value-taking short option ends a cluster and takes the rest of the token as its value: `-ovalue` and `-o=value` both set `value`.
- The next argument is always taken as the value, even when it starts with `-` (POSIX behavior). `prog -o -5` sets the value to `-5`.
- A value-taking option with no following argument returns `ARGPARSE_ERR_MISSING_VALUE`.
- After `--`, every token is a positional, including a second `--`.
- Getters take the long name; short names are not used for lookup.
- `argparse_print_help` shows both names: `[-v|--verbose]`, `[-o|--output=value]`.
- All positionals are required and have a fixed count. Optional and variadic positionals are not supported yet.

### Errors

| Code | Meaning |
|---|---|
| `ARGPARSE_ERR_NULL` | a required pointer is `NULL` |
| `ARGPARSE_ERR_ALLOC` | allocation failed |
| `ARGPARSE_ERR_MISSING_VALUE` | an `ARG_REQ` option has no value |
| `ARGPARSE_ERR_UNKNOWN_OPTION` | option was not registered |
| `ARGPARSE_ERR_NOT_SET` | option or positional was not provided |
| `ARGPARSE_ERR_WRONG_TYPE` | value getter used on an `ARG_NONE` option |
| `ARGPARSE_ERR_UNEXPECTED_POSITIONAL` | more positionals than registered |
| `ARGPARSE_ERR_MISSING_POSITIONAL` | a required positional was not provided |
| `ARGPARSE_ERR_UNKNOWN_POSITIONAL` | positional name was not registered |
