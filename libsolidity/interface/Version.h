// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Versioning.
 */

#pragma once

#include <libsolutil/Common.h>
#include <string>

namespace solidity::frontend
{

extern char const* VersionNumber;
extern std::string const VersionString;
extern std::string const VersionStringStrict;
extern bytes const VersionCompactBytes;
extern bool const VersionIsRelease;

}
