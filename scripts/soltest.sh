#!/usr/bin/env bash
set -e

REPO_ROOT="$(dirname "$0")"/..
USE_DEBUGGER=0
DEBUGGER="gdb --args"
BOOST_OPTIONS=
SOLTEST_OPTIONS=
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-build}

usage() {
    echo 2>&1 "
Usage: $0 [options] [soltest-options]
Runs BOOST C++ unit test program, soltest.

Options:
  --debug                  soltest invocation prefaced with: \"$DEBUGGER\"
  --debugger *dbg-cmd*     soltest prefaced with your own debugger command.
  --run_test | -t  *name*  filters test unit(s) to include or exclude from test.
                           This  option can be given several times.
  --boost-options *x*      Set BOOST option *x*.
  --show-progress | -p     Set BOOST option --show-progress.

Important environment variables:

SOLIDITY_BUILD_DIR: Sets directory under the repository root of where test/soltest should be found.
           The default is \"${SOLIDITY_BUILD_DIR}\".
"
}

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
		--help)
		        usage
			exit 0
			;;
		--run_test | -t )
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

exec ${DEBUG_PREFIX} ${REPO_ROOT}/${SOLIDITY_BUILD_DIR}/test/soltest ${BOOST_OPTIONS} -- --testpath ${REPO_ROOT}/test ${SOLTEST_OPTIONS}
