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
 * Helper class that keeps track of the types while performing optimizations.
 */
#pragma once

#include <libyul/ASTForward.h>
#include <libyul/YulString.h>

#include <vector>
#include <map>

namespace solidity::yul
{
struct Dialect;

/**
 * Helper class that keeps track of the types while performing optimizations.
 *
 * Only works on disambiguated sources!
 */
class TypeInfo
{
public:
	TypeInfo(Dialect const& _dialect, Block const& _ast);

	void setVariableType(YulString _name, YulString _type) { m_variableTypes[_name] = _type; }

	/// @returns the type of an expression that is assumed to return exactly one value.
	YulString typeOf(Expression const& _expression) const;

	/// \returns the type of variable
	YulString typeOfVariable(YulString _name) const;

private:
	class TypeCollector;

	struct FunctionType
	{
		std::vector<YulString> parameters;
		std::vector<YulString> returns;
	};

	Dialect const& m_dialect;
	std::map<YulString, YulString> m_variableTypes;
	std::map<YulString, FunctionType> m_functionTypes;
};

}
