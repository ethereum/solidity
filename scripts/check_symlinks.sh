#!/usr/bin/env bash

set -e

REPO_ROOT="$(dirname "$0")"/..
REPO_ROOT=$(realpath "${REPO_ROOT}")

BROKEN_LINKS=$(find -L "${REPO_ROOT}" -type l -ls)
if [ -z "${BROKEN_LINKS}" ]
then
  exit 0
else
  echo "broken symbolic link(s) found:"
  echo "${BROKEN_LINKS}"

  exit 1
fi
