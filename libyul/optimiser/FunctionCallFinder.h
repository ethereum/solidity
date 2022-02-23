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
 * AST walker that finds all calls to a function of a given name.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>

#include <vector>

namespace solidity::yul
{

/**
 * AST walker that finds all calls to a function of a given name.
 *
 * Prerequisite: Disambiguator
 */
class FunctionCallFinder: ASTModifier
{
public:
	static std::vector<FunctionCall*> run(Block& _block, YulString _functionName);
private:
	FunctionCallFinder(YulString _functionName);
	using ASTModifier::operator();
	void operator()(FunctionCall& _functionCall) override;
	YulString m_functionName;
	std::vector<FunctionCall*> m_calls;
};

}
