#!/bin/bash

if [[ ! -f "compile_commands.json" ]]; then
    echo "No 'compile_commands.json' file found. Aborting."
    exit 1
fi

python3 ../tools/run-clang-tidy.py -clang-tidy-binary=clang-tidy-12 -clang-apply-replacements-binary=clang-apply-replacements-12 -quiet -header-filter='(utils/lib/logger/include/.*)|(utils/lib/registry/.*)' -export-fixes=errors.yml
