#!/bin/sh

# This script is expected to produce working builds for all compiler versions >= 0.3.6 and
# succeeding solc-js test runs for all compiler versions >= 0.5.0.

if [ $# -lt 2 ]; then
  echo "Usage: $0 [tagFilter] [outputDirectory] [options...]"
  echo
  echo "  [tagFilter] will be passed to "git tag --list" to filter the tags to be built."
  echo "  [outputDirectory] will contain log files and the resulting soljson.js builds."
  echo "  --retest will re-run tests and bytecode comparisons, even if soljson.js is already built."
  exit 1
fi

TAGS="$1"
OUTPUTDIR="$2"
shift
shift
SCRIPTDIR=$(dirname "$0")
SCRIPTDIR=$(realpath "${SCRIPTDIR}")

if [ ! -d "${OUTPUTDIR}" ]; then
  echo "Output directory ${OUTPUTDIR} does not exist!."
  exit 1
fi
OUTPUTDIR=$(realpath "${OUTPUTDIR}")

# solbuildpackpusher/solidity-buildpack-deps:emscripten-9
DOCKER_IMAGE="solbuildpackpusher/solidity-buildpack-deps@sha256:d51534dfdd05ece86f69ed7beafd68c15b88606da00a4b7fe2873ccfbd0dce24"
docker run \
    --rm \
    --interactive \
    --tty \
    --volume /projects/solidity:/tmp/solidity:ro \
    --volume "${OUTPUTDIR}":/tmp/output \
    --volume "${SCRIPTDIR}":/tmp/scripts:ro \
    "${DOCKER_IMAGE}" \
    /tmp/scripts/docker-scripts/rebuild_tags.sh "${TAGS}" /tmp/output "$@"
