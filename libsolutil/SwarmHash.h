// SPDX-License-Identifier: GPL-3.0
/** @file SwarmHash.h
 */

#pragma once

#include <libsolutil/FixedHash.h>

#include <string>

namespace solidity::util
{

/// Compute the "swarm hash" of @a _input (OLD 0x1000-section version)
h256 bzzr0Hash(std::string const& _input);

/// Compute the "bzz hash" of @a _input (the NEW binary / BMT version)
h256 bzzr1Hash(bytes const& _input);

inline h256 bzzr1Hash(std::string const& _input)
{
	return bzzr1Hash(asBytes(_input));
}

}
