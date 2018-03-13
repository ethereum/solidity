#!/usr/bin/env bash

set -e

REPO_ROOT="$(dirname "$0")"/..
USE_DEBUGGER=0
DEBUGGER="gdb --args"
BOOST_OPTIONS=
SOLTEST_OPTIONS=

while [ $# -gt 0 ]
do
	case "$1" in
		--debugger)
			shift
			DEBUGGER="$1"
			USE_DEBUGGER=1
			;;
		--debug)
			USE_DEBUGGER=1
			;;
		--boost-options)
			shift
			BOOST_OPTIONS="${BOOST_OPTIONS} $1"
			;;
		-t)
			shift
			BOOST_OPTIONS="${BOOST_OPTIONS} -t $1"
			;;
		--show-progress | -p)
			BOOST_OPTIONS="${BOOST_OPTIONS} $1"
			;;
		*)
			SOLTEST_OPTIONS="${SOLTEST_OPTIONS} $1"
			;;
	esac
	shift
done
if [ "$USE_DEBUGGER" -ne "0" ]; then
	DEBUG_PREFIX=${DEBUGGER}
fi

exec ${DEBUG_PREFIX} ${REPO_ROOT}/build/test/soltest ${BOOST_OPTIONS} -- --testpath ${REPO_ROOT}/test ${SOLTEST_OPTIONS}
