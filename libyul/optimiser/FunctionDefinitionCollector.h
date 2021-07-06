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
 * AST walker that finds all function definitions and stores them into a map indexed by the function names.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>

#include <map>

namespace solidity::yul
{

/**
 * AST walker that finds all function definitions and stores them into a map indexed by the function names.
 *
 * Prerequisite: Disambiguator
 */
class FunctionDefinitionCollector: ASTWalker
{
public:
	static std::map<YulString, FunctionDefinition const*> run(Block& _block);
private:
	using ASTWalker::operator();
	void operator()(FunctionDefinition const& _functionDefinition) override;
	std::map<YulString, FunctionDefinition const*> m_functionDefinitions;
};

}
