// SPDX-License-Identifier: GPL-3.0
/**
 * Container of (unparsed) Yul functions identified by name which are meant to be generated
 * only once.
 */

#pragma once

#include <functional>
#include <map>
#include <string>

namespace solidity::frontend
{

/**
 * Container of (unparsed) Yul functions identified by name which are meant to be generated
 * only once.
 */
class MultiUseYulFunctionCollector
{
public:
	/// Helper function that uses @a _creator to create a function and add it to
	/// @a m_requestedFunctions if it has not been created yet and returns @a _name in both
	/// cases.
	std::string createFunction(std::string const& _name, std::function<std::string()> const& _creator);

	/// @returns concatenation of all generated functions.
	/// Guarantees that the order of functions in the generated code is deterministic and
	/// platform-independent.
	/// Clears the internal list, i.e. calling it again will result in an
	/// empty return value.
	std::string requestedFunctions();

	/// @returns true IFF a function with the specified name has already been collected.
	bool contains(std::string const& _name) const { return m_requestedFunctions.count(_name) > 0; }

private:
	/// Map from function name to code for a multi-use function.
	std::map<std::string, std::string> m_requestedFunctions;
};

}
