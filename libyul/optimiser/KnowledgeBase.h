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
/**
 * Class that can answer questions about values of variables and their relations.
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/YulString.h>
#include <map>

namespace yul
{

class Dialect;

/**
 * Class that can answer questions about values of variables and their relations.
 *
 * The reference to the map of values provided at construction is assumed to be updating.
 */
class KnowledgeBase
{
public:
	KnowledgeBase(Dialect const& _dialect, std::map<YulString, Expression const*> const& _variableValues):
		m_dialect(_dialect),
		m_variableValues(_variableValues)
	{}

	bool knownToBeDifferent(YulString _a, YulString _b) const;
	bool knownToBeEqual(YulString _a, YulString _b) const { return _a == _b; }

private:
	Dialect const& m_dialect;
	std::map<YulString, Expression const*> const& m_variableValues;
};

}
