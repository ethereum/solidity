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
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Type analyzer and checker.
 */

#pragma once

#include <libsolidity/analysis/TypeChecker.h>
#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTVisitor.h>

namespace dev
{
namespace solidity
{


/**
 * The module that performs type analysis on the AST, checks the applicability of operations on
 * those types and stores errors for invalid operations.
 * Provides a way to retrieve the type of an AST node.
 */
class TypeChecker: private ASTConstVisitor
{
public:
	/// @param _errors the reference to the list of errors and warnings to add them found during type checking.
	TypeChecker(ErrorList& _errors): m_errors(_errors) {}

	/// Performs type checking on the given contract and all of its sub-nodes.
	/// @returns true iff all checks passed. Note even if all checks passed, errors() can still contain warnings
	bool checkTypeRequirements(ASTNode const& _contract);

	/// @returns the type of an expression and asserts that it is present.
	TypePointer const& type(Expression const& _expression) const;
	/// @returns the type of the given variable and throws if the type is not present
	/// (this can happen for variables with non-explicit types before their types are resolved)
	TypePointer const& type(VariableDeclaration const& _variable) const;

private:
	/// Adds a new error to the list of errors.
	void typeError(SourceLocation const& _location, std::string const& _description);

	/// Adds a new warning to the list of errors.
	void warning(SourceLocation const& _location, std::string const& _description);

	/// Adds a new error to the list of errors and throws to abort type checking.
	void fatalTypeError(SourceLocation const& _location, std::string const& _description);

	virtual bool visit(ContractDefinition const& _contract) override;
	/// Checks that two functions defined in this contract with the same name have different
	/// arguments and that there is at most one constructor.
	void checkContractDuplicateFunctions(ContractDefinition const& _contract);
	void checkContractIllegalOverrides(ContractDefinition const& _contract);
	void checkContractAbstractFunctions(ContractDefinition const& _contract);
	void checkContractAbstractConstructors(ContractDefinition const& _contract);
	/// Checks that different functions with external visibility end up having different
	/// external argument types (i.e. different signature).
	void checkContractExternalTypeClashes(ContractDefinition const& _contract);
	/// Checks that all requirements for a library are fulfilled if this is a library.
	void checkLibraryRequirements(ContractDefinition const& _contract);

	virtual void endVisit(InheritanceSpecifier const& _inheritance) override;
	virtual void endVisit(UsingForDirective const& _usingFor) override;
	virtual bool visit(StructDefinition const& _struct) override;
	virtual bool visit(FunctionDefinition const& _function) override;
	virtual bool visit(VariableDeclaration const& _variable) override;
	virtual bool visit(EnumDefinition const& _enum) override;
	/// We need to do this manually because we want to pass the bases of the current contract in
	/// case this is a base constructor call.
	void visitManually(ModifierInvocation const& _modifier, std::vector<ContractDefinition const*> const& _bases);
	virtual bool visit(EventDefinition const& _eventDef) override;
	virtual void endVisit(FunctionTypeName const& _funType) override;
	virtual bool visit(InlineAssembly const& _inlineAssembly) override;
	virtual bool visit(IfStatement const& _ifStatement) override;
	virtual bool visit(WhileStatement const& _whileStatement) override;
	virtual bool visit(ForStatement const& _forStatement) override;
	virtual void endVisit(Return const& _return) override;
	virtual bool visit(VariableDeclarationStatement const& _variable) override;
	virtual void endVisit(ExpressionStatement const& _statement) override;
	virtual bool visit(Conditional const& _conditional) override;
	virtual bool visit(Assignment const& _assignment) override;
	virtual bool visit(TupleExpression const& _tuple) override;
	virtual void endVisit(BinaryOperation const& _operation) override;
	virtual bool visit(UnaryOperation const& _operation) override;
	virtual bool visit(FunctionCall const& _functionCall) override;
	virtual void endVisit(NewExpression const& _newExpression) override;
	virtual bool visit(MemberAccess const& _memberAccess) override;
	virtual bool visit(IndexAccess const& _indexAccess) override;
	virtual bool visit(Identifier const& _identifier) override;
	virtual void endVisit(ElementaryTypeNameExpression const& _expr) override;
	virtual void endVisit(Literal const& _literal) override;

	bool contractDependenciesAreCyclic(
		ContractDefinition const& _contract,
		std::set<ContractDefinition const*> const& _seenContracts = std::set<ContractDefinition const*>()
	) const;

	/// @returns the referenced declaration and throws on error.
	Declaration const& dereference(Identifier const& _identifier) const;
	/// @returns the referenced declaration and throws on error.
	Declaration const& dereference(UserDefinedTypeName const& _typeName) const;

	/// Runs type checks on @a _expression to infer its type and then checks that it is implicitly
	/// convertible to @a _expectedType.
	void expectType(Expression const& _expression, Type const& _expectedType);
	/// Runs type checks on @a _expression to infer its type and then checks that it is an LValue.
	void requireLValue(Expression const& _expression);

	ContractDefinition const* m_scope = nullptr;

	ErrorList& m_errors;
};

}
}
