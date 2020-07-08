// SPDX-License-Identifier: GPL-3.0
/**
 * Class that can answer questions about values of variables and their relations.
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/YulString.h>
#include <map>

namespace solidity::yul
{

struct Dialect;
struct AssignedValue;

/**
 * Class that can answer questions about values of variables and their relations.
 *
 * The reference to the map of values provided at construction is assumed to be updating.
 */
class KnowledgeBase
{
public:
	KnowledgeBase(Dialect const& _dialect, std::map<YulString, AssignedValue> const& _variableValues):
		m_dialect(_dialect),
		m_variableValues(_variableValues)
	{}

	bool knownToBeDifferent(YulString _a, YulString _b);
	bool knownToBeDifferentByAtLeast32(YulString _a, YulString _b);
	bool knownToBeEqual(YulString _a, YulString _b) const { return _a == _b; }

private:
	Expression simplify(Expression _expression);

	Dialect const& m_dialect;
	std::map<YulString, AssignedValue> const& m_variableValues;
	size_t m_recursionCounter = 0;
};

}
