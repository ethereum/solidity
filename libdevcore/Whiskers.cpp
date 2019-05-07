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
/** @file Whiskers.cpp
 * @author Chris <chis@ethereum.org>
 * @date 2017
 *
 * Moustache-like templates.
 */

#include <libdevcore/Whiskers.h>

#include <libdevcore/Assertions.h>

#include <boost/regex.hpp>

using namespace std;
using namespace dev;

Whiskers::Whiskers(string _template):
	m_template(move(_template))
{
}

Whiskers& Whiskers::operator()(string _parameter, string _value)
{
	checkParameterUnknown(_parameter);
	m_parameters[move(_parameter)] = move(_value);
	return *this;
}

Whiskers& Whiskers::operator()(string _parameter, bool _value)
{
	checkParameterUnknown(_parameter);
	m_conditions[move(_parameter)] = _value;
	return *this;
}

Whiskers& Whiskers::operator()(
	string _listParameter,
	vector<map<string, string>> _values
)
{
	checkParameterUnknown(_listParameter);
	m_listParameters[move(_listParameter)] = move(_values);
	return *this;
}

string Whiskers::render() const
{
	return replace(m_template, m_parameters, m_conditions, m_listParameters);
}

void Whiskers::checkParameterUnknown(string const& _parameter)
{
	assertThrow(
		!m_parameters.count(_parameter),
		WhiskersError,
		_parameter + " already set as value parameter."
	);
	assertThrow(
		!m_conditions.count(_parameter),
		WhiskersError,
		_parameter + " already set as condition parameter."
	);
	assertThrow(
		!m_listParameters.count(_parameter),
		WhiskersError,
		_parameter + " already set as list parameter."
	);
}

string Whiskers::replace(
	string const& _template,
	StringMap const& _parameters,
	map<string, bool> const& _conditions,
	map<string, vector<StringMap>> const& _listParameters
)
{
	using namespace boost;
	static regex listOrTag("<([^#/?!>]+)>|<#([^>]+)>(.*?)</\\2>|<\\?([^>]+)>(.*?)(<!\\4>(.*?))?</\\4>");
	return regex_replace(_template, listOrTag, [&](match_results<string::const_iterator> _match) -> string
	{
		string tagName(_match[1]);
		string listName(_match[2]);
		string conditionName(_match[4]);
		if (!tagName.empty())
		{
			assertThrow(
				_parameters.count(tagName),
				WhiskersError,
				"Value for tag " + tagName + " not provided.\n" +
				"Template:\n" +
				_template
			);
			return _parameters.at(tagName);
		}
		else if (!listName.empty())
		{
			string templ(_match[3]);
			assertThrow(
				_listParameters.count(listName),
				WhiskersError, "List parameter " + listName + " not set."
			);
			string replacement;
			for (auto const& parameters: _listParameters.at(listName))
				replacement += replace(templ, joinMaps(_parameters, parameters), _conditions);
			return replacement;
		}
		else
		{
			assertThrow(!conditionName.empty(), WhiskersError, "");
			assertThrow(
				_conditions.count(conditionName),
				WhiskersError, "Condition parameter " + conditionName + " not set."
			);
			return replace(
				_conditions.at(conditionName) ? _match[5] : _match[7],
				_parameters,
				_conditions,
				_listParameters
			);
		}
	});
}

Whiskers::StringMap Whiskers::joinMaps(
	Whiskers::StringMap const& _a,
	Whiskers::StringMap const& _b
)
{
	Whiskers::StringMap ret = _a;
	for (auto const& x: _b)
		assertThrow(
			ret.insert(x).second,
			WhiskersError,
			"Parameter collision"
		);
	return ret;
}

