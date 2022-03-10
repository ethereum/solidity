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
 * Class that can answer questions about values of variables and their relations.
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/YulString.h>

#include <libsolutil/Common.h>
#include <libsolutil/Numeric.h>

#include <map>
#include <functional>

namespace solidity::yul
{

struct Dialect;
struct AssignedValue;

/**
 * Class that can answer questions about values of variables and their relations.
 */
class KnowledgeBase
{
public:
	KnowledgeBase(
		Dialect const& _dialect,
		std::function<AssignedValue const*(YulString)> _variableValues
	):
		m_dialect(_dialect),
		m_variableValues(std::move(_variableValues))
	{}

	bool knownToBeDifferent(YulString _a, YulString _b);
	std::optional<u256> differenceIfKnownConstant(YulString _a, YulString _b);
	bool knownToBeDifferentByAtLeast32(YulString _a, YulString _b);
	bool knownToBeEqual(YulString _a, YulString _b) const { return _a == _b; }
	bool knownToBeZero(YulString _a);
	std::optional<u256> valueIfKnownConstant(YulString _a);

private:
	Expression simplify(Expression _expression);
	Expression simplifyRecursively(Expression _expression);

	Dialect const& m_dialect;
	std::function<AssignedValue const*(YulString)> m_variableValues;
	size_t m_counter = 0;
};

}
