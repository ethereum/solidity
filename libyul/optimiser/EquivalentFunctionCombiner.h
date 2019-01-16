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
 * Optimiser component that combines syntactically equivalent functions.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/EquivalentFunctionDetector.h>
#include <libyul/AsmDataForward.h>

namespace yul
{

/**
 * Optimiser component that detects syntactically equivalent functions and replaces all calls to any of them by calls
 * to one particular of them.
 *
 * Prerequisite: Disambiguator, Function Hoister
 */
class EquivalentFunctionCombiner: public ASTModifier
{
public:
	static void run(Block& _ast);

	using ASTModifier::operator();
	void operator()(FunctionCall& _funCall) override;

private:
	EquivalentFunctionCombiner(std::map<YulString, FunctionDefinition const*> _duplicates): m_duplicates(std::move(_duplicates)) {}
	std::map<YulString, FunctionDefinition const*> m_duplicates;
};


}
