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

#include <libevmasm/ThirdOptimizerTesting.h>
#include "ThirdOptimizerTesting.h"
#include <algorithm>
#include <tuple>

template<typename It>
It backExpressionBegin(It _begin, It _end)
{
	AssemblyItem const& topLevelItem = *(_end - 1);
	signed arguments = topLevelItem.arguments();
	auto currentEnd = _end - 1 /* top-level operation */;
	while (arguments > 0)
	{
		// TODO this code might break if anything returns more than 1 value, since we can't split things up
		arguments -= static_cast<AssemblyItem const&>(*(currentEnd - 1)).returnValues();
		currentEnd = backExpressionBegin(_begin, currentEnd);
		assertThrow(currentEnd >= _begin, OptimizerException, "currentEnd is before begin");
	}
	return currentEnd;
}

vector<vector<AssemblyItem>> parseExpressions(vector<AssemblyItem> const& _items, signed _limit = -1)
{
	auto currentEnd = _items.end();
	std::vector<vector<AssemblyItem>> expressions{};
	while (currentEnd != _items.begin() && (_limit == -1 || signed(expressions.size()) < _limit))
	{
		auto oldEnd = currentEnd;
		currentEnd = backExpressionBegin(_items.begin(), oldEnd);
		expressions.emplace_back(currentEnd, oldEnd);
		assertThrow(currentEnd >= _items.begin(), OptimizerException, "currentEnd is before begin");
	}

	return expressions;
}

vector<vector<AssemblyItem>> parseArguments(vector<AssemblyItem> const& _items)
{
	assertThrow(!_items.empty(), OptimizerException, "cannot get arguments from empty _items vector");

	unsigned neededArguments = _items.back().arguments();

	auto expressions = parseExpressions(vector<AssemblyItem>(_items.begin(), _items.end() - 1), neededArguments);
	assertThrow(expressions.size() >= neededArguments, OptimizerException, "more arguments needed than provided");

	expressions = vector<vector<AssemblyItem>>(expressions.rbegin(), expressions.rbegin() + neededArguments);

	std::reverse(expressions.begin(), expressions.end());

	return expressions;
}

vector<AssemblyItem> createItems(NewOptimizerPattern const& _pattern)
{
	switch (_pattern.kind())
	{
		case NewOptimizerPattern::Kind::Any:
			assertThrow(_pattern.isBound(), OptimizerException, "cannot create items from non-bound pattern");
			return _pattern.boundItems();
		case NewOptimizerPattern::Kind::Constant:
		{
			if (_pattern.hasConstant())
				return {_pattern.constant()};
			else
				return {_pattern.boundItems().back().data()};
		}
		case NewOptimizerPattern::Kind::Operation:
			if (_pattern.hasOperationValues())
			{
				auto operands = _pattern.operands();

				vector<AssemblyItem> instructions{};

				for (auto it = operands.rbegin(); it != operands.rend(); it++)
					for (auto const& item : createItems(*it))
						instructions.push_back(item);

				instructions.emplace_back(_pattern.instruction());

				return instructions;
			}
			else
				return _pattern.boundItems();
		case NewOptimizerPattern::Kind::Unknown:
			assertThrow(_pattern.isBound(), OptimizerException, "cannot create items from non-bound pattern");
			return _pattern.boundItems();
		default:
			assertThrow(false, OptimizerException, "unreachable");
	}
}


bool dev::solidity::NewOptimizerPattern::matches(std::vector<AssemblyItem> const& _items, bool _unbind)
{
	if (_unbind) unbind();

	if (isBound())
		return boundItems() == _items;

	switch (kind())
	{
		case Kind::Constant:
		{
			if (_items.size() != 1) return false;

			if (_items.back().type() != AssemblyItemType::Push) return false;

			if (hasConstant())
			{
				if (_items[0].data() == constant())
				{
					bind(_items);
					return true;
				}

				return false;
			}
			else
			{
				bind(_items);
				return true;
			}
		}
		case Kind::Operation:
		{
			if (!hasOperationValues())
			{
				if (_items.back().type() == AssemblyItemType::Operation)
				{
					bind(_items);
					return true;
				}

				return false;
			}

			auto const instruction = this->instruction();

			if (_items.back().type() != AssemblyItemType::Operation) return false;
			if (_items.back().instruction() != instruction) return false;

			auto const argumentCount = instructionInfo(instruction).args;
			assertThrow(unsigned(argumentCount) == operands().size(), OptimizerException, "");

			if (argumentCount == 0) return _items.size() == 1;

			auto parsedArguments = parseArguments(_items);

			auto& requiredArgumentPatterns = operands();

			assertThrow(
				requiredArgumentPatterns.size() == parsedArguments.size(),
				OptimizerException,
				"required argument count and parsed argument count did not match"
			);

			for (unsigned i = 0; i < parsedArguments.size(); i++)
			{
				auto& argumentItems = parsedArguments[i];

				if (!requiredArgumentPatterns[i].matches(argumentItems, false)) return false;
			}

			bind(_items);

			return true;
		}
		case Kind::Any:
			bind(_items);
			return true;
		case Kind::Unknown:
			return false;
		default:
			assertThrow(false, OptimizerException, "invalid pattern kind");
	}
}

void NewOptimizerPattern::bind(std::vector<AssemblyItem> const& _items)
{
	if (kind() == Kind::Constant)
	{
		assertThrow(_items.size() == 1 && _items.back().type() == AssemblyItemType::Push, OptimizerException, "invalid bind type");
		if (hasConstant())
			assertThrow(constant() == _items.back().data(), OptimizerException, "invalid bind constant");
	}

	if (kind() == Kind::Operation)
	{
		assertThrow(!_items.empty() && _items.back().type() == AssemblyItemType::Operation, OptimizerException, "invalid bind value");
		if (hasOperationValues())
			assertThrow(instruction() == _items.back().instruction(), OptimizerException, "invalid bind instruction");
	}

	m_ptr->m_isBound = true;
	m_ptr->m_boundItems = _items;
}

void dev::solidity::ThirdOptimizer::addDefaultRules()
{
	addRules(simplificationRuleList<NewOptimizerPattern>(
		{NewOptimizerPattern::Kind::Constant},
		{NewOptimizerPattern::Kind::Constant},
		{NewOptimizerPattern::Kind::Constant},
		{NewOptimizerPattern::Kind::Operation},
		{NewOptimizerPattern::Kind::Operation}
	));
}

void dev::solidity::ThirdOptimizer::addRules(vector<SimplificationRule<NewOptimizerPattern>> const& _rules)
{
	for (auto const& rule : _rules) addRule(rule);
}

void dev::solidity::ThirdOptimizer::addRule(SimplificationRule<NewOptimizerPattern> const& _rule)
{
	m_rules.push_back(_rule);
}

vector<AssemblyItem> dev::solidity::ThirdOptimizer::optimize(vector<AssemblyItem> const& _items)
{
	if (m_rules.empty()) addDefaultRules();

	if (_items.size() == 1) return _items;

	auto expressions = parseExpressions(_items);

	for (auto& expression : expressions)
	{
		bool canRun = std::all_of(
			expression.begin(),
			expression.end(),
			[](AssemblyItem item) -> bool {
				return
					item.returnValues() <= 1 &&
					(
						item.type() == AssemblyItemType::Operation ?
							!instructionInfo(item.instruction()).sideEffects :
							true
					);
			});

		if (canRun)
			for (auto& rule : m_rules)
				if (rule.pattern.matches(expression))
					expression = createItems(rule.action());

		std::vector<AssemblyItem> optimizedExpression{};
		auto arguments = parseArguments(expression);

		for (auto it = arguments.rbegin(); it != arguments.rend(); it++) optimizedExpression += optimize(*it);
		optimizedExpression.push_back(expression.back());

		expression = optimizedExpression;
	}

	vector<AssemblyItem> optimizedItems;
	for (auto it = expressions.rbegin(); it != expressions.rend(); it++) optimizedItems += *it;

	if (optimizedItems != _items) optimizedItems = optimize(optimizedItems);

	return optimizedItems;
}
