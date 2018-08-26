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

template<typename It, typename = typename std::enable_if<is_assembly_item<decltype(*std::declval<It>())>::value>::type>
It noReturnValueBegin(iterator_pair<It> _bounds);

template<typename It, typename = typename std::enable_if<is_assembly_item<decltype(*std::declval<It>())>::value>::type>
It backExpressionBegin(iterator_pair<It> _bounds, bool _allowNoReturn = false)
{
	auto begin = _bounds.begin;
	auto end = _bounds.end;

	AssemblyItem const& topLevelItem = *(end - 1);

	signed arguments = topLevelItem.arguments();
	auto currentEnd = end - 1 /* top-level operation */;
	while (arguments > 0)
	{
		// TODO this code might break if anything returns more than 1 value, since we can't split things up
		currentEnd = noReturnValueBegin(make_iterator_pair(begin, currentEnd));
		arguments -= static_cast<AssemblyItem const&>(*(currentEnd - 1)).returnValues();
		currentEnd = backExpressionBegin(make_iterator_pair(begin, currentEnd), false);
		assertThrow(currentEnd >= begin, OptimizerException, "currentEnd is before begin");
	}

	if (!_allowNoReturn && topLevelItem.returnValues() == 0) return backExpressionBegin(make_iterator_pair(begin, currentEnd), false);

	return currentEnd;
}

template<typename It, typename = typename std::enable_if<is_assembly_item<decltype(*std::declval<It>())>::value>::type>
It noReturnValueBegin(iterator_pair<It> _bounds)
{
	auto begin = _bounds.begin;
	auto end = _bounds.end;

	assertThrow(begin != end, OptimizerException, "expression is empty");
	AssemblyItem const& topLevelItem = *(end - 1);
	if (topLevelItem.returnValues() != 0) return end;
	return backExpressionBegin(_bounds, true);
}

template<typename It, typename = typename std::enable_if<is_assembly_item<decltype(*std::declval<It>())>::value>::type>
vector<It> parseExpressions(iterator_pair<It> _bounds, signed _limit = -1, bool _allowNoReturn = false)
{
	auto begin = _bounds.begin;
	auto end = _bounds.end;

	auto currentEnd = end;
	std::vector<It> expressions{};
	while (currentEnd != begin && (_limit == -1 || signed(expressions.size()) < _limit))
	{
		auto oldEnd = currentEnd;
		currentEnd = backExpressionBegin(make_iterator_pair(begin, oldEnd), _allowNoReturn);
		expressions.push_back(currentEnd);
		assertThrow(currentEnd >= begin, OptimizerException, "currentEnd is before begin");
	}

	return expressions;
}

template<typename It, typename = typename std::enable_if<is_assembly_item<decltype(*std::declval<It>())>::value>::type>
vector<It> parseArguments(iterator_pair<It> _bounds)
{
	auto begin = _bounds.begin;
	auto end = _bounds.end;

	auto size = end - begin;
	assertThrow(size > 0, OptimizerException, "cannot get arguments from empty _items vector");

	unsigned neededArguments = (end - 1)->arguments();

	auto expressions = parseExpressions(make_iterator_pair(begin, end - 1), neededArguments, false);
	assertThrow(expressions.size() >= neededArguments, OptimizerException, "more arguments needed than provided");

	expressions = vector<It>(expressions.begin(), expressions.begin() + neededArguments);

	return expressions;
}

vector<AssemblyItem> createItems(NewOptimizerPattern const& _pattern)
{
	if (_pattern.isBound())
	{
		auto bound = _pattern.boundItems();
		return vector<AssemblyItem>(bound.begin, bound.end);
	}

	switch (_pattern.kind())
	{
		case NewOptimizerPattern::Kind::Any:
			assertThrow(false, OptimizerException, "cannot create items from non-bound pattern");
		case NewOptimizerPattern::Kind::Constant:
		{
			if (_pattern.hasConstant())
				return {_pattern.constant()};
			else
				assertThrow(false, OptimizerException, "cannot create items from non-bound constant pattern without constant value");
		}
		case NewOptimizerPattern::Kind::Operation:
			if (_pattern.hasOperationValues())
			{
				auto operands = _pattern.operands();

				vector<AssemblyItem> instructions{};

				for (auto it = operands.rbegin(); it != operands.rend(); it++)
						instructions += createItems(*it);

				instructions.emplace_back(_pattern.instruction());

				return instructions;
			}
			else
				assertThrow(false, OptimizerException, "cannot create items from non-bound operation pattern without operation values");
		case NewOptimizerPattern::Kind::Unknown:
			assertThrow(false, OptimizerException, "cannot create items from non-bound unknown pattern");
		default:
			assertThrow(false, OptimizerException, "unreachable");
	}
}

template<typename It1, typename It2>
bool iteratorEqual(iterator_pair<It1> _bounds1, iterator_pair<It2> _bounds2)
{
	return (_bounds1.end - _bounds1.begin == _bounds2.end - _bounds2.begin) && std::equal(_bounds1.begin, _bounds1.end, _bounds2.begin);
}

template<typename It, typename>
bool dev::solidity::NewOptimizerPattern::matches(iterator_pair<It> _bounds, bool _unbind)
{
	auto begin = _bounds.begin;
	auto end = _bounds.end;

	if (_unbind) unbind();

	if (isBound())
	{
		auto bound = boundItems();
		return iteratorEqual(bound, _bounds);
	}

	auto size = end - begin;
	assertThrow(size > 0, OptimizerException, "");

	auto back = static_cast<AssemblyItem const&>(*(end - 1));

	auto bind = [&]{ this->bind(_bounds); };

	switch (kind())
	{
		case Kind::Constant:
		{
			if (size != 1) return false;

			if (back.type() != AssemblyItemType::Push) return false;

			if (hasConstant())
			{
				if (begin->data() == constant())
				{
					bind();
					return true;
				}

				return false;
			}
			else
			{
				bind();
				return true;
			}
		}
		case Kind::Operation:
		{
			if (!hasOperationValues())
			{
				if (back.type() == AssemblyItemType::Operation)
				{
					bind();
					return true;
				}

				return false;
			}

			auto const instruction = this->instruction();

			if (back.type() != AssemblyItemType::Operation) return false;
			if (back.instruction() != instruction) return false;

			auto const argumentCount = instructionInfo(instruction).args;
			assertThrow(unsigned(argumentCount) == operands().size(), OptimizerException, "");

			if (argumentCount == 0) return size == 1;

			auto parsedArguments = parseArguments(_bounds);

			auto& requiredArgumentPatterns = operands();

			assertThrow(
				requiredArgumentPatterns.size() == parsedArguments.size(),
				OptimizerException,
				"required argument count and parsed argument count did not match"
			);

			for (unsigned i = 0; i < parsedArguments.size(); i++)
			{
				auto argumentBegin = parsedArguments[i];
				auto argumentEnd = i == 0 ? (end - 1) : parsedArguments[i - 1];

				if (!requiredArgumentPatterns[i].matches(make_iterator_pair(argumentBegin, argumentEnd), false)) return false;
			}

			bind();

			return true;
		}
		case Kind::Any:
			bind();
			return true;
		case Kind::Unknown:
			return false;
		default:
			assertThrow(false, OptimizerException, "invalid pattern kind");
	}
}

template<typename It, typename>
void NewOptimizerPattern::bind(iterator_pair<It> _bounds)
{
	auto begin = _bounds.begin;
	auto end = _bounds.end;

	auto size = end - begin;
	assertThrow(size > 0, OptimizerException, "");

	auto back = static_cast<AssemblyItem const&>(*(end - 1));

	if (kind() == Kind::Constant)
	{
		assertThrow(size == 1 && back.type() == AssemblyItemType::Push, OptimizerException, "invalid bind type");
		if (hasConstant())
			assertThrow(constant() == back.data(), OptimizerException, "invalid bind constant");
	}

	if (kind() == Kind::Operation)
	{
		assertThrow(size != 0 && back.type() == AssemblyItemType::Operation, OptimizerException, "invalid bind value");
		if (hasOperationValues())
			assertThrow(instruction() == back.instruction(), OptimizerException, "invalid bind instruction");
	}

	m_ptr->m_boundItems = _bounds;
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
	return optimize(make_iterator_pair(_items));
}

template<typename It, typename>
vector<AssemblyItem> dev::solidity::ThirdOptimizer::optimize(iterator_pair<It> _bounds)
{
	auto begin = _bounds.begin;
	auto end = _bounds.end;

	if ((end - begin) <= 1) return vector<AssemblyItem>(begin, end);

	auto expressions = parseExpressions(_bounds, -1, true);

	vector<vector<AssemblyItem>> optimizedExpressions(expressions.size());

	for (unsigned i = 0; i < expressions.size(); i++)
	{
		auto expressionBegin = expressions[i];
		auto expressionEnd = i == 0 ? end : expressions[i - 1];

		bool canRun = true;
		bool canDetermineArguments = true;

		std::for_each(
			expressionBegin,
			expressionEnd,
			[&](AssemblyItem item) {
				if (item.returnValues() > 1)
				{
					canRun = false;
					canDetermineArguments = false;
				}

				if (item.type() == AssemblyItemType::Operation && instructionInfo(item.instruction()).sideEffects)
				{
					canRun = false;
				}
			});

		if (!canDetermineArguments) continue;

		vector<AssemblyItem> optimizedExpression(expressionBegin, expressionEnd);

		if (canRun)
			for (auto& rule : m_rules)
				if (rule.pattern.matches(make_iterator_pair(optimizedExpression)))
					optimizedExpression = createItems(rule.action());


		auto noReturnValueStart = noReturnValueBegin(make_iterator_pair(optimizedExpression));

		vector<AssemblyItem> returnPart(optimizedExpression.cbegin(), noReturnValueStart);
		vector<AssemblyItem> noReturnPart(noReturnValueStart, optimizedExpression.cend());

		optimizedExpression = breakAndOptimize(make_iterator_pair(returnPart)) + breakAndOptimize(make_iterator_pair(noReturnPart));

		optimizedExpressions.push_back(optimizedExpression);
	}

	vector<AssemblyItem> optimizedItems;
	for (auto it = optimizedExpressions.rbegin(); it != optimizedExpressions.rend(); it++) optimizedItems += *it;

	if (!iteratorEqual(make_iterator_pair(optimizedItems), _bounds)) optimizedItems = optimize(optimizedItems);

	return optimizedItems;
}

template<typename It, typename>
vector<AssemblyItem> dev::solidity::ThirdOptimizer::breakAndOptimize(iterator_pair<It> _bounds)
{
	auto begin = _bounds.begin;
	auto end = _bounds.end;

	if (end == begin) return {};

	std::vector<AssemblyItem> optimizedExpression{};

	auto arguments = parseArguments(_bounds);

	for (signed i = signed(arguments.size()) - 1; i >= 0; i--)
	{
		auto argumentBegin = arguments.at(i);
		auto argumentEnd = i == 0 ? end - 1 : arguments.at(i - 1);

		optimizedExpression += optimize(make_iterator_pair(argumentBegin, argumentEnd));
	}

	optimizedExpression.push_back(*(end - 1));

	return optimizedExpression;
}
