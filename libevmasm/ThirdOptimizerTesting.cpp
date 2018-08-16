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
It noReturnValueBegin(It _begin, It _end);

template<typename It, typename = typename std::enable_if<is_assembly_item<decltype(*std::declval<It>())>::value>::type>
It backExpressionBegin(It _begin, It _end, bool _allowNoReturn = false)
{
	AssemblyItem const& topLevelItem = *(_end - 1);

	signed arguments = topLevelItem.arguments();
	auto currentEnd = _end - 1 /* top-level operation */;
	while (arguments > 0)
	{
		// TODO this code might break if anything returns more than 1 value, since we can't split things up
		currentEnd = noReturnValueBegin(_begin, currentEnd);
		arguments -= static_cast<AssemblyItem const&>(*(currentEnd - 1)).returnValues();
		currentEnd = backExpressionBegin(_begin, currentEnd, false);
		assertThrow(currentEnd >= _begin, OptimizerException, "currentEnd is before begin");
	}

	if (!_allowNoReturn && topLevelItem.returnValues() == 0) return backExpressionBegin(_begin, currentEnd, false);

	return currentEnd;
}

template<typename It, typename = typename std::enable_if<is_assembly_item<decltype(*std::declval<It>())>::value>::type>
It noReturnValueBegin(It _begin, It _end)
{
	assertThrow(_begin != _end, OptimizerException, "expression is empty");
	AssemblyItem const& topLevelItem = *(_end - 1);
	if (topLevelItem.returnValues() != 0) return _end;
	return backExpressionBegin(_begin, _end, true);
}

template<typename It, typename = typename std::enable_if<is_assembly_item<decltype(*std::declval<It>())>::value>::type>
vector<It> parseExpressions(It _begin, It _end, signed _limit = -1, bool _allowNoReturn = false)
{
	auto currentEnd = _end;
	std::vector<It> expressions{};
	while (currentEnd != _begin && (_limit == -1 || signed(expressions.size()) < _limit))
	{
		auto oldEnd = currentEnd;
		currentEnd = backExpressionBegin(_begin, oldEnd, _allowNoReturn);
		expressions.push_back(currentEnd);
		assertThrow(currentEnd >= _begin, OptimizerException, "currentEnd is before begin");
	}

	return expressions;
}

template<typename It, typename = typename std::enable_if<is_assembly_item<decltype(*std::declval<It>())>::value>::type>
vector<It> parseArguments(It _begin, It _end)
{
	auto size = _end - _begin;
	assertThrow(size > 0, OptimizerException, "cannot get arguments from empty _items vector");

	unsigned neededArguments = (_end - 1)->arguments();

	auto expressions = parseExpressions(_begin, _end - 1, neededArguments, false);
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
bool iteratorEqual(It1 _begin1, It1 _end1, It2 _begin2, It2 _end2)
{
	return (_end1 - _begin1 == _end2 - _begin2) && std::equal(_begin1, _end1, _begin2);
}

template<typename It, typename>
bool dev::solidity::NewOptimizerPattern::matches(It _begin, It _end, bool _unbind)
{
	if (_unbind) unbind();

	if (isBound())
	{
		auto bound = boundItems();
		return iteratorEqual(bound.begin, bound.end, _begin, _end);
	}

	auto size = _end - _begin;
	assertThrow(size > 0, OptimizerException, "");

	auto back = static_cast<AssemblyItem const&>(*(_end - 1));

	auto bind = [&]{ this->bind(_begin, _end); };

	switch (kind())
	{
		case Kind::Constant:
		{
			if (size != 1) return false;

			if (back.type() != AssemblyItemType::Push) return false;

			if (hasConstant())
			{
				if (_begin->data() == constant())
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

			auto parsedArguments = parseArguments(_begin, _end);

			auto& requiredArgumentPatterns = operands();

			assertThrow(
				requiredArgumentPatterns.size() == parsedArguments.size(),
				OptimizerException,
				"required argument count and parsed argument count did not match"
			);

			for (unsigned i = 0; i < parsedArguments.size(); i++)
			{
				auto argumentBegin = parsedArguments[i];
				auto argumentEnd = i == 0 ? (_end - 1) : parsedArguments[i - 1];

				if (!requiredArgumentPatterns[i].matches(argumentBegin, argumentEnd, false)) return false;
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
void NewOptimizerPattern::bind(It _begin, It _end)
{
	auto size = _end - _begin;
	assertThrow(size > 0, OptimizerException, "");

	auto back = static_cast<AssemblyItem const&>(*(_end - 1));

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

	// Temp workaround
	m_ptr->m_boundItems = iterator_pair<AssemblyItemIterator>{_begin, _end};
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
	return optimize(_items.begin(), _items.end());
}

template<typename It, typename>
vector<AssemblyItem> dev::solidity::ThirdOptimizer::optimize(It _begin, It _end)
{
	if (m_rules.empty()) addDefaultRules();

	if ((_end - _begin) <= 1) return vector<AssemblyItem>(_begin, _end);

	auto expressions = parseExpressions(_begin, _end, -1, true);

	vector<vector<AssemblyItem>> optimizedExpressions(expressions.size());

	for (unsigned i = 0; i < expressions.size(); i++)
	{
		auto expressionBegin = expressions[i];
		auto expressionEnd = i == 0 ? _end : expressions[i - 1];

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
				if (rule.pattern.matches(optimizedExpression.begin(), optimizedExpression.end()))
					optimizedExpression = createItems(rule.action());


		auto noReturnValueStart = noReturnValueBegin(optimizedExpression.begin(), optimizedExpression.end());

		vector<AssemblyItem> returnPart(optimizedExpression.begin(), noReturnValueStart);
		vector<AssemblyItem> noReturnPart(noReturnValueStart, optimizedExpression.end());

		optimizedExpression = breakAndOptimize(returnPart.begin(), returnPart.end()) + breakAndOptimize(noReturnPart.begin(), noReturnPart.end());

		optimizedExpressions.push_back(optimizedExpression);
	}

	vector<AssemblyItem> optimizedItems;
	for (auto it = optimizedExpressions.rbegin(); it != optimizedExpressions.rend(); it++) optimizedItems += *it;

	if (!iteratorEqual(optimizedItems.begin(), optimizedItems.end(), _begin, _end)) optimizedItems = optimize(optimizedItems);

	return optimizedItems;
}

template<typename It, typename>
vector<AssemblyItem> dev::solidity::ThirdOptimizer::breakAndOptimize(It _begin, It _end)
{
	if (_end == _begin) return {};

	std::vector<AssemblyItem> optimizedExpression{};

	auto arguments = parseArguments(_begin, _end);

	for (signed i = signed(arguments.size()) - 1; i >= 0; i--)
	{
		auto argumentBegin = arguments.at(i);
		auto argumentEnd = i == 0 ? _end - 1 : arguments.at(i - 1);

		optimizedExpression += optimize(argumentBegin, argumentEnd);
	}

	optimizedExpression.push_back(*(_end - 1));

	return optimizedExpression;
}
