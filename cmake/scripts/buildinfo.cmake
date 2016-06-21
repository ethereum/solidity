# generates BuildInfo.h
# 
# this module expects
# ETH_SOURCE_DIR - main CMAKE_SOURCE_DIR
# ETH_DST_DIR - main CMAKE_BINARY_DIR
# ETH_BUILD_TYPE
# ETH_BUILD_PLATFORM
# ETH_BUILD_NUMBER
# ETH_VERSION_SUFFIX
#
# example usage:
# cmake -DETH_SOURCE_DIR=. -DETH_DST_DIR=build -DETH_BUILD_TYPE=Debug -DETH_BUILD_PLATFORM=mac -DETH_BUILD_NUMBER=45 -DETH_VERSION_SUFFIX="-rc1" -P scripts/buildinfo.cmake

if (NOT ETH_BUILD_TYPE)
	set(ETH_BUILD_TYPE "unknown")
endif()

if (NOT ETH_BUILD_PLATFORM)
	set(ETH_BUILD_PLATFORM "unknown")
endif()

execute_process(
	COMMAND git --git-dir=${ETH_SOURCE_DIR}/.git --work-tree=${ETH_SOURCE_DIR} rev-parse HEAD
	OUTPUT_VARIABLE ETH_COMMIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
) 

if (NOT ETH_COMMIT_HASH)
	set(ETH_COMMIT_HASH 0)
endif()

execute_process(
	COMMAND git --git-dir=${ETH_SOURCE_DIR}/.git --work-tree=${ETH_SOURCE_DIR} diff HEAD --shortstat
	OUTPUT_VARIABLE ETH_LOCAL_CHANGES OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
)

if (ETH_LOCAL_CHANGES)
	set(ETH_CLEAN_REPO 0)
else()
	set(ETH_CLEAN_REPO 1)
endif()

set(TMPFILE "${ETH_DST_DIR}/BuildInfo.h.tmp")
set(OUTFILE "${ETH_DST_DIR}/BuildInfo.h")

configure_file("${ETH_BUILDINFO_IN}" "${TMPFILE}")

include("${ETH_CMAKE_DIR}/EthUtils.cmake")
replace_if_different("${TMPFILE}" "${OUTFILE}" CREATE)

