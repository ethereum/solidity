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
/** @file Whiskers.h
 * @author Chris <chis@ethereum.org>
 * @date 2017
 *
 * Moustache-like templates.
 */

#pragma once

#include <libdevcore/Exceptions.h>

#include <string>
#include <map>
#include <vector>

namespace dev
{

DEV_SIMPLE_EXCEPTION(WhiskersError);

///
/// Moustache-like templates.
///
/// Usage:
///     std::vector<std::map<std::string, std::string>> listValues(2);
///     listValues[0]["k"] = "key1";
///     listValues[0]["v"] = "value1";
///     listValues[1]["k"] = "key2";
///     listValues[1]["v"] = "value2";
///     auto s = Whiskers("<p1>\n<#list><k> -> <v>\n</list>")
///         ("p1", "HEAD")
///         ("list", listValues)
///         .render();
///
/// results in s == "HEAD\nkey1 -> value1\nkey2 -> value2\n"
///
/// Note that lists cannot themselves contain lists - this would be a future feature.
class Whiskers
{
public:
	using StringMap = std::map<std::string, std::string>;
	using StringListMap = std::map<std::string, std::vector<StringMap>>;

	explicit Whiskers(std::string _template);

	/// Sets a single parameter, <paramName>.
	Whiskers& operator()(std::string _parameter, std::string _value);
	/// Sets a list parameter, <#listName> </listName>.
	Whiskers& operator()(
		std::string _listParameter,
		std::vector<StringMap> _values
	);

	std::string render() const;

private:
	void checkParameterUnknown(std::string const& _parameter);

	static std::string replace(
		std::string const& _template,
		StringMap const& _parameters,
		StringListMap const& _listParameters = StringListMap()
	);

	/// Joins the two maps throwing an exception if two keys are equal.
	static StringMap joinMaps(StringMap const& _a, StringMap const& _b);

	std::string m_template;
	StringMap m_parameters;
	StringListMap m_listParameters;
};

}
