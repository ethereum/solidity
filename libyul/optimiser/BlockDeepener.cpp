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
#include <libyul/optimiser/BlockDeepener.h>
#include <libyul/AsmData.h>
#include <libdevcore/Visitor.h>
#include <libdevcore/CommonData.h>
#include <functional>

using namespace std;
using namespace dev;
using namespace yul;

void BlockDeepener::operator()(Block& _block)
{
	ASTModifier::operator()(_block);

	if (_block.statements.size() <= 1)
		return ;
	std::vector<Statement> result;
	std::vector<Statement> *current = &result;
	for (std::size_t i = 0; i < _block.statements.size() - 1; ++i)
	{
		current->emplace_back(std::move(_block.statements[i]));
		current->emplace_back(Block{
			_block.location,
			{}
		});
		current = &boost::get<Block>(current->back()).statements;
	}
	current->emplace_back(std::move(_block.statements.back()));

	_block.statements = std::move(result);
}
