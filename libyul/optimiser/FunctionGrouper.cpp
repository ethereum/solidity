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
 * Optimiser component that changes the code of a block so that all non-function definition
 * statements are moved to a block of their own followed by all function definitions.
 */

#include <libyul/optimiser/FunctionGrouper.h>

#include <libyul/AsmData.h>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace dev;
using namespace yul;
using namespace dev::solidity;


void FunctionGrouper::operator()(Block& _block)
{
	if (alreadyGrouped(_block))
		return;

	vector<Statement> reordered;
	reordered.emplace_back(Block{_block.location, {}});

	for (auto&& statement: _block.statements)
	{
		if (statement.type() == typeid(FunctionDefinition))
			reordered.emplace_back(std::move(statement));
		else
			boost::get<Block>(reordered.front()).statements.emplace_back(std::move(statement));
	}
	_block.statements = std::move(reordered);
}

bool FunctionGrouper::alreadyGrouped(Block const& _block)
{
	if (_block.statements.empty())
		return false;
	if (_block.statements.front().type() != typeid(Block))
		return false;
	for (size_t i = 1; i < _block.statements.size(); ++i)
		if (_block.statements.at(i).type() != typeid(FunctionDefinition))
			return false;
	return true;
}
