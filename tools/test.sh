#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
logic_test_dir=$(mktemp -d)
trap 'rm -rf "$logic_test_dir"' EXIT HUP INT TERM
"${CC:-clang}" -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined \
  -Isrc -Itests src/logic.c src/app.c src/render.c src/save.c tests/test.c \
  -o "$logic_test_dir/test"
"$logic_test_dir/test"
