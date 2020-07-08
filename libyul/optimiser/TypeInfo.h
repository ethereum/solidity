// SPDX-License-Identifier: GPL-3.0
/**
 * Helper class that keeps track of the types while performing optimizations.
 */
#pragma once

#include <libyul/AsmDataForward.h>
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
