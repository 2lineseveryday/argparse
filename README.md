# argparse

A small C11 command-line argument parser with no dependencies.

## Features

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

    if (argparse_add_option(opts, "verbose", ARG_NONE) != ARGPARSE_OK ||
        argparse_add_option(opts, "output", ARG_REQ) != ARGPARSE_OK ||
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
$ ./prog --verbose --output out.txt in.txt
verbose
input: in.txt
output: out.txt
```

## Syntax

| Input | Result |
|---|---|
| `prog in.txt` | positional `input` = `in.txt` |
| `prog --verbose in.txt` | flag set, positional filled |
| `prog --output=out.txt in.txt` | option value from `=` |
| `prog --output out.txt in.txt` | option value from the next argument |
| `prog -- --verbose` | `--verbose` becomes a positional |
| `prog -` | `-` becomes a positional |

## API

```c
ARGPARSE_OPTIONS* argparse_new_options(const char* program);
void argparse_free_options(ARGPARSE_OPTIONS* opts);

argparse_error argparse_add_option(ARGPARSE_OPTIONS* opts,
                                   const char* option_string,
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
- Values returned by the getters are owned by the options object and remain valid until `argparse_free_options`.

### Behavior

- `argparse_parse` resets option and positional state before parsing, so the same `opts` can be reused for another call.
- `argparse_get_option_value` returns `ARGPARSE_ERR_WRONG_TYPE` for `ARG_NONE` options; use `argparse_has_option` for flags.
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
