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
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/AsmData.h>
#include <libdevcore/CommonData.h>
#include <functional>

using namespace std;
using namespace dev;
using namespace yul;

void ForLoopInitRewriter::operator()(Block& _block)
{
	iterateReplacing(
		_block.statements,
		[](Statement& _stmt) -> boost::optional<vector<Statement>>
		{
			if (_stmt.type() == typeid(ForLoop))
			{
				auto& forLoop = boost::get<ForLoop>(_stmt);
				vector<Statement> rewrite;
				swap(rewrite, forLoop.pre.statements);
				rewrite.emplace_back(move(forLoop));
				return rewrite;
			}
			return {};
		}
	);
}
