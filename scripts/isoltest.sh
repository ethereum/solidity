#!/usr/bin/env bash

set -e

REPO_ROOT="$(dirname "$0")"/..
exec ${REPO_ROOT}/build/test/tools/isoltest --testpath ${REPO_ROOT}/test
