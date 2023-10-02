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

#include <libsolidity/ast/ASTVisitor.h>
#include <liblangutil/ErrorReporter.h>

namespace solidity::frontend
{

/**
 * Validates access and initialization of immutable variables:
 * must be directly initialized in a c'tor or inline
*/
class ImmutableValidator: private ASTConstVisitor
{
public:
	ImmutableValidator(langutil::ErrorReporter& _errorReporter, ContractDefinition const& _contractDefinition):
		m_mostDerivedContract(_contractDefinition),
		m_errorReporter(_errorReporter)
	{ }

	void analyze();

private:
	bool visit(FunctionDefinition const& _functionDefinition);
	void endVisit(MemberAccess const& _memberAccess);
	void endVisit(Identifier const& _identifier);

	void analyseVariableReference(Declaration const* _variableReference, Expression const& _expression);

	ContractDefinition const& m_mostDerivedContract;

	langutil::ErrorReporter& m_errorReporter;
};

}
