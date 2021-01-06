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

class FullSSATransform: public ASTModifier
{
public:
	static constexpr char const* name{"FullSSATransform"};
	static void run(OptimiserStepContext& _context, Block& _ast);
	explicit FullSSATransform(OptimiserStepContext& _context);

	using ASTModifier::operator();
	void operator()(VariableDeclaration& _varDecl) override;
	void operator()(Identifier& _varDecl) override;
	void operator()(FunctionDefinition& _functionDefinition) override;
	void operator()(Block& _block) override;
private:
	std::map<YulString, YulString> m_currentSSANames;
	NameDispenser& m_nameDispenser;
	std::vector<YulString> m_currentFunctionReturnVariables;
	std::map<YulString, YulString> m_currentLoopAssignments;

	static Statement makePhiStore(YulString _var, YulString _value);
	static Expression makePhiLoad(YulString _var);
};

}
