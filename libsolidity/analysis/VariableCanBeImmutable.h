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

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTVisitor.h>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace solidity::langutil
{
class ErrorReporter;
struct SourceLocation;
}


/// Need to figure out the control flow graph. If a state variable is never
/// assigned outside of constructor context, then we can make it immutable.
struct VariableCanBeImmutable: public ASTConstVisitor
{
	VariableCanBeImmutable(ErrorReporter& _errorReporter):
		m_errorReporter(_errorReporter)
	{}

	bool visit(ContractDefinition const& _contractDefinition) override;
	void endVisit(ContractDefinition const& _contractDefinition) override;
	bool visit(FunctionDefinition const& _functionDefinition) override;
	void endVisit(FunctionDefinition const& ) override;
	void endVisit(Identifier const& _identifier) override;
private:
	/// The current contract
	ContractDefinition const* m_contract = nullptr;
	/// The set of functions that can be called from external
	std::set<FunctionDefinition const*> m_reachableFunctions;
	/// Current function;
	FunctionDefinition const* m_function;
	/// Variables that are written to in deploy code.
	std::set<VariableDeclaration const*> m_variablesWrittenTo;
	ErrorReporter& m_errorReporter;
};
