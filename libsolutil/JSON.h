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

namespace detail
{

template<typename T>
struct helper;

#define DEFINE_HELPER(TYPE, CHECK_TYPE, CONVERT_TYPE)                                                     \
	template<>                                                                                            \
	struct helper<TYPE>                                                                                   \
	{                                                                                                     \
		static bool ofType(Json::Value const& _input, std::string const& _name)                           \
		{                                                                                                 \
			return _input[_name].CHECK_TYPE();                                                            \
		}                                                                                                 \
		static TYPE get(Json::Value const& _input, std::string const& _name)                              \
		{                                                                                                 \
			return _input[_name].CONVERT_TYPE();                                                          \
		}                                                                                                 \
		static TYPE getOrDefault(Json::Value const& _input, std::string const& _name, TYPE _default = {}) \
		{                                                                                                 \
			TYPE result = _default;                                                                       \
			if (helper::ofType(_input, _name))                                                            \
				result = _input[_name].CONVERT_TYPE();                                                    \
			return result;                                                                                \
		}                                                                                                 \
	};

DEFINE_HELPER(float, isDouble, asFloat)
DEFINE_HELPER(double, isDouble, asDouble)
DEFINE_HELPER(std::string, isString, asString)
DEFINE_HELPER(Json::Int, isInt, asInt)
DEFINE_HELPER(Json::Int64, isInt64, asInt64)
DEFINE_HELPER(Json::UInt, isUInt, asUInt)
DEFINE_HELPER(Json::UInt64, isUInt64, asUInt64)

} // namespace detail

template<typename T>
bool ofType(Json::Value const& _input, std::string const& _name)
{
	return detail::helper<T>::ofType(_input, _name);
}

template<typename T>
bool ofTypeIfExists(Json::Value const& _input, std::string const& _name)
{
	if (_input.isMember(_name))
		return ofType<T>(_input, _name);
	return true;
}

template<typename T>
T get(Json::Value const& _input, std::string const& _name)
{
	return detail::helper<T>::get(_input, _name);
}

template<typename T>
T getOrDefault(Json::Value const& _input, std::string const& _name, T _default = {})
{
	return detail::helper<T>::getOrDefault(_input, _name, _default);
}

} // namespace solidity::util
