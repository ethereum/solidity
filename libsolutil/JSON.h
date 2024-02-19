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
/** @file JSON.h
 * @date 2016
 *
 * JSON related helpers
 */

#pragma once

#define JSON_DIAGNOSTICS 1
#include <libsolutil/nlohmannjson.hpp>

#include <string>
#include <string_view>
#include <optional>

using Json = nlohmann::json;

namespace solidity::util
{

/// Removes members with null value recursively from (@a _json).
Json removeNullMembers(Json _json);

/// JSON printing format.
struct JsonFormat
{
	enum Format
	{
		Compact, // No unnecessary whitespace (including new lines and indentation)
		Pretty,  // Nicely indented, with new lines
	};

	static constexpr uint32_t defaultIndent = 2;

	bool operator==(JsonFormat const& _other) const noexcept { return (format == _other.format) && (indent == _other.indent); }
	bool operator!=(JsonFormat const& _other) const noexcept { return !(*this == _other); }

	Format format = Compact;
	uint32_t indent = defaultIndent;
};

/// Serialise the JSON object (@a _input) with indentation
std::string jsonPrettyPrint(Json const& _input);

/// Serialise the JSON object (@a _input) without indentation
std::string jsonCompactPrint(Json const& _input);

/// Serialise the JSON object (@a _input) using specified format (@a _format)
std::string jsonPrint(Json const& _input, JsonFormat const& _format);

/// Parse a JSON string (@a _input) with enabled strict-mode and writes resulting JSON object to (@a _json)
/// \param _input JSON input string
/// \param _json [out] resulting JSON object
/// \param _errs [out] Formatted error messages
/// \return \c true if the document was successfully parsed, \c false if an error occurred.
bool jsonParseStrict(std::string const& _input, Json& _json, std::string* _errs = nullptr);

/// Retrieves the value specified by @p _jsonPath by from a series of nested JSON dictionaries.
/// @param _jsonPath A dot-separated series of dictionary keys.
/// @param _node The node representing the start of the path.
/// @returns The value of the last key on the path. @a nullptr if any node on the path descends
/// into something that is not a dictionary or the key is not present.
std::optional<Json> jsonValueByPath(Json const& _node, std::string_view _jsonPath);

namespace detail
{

template<typename T>
struct helper;

template<typename T, bool(Json::*checkMember)() const, T(Json::*convertMember)() const>
struct helper_impl
{
	static bool isOfType(Json const& _input)
	{
		return (_input.*checkMember)();
	}
	static T get(Json const& _input)
	{
		return (_input.*convertMember)();
	}
	static T getOrDefault(Json const& _input, T _default = {})
	{
		T result = _default;
		if (isOfType(_input))
			result = (_input.*convertMember)();
		return result;
	}
};

template<> struct helper<float>: helper_impl<float, &Json::is_number_float, &Json::get<float>> {};
template<> struct helper<double>: helper_impl<double, &Json::is_number_float, &Json::get<double>> {};
template<> struct helper<std::string>: helper_impl<std::string, &Json::is_string, &Json::get<std::string>> {};
template<> struct helper<int>: helper_impl<int, &Json::is_number_integer, &Json::get<int>> {};
template<> struct helper<int64_t>: helper_impl<int64_t, &Json::is_number_integer, &Json::get<int64_t>> {};
template<> struct helper<unsigned>: helper_impl<unsigned, &Json::is_number_unsigned, &Json::get<uint>> {};
template<> struct helper<uint64_t>: helper_impl<uint64_t , &Json::is_number_unsigned, &Json::get<uint64_t>> {};

} // namespace detail

template<typename T>
bool isOfType(Json const& _input)
{
	return detail::helper<T>::isOfType(_input);
}

template<typename T>
bool isOfTypeIfExists(Json const& _input, std::string const& _name)
{
	if (_input.contains(_name))
		return isOfType<T>(_input[_name]);
	return true;
}

template<typename T>
T get(Json const& _input)
{
	return detail::helper<T>::get(_input);
}

template<typename T>
T getOrDefault(Json const& _input, T _default = {})
{
	return detail::helper<T>::getOrDefault(_input, _default);
}

} // namespace solidity::util
