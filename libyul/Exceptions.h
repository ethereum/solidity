// SPDX-License-Identifier: GPL-3.0
/**
 * Exceptions in Yul.
 */

#pragma once

#include <libsolutil/Exceptions.h>
#include <libsolutil/Assertions.h>

namespace solidity::yul
{

struct YulException: virtual util::Exception {};
struct OptimizerException: virtual YulException {};
struct CodegenException: virtual YulException {};
struct YulAssertion: virtual YulException {};

/// Assertion that throws an YulAssertion containing the given description if it is not met.
#define yulAssert(CONDITION, DESCRIPTION) \
	assertThrow(CONDITION, ::solidity::yul::YulAssertion, DESCRIPTION)

}
