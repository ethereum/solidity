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

#include <libsolidity/analysis/ImmutableValidator.h>

#include <range/v3/view/reverse.hpp>

using namespace solidity::frontend;
using namespace solidity::langutil;

void ImmutableValidator::analyze()
{
	auto linearizedContracts = m_mostDerivedContract.annotation().linearizedBaseContracts | ranges::views::reverse;

	for (ContractDefinition const* contract: linearizedContracts)
	{
		for (FunctionDefinition const* function: contract->definedFunctions())
			function->accept(*this);

		for (ModifierDefinition const* modifier: contract->functionModifiers())
			modifier->accept(*this);
	}
}

bool ImmutableValidator::visit(FunctionDefinition const& _functionDefinition)
{
	return !_functionDefinition.isConstructor();
}

void ImmutableValidator::endVisit(MemberAccess const& _memberAccess)
{
	analyseVariableReference(_memberAccess.annotation().referencedDeclaration, _memberAccess);
}

void ImmutableValidator::endVisit(Identifier const& _identifier)
{
	analyseVariableReference(_identifier.annotation().referencedDeclaration, _identifier);
}

void ImmutableValidator::analyseVariableReference(Declaration const* _reference, Expression const& _expression)
{
	auto const* variable = dynamic_cast<VariableDeclaration const*>(_reference);
	if (!variable || !variable->isStateVariable() || !variable->immutable())
		return;

	// If this is not an ordinary assignment, we write and read at the same time.
	if (_expression.annotation().willBeWrittenTo)
		m_errorReporter.typeError(
			1581_error,
			_expression.location(),
			"Cannot write to immutable here: Immutable variables can only be initialized inline or assigned directly in the constructor."
		);
}
