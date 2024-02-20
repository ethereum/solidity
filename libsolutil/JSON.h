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

#include <libsolutil/Assertions.h>
#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <optional>
#include <limits>

namespace solidity
{

using Json = nlohmann::json;

} // namespace solidity

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

std::string removeNlohmannInternalErrorIdentifier(std::string const& _input);

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
		assertThrow(isOfType(_input), InvalidType,  "");
		return (_input.*convertMember)();
	}
	static T getOrDefault(Json const& _input, std::string const& _name, T _default = {})
	{
		T result = _default;
		if (_input.contains(_name) && isOfType(_input[_name]))
			result = (_input[_name].*convertMember)();
		return result;
	}
};

template<typename T, typename RT, bool(Json::*checkMember)() const, T(Json::*convertMember)() const>
struct helper_impl_with_range_check
{
	static bool isOfType(Json const& _input)
	{
		if ( (_input.*checkMember)())
		{
			T value = (_input.*convertMember)();
			return (value >= std::numeric_limits<RT>::min() && value <= std::numeric_limits<RT>::max());
		}
		return false;
	}

	static RT get(Json const& _input)
	{
		assertThrow(isOfType(_input), InvalidType,  "");
		return static_cast<RT>((_input.*convertMember)());
	}

	static RT getOrDefault(Json const& _input, std::string const& _name, RT _default = {})
	{
		RT value = _default;
		if (_input.contains(_name) && isOfType(_input[_name]))
			value = static_cast<RT>((_input[_name].*convertMember)());
		return value;
	}
};

template<> struct helper<Json::string_t>: helper_impl<Json::string_t, &Json::is_string, &Json::get<Json::string_t>> {};
template<> struct helper<Json::number_integer_t>: helper_impl<Json::number_integer_t, &Json::is_number_integer, &Json::get<Json::number_integer_t>> {};
template<> struct helper<Json::number_unsigned_t>: helper_impl<Json::number_unsigned_t, &Json::is_number_integer, &Json::get<Json::number_unsigned_t>> {};
template<> struct helper<Json::number_float_t>: helper_impl<Json::number_float_t, &Json::is_number, &Json::get<Json::number_float_t>> {};
template<> struct helper<Json::boolean_t>: helper_impl<Json::boolean_t, &Json::is_boolean, &Json::get<Json::boolean_t>> {};

template<> struct helper<int32_t>: helper_impl_with_range_check<Json::number_integer_t, int32_t, &Json::is_number_integer, &Json::get<Json::number_integer_t>> {};
template<> struct helper<int16_t>: helper_impl_with_range_check<Json::number_integer_t, int16_t, &Json::is_number_integer, &Json::get<Json::number_integer_t>> {};
template<> struct helper<int8_t>: helper_impl_with_range_check<Json::number_integer_t, int8_t, &Json::is_number_integer, &Json::get<Json::number_integer_t>> {};
template<> struct helper<uint32_t>: helper_impl_with_range_check<Json::number_unsigned_t, uint32_t, &Json::is_number_integer, &Json::get<Json::number_unsigned_t>> {};
template<> struct helper<uint16_t>: helper_impl_with_range_check<Json::number_unsigned_t, uint16_t, &Json::is_number_integer, &Json::get<Json::number_unsigned_t>> {};
template<> struct helper<uint8_t>: helper_impl_with_range_check<Json::number_integer_t, uint8_t, &Json::is_number_integer, &Json::get<Json::number_integer_t>> {};
template<> struct helper<float>: helper_impl_with_range_check<Json::number_float_t, float, &Json::is_number, &Json::get<Json::number_float_t>> {};

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
T getOrDefault(Json const& _input, std::string const& _name, T _default = {})
{
	return detail::helper<T>::getOrDefault(_input, _name, _default);
}

} // namespace solidity::util
