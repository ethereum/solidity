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

#pragma once

#include <libsolidity/ast/ASTEnums.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTVisitor.h>

#include <libsolidity/interface/ErrorReporter.h>

#include <map>
#include <memory>

namespace dev
{
namespace solidity
{

class ViewPureChecker: private ASTConstVisitor
{
public:
	ViewPureChecker(std::vector<std::shared_ptr<ASTNode>> const& _ast, ErrorReporter& _errorReporter):
		m_ast(_ast), m_errorReporter(_errorReporter) {}

	bool check();

private:
	struct MutabilityAndLocation
	{
		StateMutability mutability;
		SourceLocation location;
	};

	virtual bool visit(FunctionDefinition const& _funDef) override;
	virtual void endVisit(FunctionDefinition const& _funDef) override;
	virtual bool visit(ModifierDefinition const& _modifierDef) override;
	virtual void endVisit(ModifierDefinition const& _modifierDef) override;
	virtual void endVisit(Identifier const& _identifier) override;
	virtual bool visit(MemberAccess const& _memberAccess) override;
	virtual void endVisit(MemberAccess const& _memberAccess) override;
	virtual void endVisit(IndexAccess const& _indexAccess) override;
	virtual void endVisit(ModifierInvocation const& _modifier) override;
	virtual void endVisit(FunctionCall const& _functionCall) override;
	virtual void endVisit(InlineAssembly const& _inlineAssembly) override;

	/// Called when an element of mutability @a _mutability is encountered.
	/// Creates appropriate warnings and errors and sets @a m_currentBestMutability.
	void reportMutability(
		StateMutability _mutability,
		SourceLocation const& _location,
		boost::optional<SourceLocation> const& _nestedLocation = {}
	);

	std::vector<std::shared_ptr<ASTNode>> const& m_ast;
	ErrorReporter& m_errorReporter;

	bool m_errors = false;
	MutabilityAndLocation m_bestMutabilityAndLocation = MutabilityAndLocation{StateMutability::Payable, SourceLocation()};
	FunctionDefinition const* m_currentFunction = nullptr;
	std::map<ModifierDefinition const*, MutabilityAndLocation> m_inferredMutability;
};

}
}
