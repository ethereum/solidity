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
#include <libyul/optimiser/BlockFlattener.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libdevcore/Visitor.h>
#include <libdevcore/CommonData.h>
#include <functional>

using namespace std;
using namespace dev;
using namespace dev::yul;

void BlockFlattener::operator()(Block& _block)
{
	ASTModifier::operator()(_block);

	iterateReplacing(
		_block.statements,
		[](Statement& _s) -> boost::optional<vector<Statement>>
		{
			if (_s.type() == typeid(Block))
				return std::move(boost::get<Block>(_s).statements);
			else
				return {};
		}
	);
}
