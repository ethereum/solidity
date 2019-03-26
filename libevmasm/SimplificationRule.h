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
 * Expression simplification pattern.
 */

#pragma once

#include <functional>

namespace dev
{
namespace solidity
{

/**
 * Rule that contains a pattern, an action that can be applied
 * after the pattern has matched and a bool that indicates
 * whether the action would remove something from the expression
 * than is not a constant literal.
 */
template <class Pattern>
struct SimplificationRule
{
	SimplificationRule(
		Pattern _pattern,
		std::function<Pattern()> _action,
		bool _removesNonConstants,
		std::function<bool()> _feasible = {}
	):
		pattern(std::move(_pattern)),
		action(std::move(_action)),
		removesNonConstants(_removesNonConstants),
		feasible(std::move(_feasible))
	{}

	Pattern pattern;
	std::function<Pattern()> action;
	bool removesNonConstants;
	std::function<bool()> feasible;
};

}
}
