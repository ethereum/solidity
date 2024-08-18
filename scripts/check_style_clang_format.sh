#!/usr/bin/env bash

set -eu

REPO_ROOT="$(realpath "$(dirname "$0")"/..)"

TEMPDIR=$(mktemp -d)

function cleanup() {
	rm -rf "${TEMPDIR}"
}

trap cleanup EXIT

pushd "${TEMPDIR}"
wget -q https://github.com/ekpyron/clang_format_always_block_indent/releases/download/v0.0.1/clang-format -O clang-format
sha256sum -c --quiet << EOF
24269dfc5d658ec5b73a49e326fe0bdb33336cb25bbe6737c02d1df83179928d  clang-format
EOF
chmod 755 clang-format
./clang-format -i "${REPO_ROOT}"/libsolidity/analysis/TypeChecker.*
popd

pushd "${REPO_ROOT}"
git --no-pager diff --exit-code
popd

echo "Style check passed."
