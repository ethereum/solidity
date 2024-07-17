#!/bin/bash -e

# This script is expected to be run inside the docker image trzeci/emscripten:sdk-tag-1.39.3-64bit.
# Its main purpose is to be called by ../rebuild.sh.

# Usage: $0 [tagFilter] [outputDirectory]

# The output directory must be outside the repository,
# since the script will prune the repository directory after
# each build.

TAG_FILTER="$1"
OUTPUTDIR="$2"
RETEST=0
shift
shift
while (( "$#" )); do
  if [[ "$1" == "--retest" ]]; then
    RETEST=1
  else
    echo "Unrecognized option: $1"
    exit 1
  fi
  shift
done

SOLIDITY_REPO_URL="https://github.com/ethereum/solidity"
SOLC_JS_REPO_URL="https://github.com/ethereum/solc-js"
SOLC_JS_BRANCH=wasmRebuildTests
RELEASE_URL="https://binaries.soliditylang.org/bin"
RELEASE_COMMIT_LIST_URL="$RELEASE_URL/list.txt"

SCRIPTDIR=$(dirname "$0")
SCRIPTDIR=$(realpath "${SCRIPTDIR}")
RED='\033[0;31m'
GREEN='\033[0;32m'
ORANGE='\033[0;33m'
CYAN='\033[0;36m'
RESET='\033[0m'

function generate_bytecode_report
{
  rm -rf /tmp/report.txt

  local EXIT_STATUS

  if semver -r "<0.4.12" "$3" > /dev/null; then
    set +e
    "${SCRIPTDIR}/genbytecode.sh" "$1" >/dev/null 2>&1
    EXIT_STATUS=$?
    set -e
  else
    set +e
    (
      set -e

      git reset --hard HEAD --quiet
      git clean -f -d -x --quiet

      for dir in build/solc build/libsolc emscripten_build/libsolc; do
        mkdir -p $dir
        rm -rf $dir/soljson.js
        ln -sf "$1" $dir/soljson.js
      done

       /tmp/storebytecode.sh >/dev/null 2>&1
    )
    EXIT_STATUS=$?
  fi

  if [ $EXIT_STATUS -eq 0 ] && [ -f /tmp/report.txt ] && grep -q -v -c -e "ERROR" -e "NO BYTECODE" /tmp/report.txt; then
    mv /tmp/report.txt "$2"
    echo -e "${GREEN}SUCCESS${RESET}"
  else
    echo -e "${RED}FAILURE${RESET}"
  fi
}
function clean_git_checkout
{
  git submodule deinit --all -q
  git reset --hard HEAD --quiet
  git clean -f -d -x --quiet
  git checkout "$1" --quiet
  git submodule init -q
  git submodule update -q
}
function process_tag
{
  local TAG=$1
  cd /src
  # Checkout the historic commit instead of the tag directly.
  local HISTORIC_COMMIT_HASH; HISTORIC_COMMIT_HASH="$(grep "${TAG}+" /tmp/release_commit_list.txt | cut -d '+' -f 2 | cut -d '.' -f 2)"
  if [ "$(git cat-file -t "${HISTORIC_COMMIT_HASH}" 2>/dev/null)" == "commit" ]; then
    clean_git_checkout "$HISTORIC_COMMIT_HASH"
  else
    clean_git_checkout "${TAG}"
  fi

  # compatibility symlink
  ln -s . solidity

  local VERSION
  if [ -f ./scripts/get_version.sh ]; then
    VERSION=$(./scripts/get_version.sh)
  else
    VERSION=$(echo "$TAG" | cut -d v -f 2)
  fi

  local COMMIT_HASH; COMMIT_HASH=$(git rev-parse --short=8 HEAD)
  local FULL_VERSION_SUFFIX="${TAG}+commit.${COMMIT_HASH}"
  local HISTORIC_VERSION_SUFFIX="${TAG}+commit.${HISTORIC_COMMIT_HASH}"

  if [ ! -f "${OUTPUTDIR}/bin/soljson-${FULL_VERSION_SUFFIX}.js" ]; then
    echo -ne "BUILDING ${CYAN}${TAG}${RESET}... "
    set +e
    (
      set -e
      "${SCRIPTDIR}/rebuild_current.sh" "${VERSION}" >"${OUTPUTDIR}/log/running/build-$TAG.txt" 2>&1
      "${SCRIPTDIR}/patch.sh" "$TAG" upload/soljson.js
      cp upload/soljson.js "${OUTPUTDIR}/bin/soljson-${FULL_VERSION_SUFFIX}.js"
      rm upload/soljson.js
    )
    local EXIT_STATUS=$?
    set -e
    rm -f "${OUTPUTDIR}/log/success/build-$TAG.txt"
    rm -f "${OUTPUTDIR}/log/fail/build-$TAG.txt"
    if [ $EXIT_STATUS -eq 0 ]; then
      mv "${OUTPUTDIR}/log/running/build-$TAG.txt" "${OUTPUTDIR}/log/success"
      echo -e "${GREEN}SUCCESS${RESET}"
    else
      mv "${OUTPUTDIR}/log/running/build-$TAG.txt" "${OUTPUTDIR}/log/fail"
      echo -e "${RED}FAIL${RESET}"
    fi
  else
    echo -e "${CYAN}${TAG}${RESET} ALREADY EXISTS."
    if [ $RETEST -eq 0 ]; then
      return 0
    fi
  fi

  if [ -f "${OUTPUTDIR}/bin/soljson-${FULL_VERSION_SUFFIX}.js" ]; then

    echo -ne "GENERATE BYTECODE REPORT FOR ${CYAN}${TAG}${RESET}... "
    generate_bytecode_report "${OUTPUTDIR}/bin/soljson-${FULL_VERSION_SUFFIX}.js" "${OUTPUTDIR}/log/reports/report-${TAG}.txt" "${TAG}"
    echo -ne "GENERATE BYTECODE REPORT FOR HISTORIC ${CYAN}${TAG}${RESET}... "
    rm -rf /tmp/soljson.js
    if wget -q "$RELEASE_URL/soljson-${HISTORIC_VERSION_SUFFIX}.js" -O /tmp/soljson.js; then
      generate_bytecode_report /tmp/soljson.js "${OUTPUTDIR}/log/reports/report-historic-${TAG}.txt" "${TAG}"
    else
      echo -e "${ORANGE}CANNOT FETCH RELEASE${RESET}"
    fi
    rm -rf /tmp/soljson.js

    if [ -f "${OUTPUTDIR}/log/reports/report-${TAG}.txt" ] && [ -f "${OUTPUTDIR}/log/reports/report-historic-${TAG}.txt" ]; then
      rm -rf "${OUTPUTDIR}/log/success/bytecode-${TAG}.txt"
      rm -rf "${OUTPUTDIR}/log/fail/bytecode-${TAG}.txt"
      if diff -q "${OUTPUTDIR}/log/reports/report-${TAG}.txt" "${OUTPUTDIR}/log/reports/report-historic-${TAG}.txt" >/dev/null 2>&1; then
        echo -e "${GREEN}BYTECODE MATCHES FOR ${CYAN}${TAG}${RESET}"
        grep -v -c -e "ERROR" -e "NO BYTECODE" "${OUTPUTDIR}/log/reports/report-${TAG}.txt" >"${OUTPUTDIR}/log/success/bytecode-${TAG}.txt"
      else
        echo -e "${RED}BYTECODE DOES NOT MATCH FOR ${CYAN}${TAG}${RESET}"
        echo "MISMATCH" >"${OUTPUTDIR}/log/fail/bytecode-${TAG}.txt"
      fi
    fi

    echo -ne "TESTING ${CYAN}${TAG}${RESET}... "
    cd /root/solc-js
    npm version --allow-same-version --no-git-tag-version "${VERSION}" >/dev/null
    sed -i -e "s/runTests(solc, .*)/runTests(solc, '${FULL_VERSION_SUFFIX}')/" test/compiler.js
    ln -sf "${OUTPUTDIR}/bin/soljson-${FULL_VERSION_SUFFIX}.js" soljson.js
    rm -f "${OUTPUTDIR}/log/success/test-$TAG.txt"
    rm -f "${OUTPUTDIR}/log/fail/test-$TAG.txt"
    if npm test >"${OUTPUTDIR}/log/running/test-$TAG.txt" 2>&1; then
      mv "${OUTPUTDIR}/log/running/test-$TAG.txt" "${OUTPUTDIR}/log/success"
      echo -e "${GREEN}SUCCESS${RESET}"
    else
      mv "${OUTPUTDIR}/log/running/test-$TAG.txt" "${OUTPUTDIR}/log/fail"
      echo -e "${RED}FAIL${RESET}"
    fi
  fi
}

cd /tmp

echo "Check out solidity repository..."
if [ -d /root/project ]; then
  echo "Solidity repo checkout already exists."
else
  git clone "${SOLIDITY_REPO_URL}" /root/project --quiet
fi

echo "Extract bytecode comparison scripts from v0.6.1..."
cd /root/project
git checkout v0.6.1 --quiet
cp scripts/bytecodecompare/storebytecode.sh /tmp
# shellcheck disable=SC2016
sed -i -e 's/rm -rf "\$TMPDIR"/cp "\$TMPDIR"\/report.txt \/tmp\/report.txt ; rm -rf "\$TMPDIR"/' /tmp/storebytecode.sh
sed -i -e 's/REPO_ROOT=.*/REPO_ROOT=\/src/' /tmp/storebytecode.sh
sed -i -e 's/git clone/git clone --branch '"${SOLC_JS_BRANCH}"'/' /tmp/storebytecode.sh
export SOLC_EMSCRIPTEN="On"

echo "Check out solc-js repository..."
if [ -d /root/solc-js ]; then
  echo "solc-js repo checkout already exists."
else
  git clone --branch "${SOLC_JS_BRANCH}" "${SOLC_JS_REPO_URL}" /root/solc-js --quiet
fi

echo "Create symbolic links for backwards compatibility with older emscripten docker images."
ln -sf /emsdk_portable/node/current/* /emsdk_portable/node/
ln -sf /emsdk_portable/emscripten/sdk/ /emsdk_portable/
ln -sf sdk /emsdk_portable/emscripten/bin
ln -sf /emsdk_portable/emscripten/bin/* /usr/local/bin
rm -rf /src
ln -sf /root/project /src

echo "Install dependencies and upgrade system packages."
apt-get -qq update >/dev/null 2>&1
apt-get -qq upgrade >/dev/null 2>&1
apt-get -qq install cmake >/dev/null 2>&1

echo "Create output directories."
mkdir -p "${OUTPUTDIR}"
mkdir -p "${OUTPUTDIR}"/log
mkdir -p "${OUTPUTDIR}"/log/success
mkdir -p "${OUTPUTDIR}"/log/fail
mkdir -p "${OUTPUTDIR}"/log/running
mkdir -p "${OUTPUTDIR}"/log/reports
mkdir -p "${OUTPUTDIR}"/bin

echo "Prepare solc-js."
cd /root/solc-js
npm install >/dev/null 2>&1

echo "Install semver helper."
npm install -g semver >/dev/null 2>&1

echo "Fetching release commit list."
wget -q "${RELEASE_COMMIT_LIST_URL}" -O /tmp/release_commit_list.txt

cd /src
TAGS=$(git tag --list "${TAG_FILTER}" | tac)
echo "Matching tags: ${TAGS}"
for TAG in ${TAGS}; do
  process_tag "${TAG}"
done
