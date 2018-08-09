#include <libevmasm/ThirdOptimizerTesting.h>
#include "ThirdOptimizerTesting.h"
#include <algorithm>
#include <tuple>

bool dev::solidity::NewOptimizerPattern::matches(std::vector<AssemblyItem> _items) const
{
	switch (kind())
	{
		case Kind::Constant:
		{
			if (_items.size() != 1) return false;

			if (m_hasConstant)
				return _items[0].data() == constant();
			else
				return _items[0].type() == AssemblyItemType::Push;
		}
		case Kind::Operation:
		{
			if (!m_hasOperationValues) return _items.back().type() == AssemblyItemType::Operation;

			auto const instruction = this->instruction();

			if (_items.back().type() != AssemblyItemType::Operation) return false;
			if (_items.back().instruction() != instruction) return false;

			auto const argumentCount = instructionInfo(instruction).args;
			assertThrow(unsigned(argumentCount) == operands().size(), OptimizerException, "");

			if (argumentCount == 0) return _items.size() == 1;

			auto parsedArguments = parseArguments(_items);

			if (parsedArguments.empty()) return false;

			auto requiredArgumentPatterns = operands();

			assertThrow(
				requiredArgumentPatterns.size() == parsedArguments.size(),
				OptimizerException,
				"required argument count and parsed argument count did not match"
			);

			for (unsigned i = 0; i < parsedArguments.size(); i++)
			{
				auto argument = parsedArguments[i];

				std::reverse(argument.begin(), argument.end());

				if (!requiredArgumentPatterns[i].matches(argument)) return false;
			}

			return true;
		}
		case Kind::Any:
			return true;
		case Kind::Unknown:
			return false;
		default:
			assertThrow(false, OptimizerException, "invalid pattern kind");
	}
}

vector<vector<AssemblyItem>> dev::solidity::NewOptimizerPattern::parseArguments(vector<AssemblyItem> const& _items)
{
	auto argumentsFound = 0;

	signed int stackHeight = 0;

	std::vector<vector<AssemblyItem>> arguments{};
	std::vector<AssemblyItem> currentArgument{};

	for (auto it = ++_items.rbegin(); it != _items.rend(); it++)
	{
		stackHeight -= it->arguments();
		stackHeight += it->returnValues();

		currentArgument.push_back(*it);

		if (!currentArgument.empty() && stackHeight == argumentsFound + 1)
		{
			arguments.push_back(currentArgument);
			currentArgument.clear();
			argumentsFound++;
		}
	}

	if (!currentArgument.empty()) return {}; // Did not parse all of the items

	return arguments;
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



vector<AssemblyItem> dev::solidity::ThirdOptimizer::optimize(vector<AssemblyItem> _items)
{
	if (m_rules.empty()) addDefaultRules();



	return _items;
}
