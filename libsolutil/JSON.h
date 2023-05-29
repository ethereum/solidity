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

#include <json/json.h>

#include <string>
#include <string_view>
#include <optional>

namespace solidity::util
{

/// Removes members with null value recursively from (@a _json).
Json::Value removeNullMembers(Json::Value _json);

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
std::string jsonPrettyPrint(Json::Value const& _input);

/// Serialise the JSON object (@a _input) without indentation
std::string jsonCompactPrint(Json::Value const& _input);

/// Serialise the JSON object (@a _input) using specified format (@a _format)
std::string jsonPrint(Json::Value const& _input, JsonFormat const& _format);

/// Parse a JSON string (@a _input) with enabled strict-mode and writes resulting JSON object to (@a _json)
/// \param _input JSON input string
/// \param _json [out] resulting JSON object
/// \param _errs [out] Formatted error messages
/// \return \c true if the document was successfully parsed, \c false if an error occurred.
bool jsonParseStrict(std::string const& _input, Json::Value& _json, std::string* _errs = nullptr);

/// Retrieves the value specified by @p _jsonPath by from a series of nested JSON dictionaries.
/// @param _jsonPath A dot-separated series of dictionary keys.
/// @param _node The node representing the start of the path.
/// @returns The value of the last key on the path. @a nullptr if any node on the path descends
/// into something that is not a dictionary or the key is not present.
std::optional<Json::Value> jsonValueByPath(Json::Value const& _node, std::string_view _jsonPath);

namespace detail
{

template<typename T>
struct helper;

template<typename T, bool(Json::Value::*checkMember)() const, T(Json::Value::*convertMember)() const>
struct helper_impl
{
	static bool isOfType(Json::Value const& _input)
	{
		return (_input.*checkMember)();
	}
	static T get(Json::Value const& _input)
	{
		return (_input.*convertMember)();
	}
	static T getOrDefault(Json::Value const& _input, T _default = {})
	{
		T result = _default;
		if (isOfType(_input))
			result = (_input.*convertMember)();
		return result;
	}
};

template<> struct helper<float>: helper_impl<float, &Json::Value::isDouble, &Json::Value::asFloat> {};
template<> struct helper<double>: helper_impl<double, &Json::Value::isDouble, &Json::Value::asDouble> {};
template<> struct helper<std::string>: helper_impl<std::string, &Json::Value::isString, &Json::Value::asString> {};
template<> struct helper<Json::Int>: helper_impl<Json::Int, &Json::Value::isInt, &Json::Value::asInt> {};
template<> struct helper<Json::Int64>: helper_impl<Json::Int64, &Json::Value::isInt64, &Json::Value::asInt64> {};
template<> struct helper<Json::UInt>: helper_impl<Json::UInt, &Json::Value::isUInt, &Json::Value::asUInt> {};
template<> struct helper<Json::UInt64>: helper_impl<Json::UInt64, &Json::Value::isUInt64, &Json::Value::asUInt64> {};

} // namespace detail

template<typename T>
bool isOfType(Json::Value const& _input)
{
	return detail::helper<T>::isOfType(_input);
}

template<typename T>
bool isOfTypeIfExists(Json::Value const& _input, std::string const& _name)
{
	if (_input.isMember(_name))
		return isOfType<T>(_input[_name]);
	return true;
}

template<typename T>
T get(Json::Value const& _input)
{
	return detail::helper<T>::get(_input);
}

template<typename T>
T getOrDefault(Json::Value const& _input, T _default = {})
{
	return detail::helper<T>::getOrDefault(_input, _default);
}

} // namespace solidity::util
