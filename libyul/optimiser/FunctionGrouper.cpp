// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that changes the code of a block so that all non-function definition
 * statements are moved to a block of their own followed by all function definitions.
 */

#include <libyul/optimiser/FunctionGrouper.h>

#include <libyul/AsmData.h>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;


void FunctionGrouper::operator()(Block& _block)
{
	if (alreadyGrouped(_block))
		return;

	vector<Statement> reordered;
	reordered.emplace_back(Block{_block.location, {}});

	for (auto&& statement: _block.statements)
	{
		if (holds_alternative<FunctionDefinition>(statement))
			reordered.emplace_back(std::move(statement));
		else
			std::get<Block>(reordered.front()).statements.emplace_back(std::move(statement));
	}
	_block.statements = std::move(reordered);
}

bool FunctionGrouper::alreadyGrouped(Block const& _block)
{
	if (_block.statements.empty())
		return false;
	if (!holds_alternative<Block>(_block.statements.front()))
		return false;
	for (size_t i = 1; i < _block.statements.size(); ++i)
		if (!holds_alternative<FunctionDefinition>(_block.statements.at(i)))
			return false;
	return true;
}
