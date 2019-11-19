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
 * Changes the topmost block to be a function with a specific name ("main") which has no
 * inputs nor outputs.
 */

#include <libyul/optimiser/MainFunction.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/Exceptions.h>

#include <libyul/AsmData.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

void MainFunction::operator()(Block& _block)
{
	assertThrow(_block.statements.size() >= 1, OptimizerException, "");
	assertThrow(holds_alternative<Block>(_block.statements[0]), OptimizerException, "");
	for (size_t i = 1; i < _block.statements.size(); ++i)
		assertThrow(holds_alternative<FunctionDefinition>(_block.statements.at(i)), OptimizerException, "");
	/// @todo this should handle scopes properly and instead of an assertion it should rename the conflicting function
	assertThrow(NameCollector(_block).names().count("main"_yulstring) == 0, OptimizerException, "");

	Block& block = std::get<Block>(_block.statements[0]);
	FunctionDefinition main{
		block.location,
		"main"_yulstring,
		{},
		{},
		std::move(block)
	};
	_block.statements[0] = std::move(main);
}
