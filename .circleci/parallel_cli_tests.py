#!/usr/bin/env python3

import subprocess
import sys

# Slowest CLI tests, whose execution takes time on the order of minutes (as of June 2023).
# When adding/removing items here, remember to update `parallelism` value in jobs that run this script.
# TODO: We should switch to time-based splitting but that requires JUnit XML report support in cmdlineTests.sh.
tests_to_run_in_parallel = [
    '~ast_import_export',                   # ~7 min
    '~evmasm_import_export',                # ~5 min
    '~ast_export_with_stop_after_parsing',  # ~4 min
    '~soljson_via_fuzzer',                  # ~3 min
    '~via_ir_equivalence',                  # ~1 min
    '~compilation_tests',                   # ~1 min
    '~documentation_examples',              # ~1 min
    '*',                                    # This item represents all the remaining tests
]

# Ask CircleCI to select a subset of tests for this parallel execution.
# If `parallelism` in CI config is set correctly, we should get just one but we can handle any split.
selected_tests = subprocess.check_output(
    ['circleci', 'tests', 'split'],
    input='\n'.join(tests_to_run_in_parallel),
    encoding='ascii',
).strip().split('\n')
selected_tests = set(selected_tests) - {''}
excluded_tests = set(tests_to_run_in_parallel) - selected_tests
assert selected_tests.issubset(set(tests_to_run_in_parallel))

if len(selected_tests) == 0:
    print("No tests to run.")
    sys.exit(0)

if '*' in selected_tests:
    filters = [arg for test_name in excluded_tests for arg in ['--exclude', test_name]]
else:
    filters = list(selected_tests)

subprocess.run(
    ['test/cmdlineTests.sh'] + filters,
    stdin=sys.stdin,
    stdout=sys.stdout,
    stderr=sys.stderr,
    check=True,
)
