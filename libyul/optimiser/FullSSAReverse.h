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
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/KnowledgeBase.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/YulString.h>
#include <libyul/AST.h>
#include <libyul/SideEffects.h>

#include <libsolutil/InvertibleMap.h>

#include <map>
#include <set>

namespace solidity::yul
{
struct Dialect;

class FullSSAReverse: public ASTModifier
{
public:
	static constexpr char const* name{"FullSSAReverse"};
	static void run(OptimiserStepContext& _context, Block& _ast);
	using ASTModifier::operator();
	void operator()(Block& _block) override;
	void operator()(FunctionDefinition& _funDef) override;
private:
	FullSSAReverse(NameDispenser& _nameDispenser): m_nameDispenser(_nameDispenser) {}
	NameDispenser& m_nameDispenser;
	std::map<YulString, YulString> m_variableNames;
	std::set<YulString> m_currentFunctionReturns;
};

}
