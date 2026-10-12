/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0
/**
 * Container of (unparsed) Yul functions identified by name which are meant to be generated
 * only once.
 */

#pragma once

#include <libsolutil/UnorderedContainers.h>

#include <cstdint>
#include <functional>
#include <source_location>
#include <string>
#include <string_view>
#include <typeindex>

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
	/// Asserts that repeated requests for @a _name come from the call site @a _site and the
	/// creator lambda that generated it.
	std::string createFunction(
		std::string const& _name,
		std::function<std::string()> const& _creator,
		std::source_location _site = std::source_location::current()
	);

	std::string createFunction(
		std::string const& _name,
		std::function<std::string(std::vector<std::string>&, std::vector<std::string>&)> const& _creator,
		std::source_location _site = std::source_location::current()
	);

	/// @returns concatenation of all generated functions in the order in which they were
	/// generated.
	/// Clears the internal list, i.e. calling it again will result in an
	/// empty return value.
	std::string requestedFunctions();

	/// @returns true IFF a function with the specified name has already been collected.
	bool contains(std::string const& _name) const { return m_requestedFunctions.count(_name) > 0; }

private:
	/// Identifies where a function was requested from
	struct CreationSite
	{
		std::string_view file;  // from std::source_location::file_name() which has static storage
		std::uint_least32_t line;
		std::uint_least32_t column;
		std::type_index creator;

		bool operator==(CreationSite const&) const = default;
	};

	/// Common implementation of all createFunction overloads.
	/// Creates the function on first request and otherwise asserts that the request comes
	/// from @a _location and a creator of type @a _creatorType.
	std::string createFunctionImpl(
		std::string const& _name,
		std::source_location const& _location,
		std::type_index _creatorType,
		std::function<std::string()> const& _creator
	);

	static void assertSameCreationSite(
		std::string const& _name,
		CreationSite const& _generatedAt,
		CreationSite const& _requestedAt
	);

	util::unordered_flat_map<std::string, CreationSite> m_requestedFunctions;
	std::string m_code;
};

}
