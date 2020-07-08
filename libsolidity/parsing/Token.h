// SPDX-License-Identifier: GPL-3.0
/**
 * Solidity and Yul both share the same Token (and Scanner) API.
 *
 * This may (or may not) change in the future. But for the time being, we've put both
 * at a shared place, and *just* import them.
*/
#pragma once

#include <liblangutil/Token.h>

namespace solidity::frontend
{
namespace TokenTraits = langutil::TokenTraits;

using langutil::Token;
using langutil::ElementaryTypeNameToken;
}
