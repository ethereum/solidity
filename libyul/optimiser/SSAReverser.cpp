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
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/AsmData.h>
#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

void SSAReverser::operator()(Block& _block)
{
	walkVector(_block.statements);
	iterateReplacingWindow<2>(
		_block.statements,
		[&](Statement& _stmt1, Statement& _stmt2) -> boost::optional<vector<Statement>>
		{
			// Replaces
			//   let a_1 := E
			//   a := a_1
			// with
			//   a := E
			//   let a_1 := a

			auto* varDecl = boost::get<VariableDeclaration>(&_stmt1);
			auto* assignment = boost::get<Assignment>(&_stmt2);

			if (!varDecl || !assignment)
				return {};

			auto* identifier = boost::get<Identifier>(assignment->value.get());

			if (
				varDecl->variables.size() == 1 &&
				varDecl->value &&
				assignment->variableNames.size() == 1 &&
				identifier &&
				identifier->name == varDecl->variables.front().name
			)
			{
				vector<Statement> result;
				result.emplace_back(Assignment{
					std::move(assignment->location),
					assignment->variableNames,
					std::move(varDecl->value)
				});
				result.emplace_back(VariableDeclaration{
							std::move(varDecl->location),
							std::move(varDecl->variables),
							std::make_unique<Expression>(std::move(assignment->variableNames.front()))
				});
				return { std::move(result) };
			}
			return {};
		}
	);
}
