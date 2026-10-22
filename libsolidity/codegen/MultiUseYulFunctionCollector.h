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

#include <libsolidity/interface/DebugSettings.h>

#include <liblangutil/EVMVersion.h>
#include <libsolutil/UnorderedContainers.h>
#include <libsolutil/Whiskers.h>

#include <boost/container_hash/hash.hpp>

#include <concepts>
#include <cstdint>
#include <functional>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>
#include <typeindex>
#include <type_traits>
#include <utility>
#include <vector>

namespace solidity::frontend
{

class ASTNode;
class MultiUseYulFunctionCollector;
class Type;
class YulUtilFunctions;

/// Cache of generated Yul functions by the kind and the arguments they are requested with, so the name and the
/// definition of a function are only built on its first request.
/// E.g., a second `collector.request<shiftLeftKind>(8)` returns `shift_left_8` right away.
namespace yulcache
{

class FunctionEnv;

/// Key of an argument of a request s.t. arguments of the same type with equal keys yield the same function.
/// The types of the arguments are determined by the kind, so keys of arguments of different types are never compared.
/// E.g., "8" for the number of bits 8 and "t_uint256" for the type uint256.
using RequestArgumentKey = std::string;

RequestArgumentKey requestArgumentKey(Type const& _type);  ///< Represented by its rich identifier

RequestArgumentKey requestArgumentKey(ASTNode const& _node);  ///< Represented by its ID

inline RequestArgumentKey requestArgumentKey(std::string const& _value) { return _value; }

template<std::integral T>
RequestArgumentKey requestArgumentKey(T _value) { return std::to_string(_value); }

template<typename T> requires std::is_enum_v<T>
RequestArgumentKey requestArgumentKey(T _value) { return std::to_string(std::to_underlying(_value)); }

/// The key of a present value is prefixed, so it differs from the empty key of an absent value, also for nested optionals.
/// E.g., "" for an absent value, "+0" for 0 and "++0" for 0 in an optional of an optional.
template<typename T>
RequestArgumentKey requestArgumentKey(std::optional<T> const& _value)
{
	if (!_value.has_value())
		return {};
	return "+" + requestArgumentKey(*_value);
}

/// A type of arguments of requests.
/// Two arguments of the same type are the same if their keys returned by requestArgumentKey are equal.
/// Other types can be supported by overloading `requestArgumentKey`.
/// E.g.,
///   - `std::size_t` for the number of bits of `shift_left_<bits>`, or
///   - `Type` and `VariableDeclaration::Location` for `storage_set_to_zero_<type>`.
template<typename T>
concept RequestArgument = requires(T const& _value)
{
	{ requestArgumentKey(_value) } -> std::same_as<RequestArgumentKey>;
};

/// A family of generated functions, e.g. the shift_left_<bits> helpers.
/// A function is identified by the address of its kind and its arguments, which therefore have to comprise every
/// input its definition depends on.
/// @a name and @a define are function pointers, so they cannot capture anything, and are invoked with the arguments.
/// @a define can additionally only reach the per-collector state provided by FunctionEnv.
/// Kinds are passed to `MultiUseYulFunctionCollector::request` as template arguments, so they have static
/// storage duration and their addresses identify them.
/// @a define returns the template of the function, whose `<functionName>` is set by the collector. Whiskers asserts
/// that the template contains it and that the kind does not set it itself.
/// E.g., `shiftLeftKind`, whose `name(8)` is `shift_left_8` and whose `define(_env, 8)` is the template
/// `function <functionName>(value) -> newValue { newValue := shl(8, value) }`.
template<RequestArgument... Args>
struct FunctionKind
{
	/// @returns the name of the function for the given arguments
	std::string (*name)(Args const&...);
	util::Whiskers (*define)(FunctionEnv const&, Args const&...);
	/// Where the kind is defined. Only used in error messages.
	std::source_location location = std::source_location::current();
};

/// Environment of the definition of a utility helper function.
/// E.g., the EVM version, by which `shift_left_<bits>` chooses between `shl` and `mul`.
class FunctionEnv
{
public:
	FunctionEnv(
		langutil::EVMVersion const _evmVersion,
		RevertStrings const _revertStrings,
		MultiUseYulFunctionCollector& _collector
	):
		m_evmVersion(_evmVersion),
		m_revertStrings(_revertStrings),
		m_collector(_collector)
	{}

	langutil::EVMVersion evmVersion() const { return m_evmVersion; }
	/// @returns a generator of utility functions that adds them to the same collector, to request
	/// helper functions that are not generated through a kind yet.
	YulUtilFunctions utils() const;
	template<auto const& Kind>
	std::string request(auto const&... _args) const;

private:
	langutil::EVMVersion m_evmVersion;
	RevertStrings m_revertStrings;
	MultiUseYulFunctionCollector& m_collector;
};

}

/**
 * Container of (unparsed) Yul functions identified by name which are meant to be generated
 * only once.
 */
class MultiUseYulFunctionCollector
{
public:
	MultiUseYulFunctionCollector(langutil::EVMVersion _evmVersion, RevertStrings _revertStrings):
		m_evmVersion(_evmVersion),
		m_revertStrings(_revertStrings)
	{}

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

	/// @returns the name of the function of kind @a Kind for the arguments @a _args.
	template<auto const& Kind>
	std::string request(auto const&... _args)
	{
		return requestImpl(Kind, _args...);
	}

	/// @returns concatenation of all generated functions in the order in which they were
	/// generated.
	/// Clears the internal list, i.e. calling it again will result in an
	/// empty return value.
	std::string requestedFunctions();

	/// @returns true IFF a function with the specified name has already been collected.
	bool contains(std::string const& _name) const { return m_requestedFunctions.count(_name) > 0; }

private:
	/// Identifies what a function was requested by:
	/// - the call site and the creator type of a request through createFunction, or
	/// - the definition site and the type of a kind.
	struct CreationSite
	{
		CreationSite(std::source_location const& _location, std::type_index const _creator):
			file(_location.file_name()),
			line(_location.line()),
			column(_location.column()),
			creator(_creator)
		{}

		std::string_view file;  // from std::source_location::file_name() which has static storage
		std::uint_least32_t line;
		std::uint_least32_t column;
		std::type_index creator;

		bool operator==(CreationSite const&) const = default;
	};

	/// Identifies a function requested through a kind: the address of the kind, which determines
	/// the types of the arguments, and the keys of the arguments.
	using RequestKey = std::pair<void const*, std::vector<yulcache::RequestArgumentKey>>;

	template<typename... Args>
	std::string requestImpl(
		yulcache::FunctionKind<Args...> const& _kind,
		std::type_identity_t<Args> const&... _args
	)
	{
		// Unqualified, so that overloads for other types are found by ADL
		using yulcache::requestArgumentKey;
		RequestKey key{&_kind, {requestArgumentKey(_args)...}};
		if (
			auto const it = m_keyToName.find(key);
			it != m_keyToName.end()
		)
			return it->second;

		std::string name = _kind.name(_args...);
		// The name is assigned before the definition, which may request the function recursively.
		claimName(name, CreationSite{_kind.location, typeid(_kind)});
		m_keyToName.emplace(std::move(key), name);
		m_code += _kind.define(
			yulcache::FunctionEnv{m_evmVersion, m_revertStrings, *this},
			_args...
		)("functionName", name).render();
		return name;
	}

	/// Records that @a _name is requested by the kind @a _site and asserts that the name was not
	/// requested before.
	void claimName(std::string const& _name, CreationSite const& _site);

	/// Common implementation of all createFunction overloads.
	/// Creates the function on first request and otherwise asserts that the request comes
	/// from @a _location and a creator of type @a _creatorType.
	std::string createFunctionImpl(
		std::string const& _name,
		std::source_location const& _location,
		std::type_index _creatorType,
		std::function<std::string()> const& _creator
	);

	/// @returns the message of the assertion that @a _name was requested by @a _requestedBy but
	/// generated by @a _generatedBy.
	static std::string nameClashMessage(
		std::string const& _name,
		CreationSite const& _generatedBy,
		CreationSite const& _requestedBy
	);

	langutil::EVMVersion m_evmVersion;
	RevertStrings m_revertStrings;
	util::unordered_flat_map<std::string, CreationSite> m_requestedFunctions;
	/// Names of the functions requested through kinds.
	util::unordered_flat_map<RequestKey, std::string, boost::hash<RequestKey>> m_keyToName;
	std::string m_code;
};

template<auto const& Kind>
std::string yulcache::FunctionEnv::request(auto const&... _args) const
{
	return m_collector.request<Kind>(_args...);
}

}
