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
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Type analyzer and checker.
 */

#pragma once

#include <liblangutil/EVMVersion.h>

#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/ast/Types.h>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::frontend
{

/**
 * The module that performs type analysis on the AST, checks the applicability of operations on
 * those types and stores errors for invalid operations.
 * Provides a way to retrieve the type of an AST node.
 */
class TypeChecker: private ASTConstVisitor
{
public:
	/// @param _errorReporter provides the error logging functionality.
	TypeChecker(langutil::EVMVersion _evmVersion, langutil::ErrorReporter& _errorReporter):
		m_evmVersion(_evmVersion),
		m_errorReporter(_errorReporter)
	{}

	/// Performs type checking on the given source and all of its sub-nodes.
	/// @returns true iff all checks passed. Note even if all checks passed, errors() can still contain warnings
	bool checkTypeRequirements(SourceUnit const& _source);

	/// @returns the type of an expression and asserts that it is present.
	Type const* type(Expression const& _expression) const;
	/// @returns the type of the given variable and throws if the type is not present
	/// (this can happen for variables with non-explicit types before their types are resolved)
	Type const* type(VariableDeclaration const& _variable) const;

	static bool typeSupportedByOldABIEncoder(Type const& _type, bool _isLibraryCall);

private:

	bool visit(ContractDefinition const& _contract) override;
	/// Checks (and warns) if a tuple assignment might cause unexpected overwrites in storage.
	/// Should only be called if the left hand side is tuple-typed.
	void checkDoubleStorageAssignment(Assignment const& _assignment);
	// Checks whether the expression @arg _expression can be assigned from type @arg _type
	// and reports an error, if not.
	void checkExpressionAssignment(Type const& _type, Expression const& _expression);

	/// Performs type checks for ``abi.decode(bytes memory, (...))`` and returns the
	/// vector of return types (which is basically the second argument) if successful. It returns
	/// the empty vector on error.
	TypePointers typeCheckABIDecodeAndRetrieveReturnType(
		FunctionCall const& _functionCall,
		bool _abiEncoderV2
	);

	TypePointers typeCheckMetaTypeFunctionAndRetrieveReturnType(FunctionCall const& _functionCall);

	/// Performs type checks and determines result types for type conversion FunctionCall nodes.
	Type const* typeCheckTypeConversionAndRetrieveReturnType(
		FunctionCall const& _functionCall
	);

	/// Performs type checks on function call and struct ctor FunctionCall nodes (except for kind ABIDecode).
	void typeCheckFunctionCall(
		FunctionCall const& _functionCall,
		FunctionTypePointer _functionType
	);

	void typeCheckFallbackFunction(FunctionDefinition const& _function);
	void typeCheckConstructor(FunctionDefinition const& _function);

	/// Performs general number and type checks of arguments against function call and struct ctor FunctionCall node parameters.
	void typeCheckFunctionGeneralChecks(
		FunctionCall const& _functionCall,
		FunctionTypePointer _functionType
	);

	/// Performs general checks and checks specific to ABI encode functions
	void typeCheckABIEncodeFunctions(
		FunctionCall const& _functionCall,
		FunctionTypePointer _functionType
	);

	/// Performs checks specific to the ABI encode functions of type ABIEncodeCall
	void typeCheckABIEncodeCallFunction(FunctionCall const& _functionCall);

	/// Performs general checks and checks specific to string concat function call
	void typeCheckStringConcatFunction(
		FunctionCall const& _functionCall,
		FunctionType const* _functionType
	);

	/// Performs general checks and checks specific to bytes concat function call
	void typeCheckBytesConcatFunction(
		FunctionCall const& _functionCall,
		FunctionType const* _functionType
	);

	bool visit(ImportDirective const&) override;

	void endVisit(InheritanceSpecifier const& _inheritance) override;
	void endVisit(ModifierDefinition const& _modifier) override;
	bool visit(FunctionDefinition const& _function) override;
	void endVisit(ArrayTypeName const& _typeName) override;
	bool visit(VariableDeclaration const& _variable) override;
	void endVisit(StructDefinition const& _struct) override;
	/// We need to do this manually because we want to pass the bases of the current contract in
	/// case this is a base constructor call.
	void visitManually(ModifierInvocation const& _modifier, std::vector<ContractDefinition const*> const& _bases);
	bool visit(EventDefinition const& _eventDef) override;
	bool visit(ErrorDefinition const& _errorDef) override;
	void endVisit(FunctionTypeName const& _funType) override;
	bool visit(InlineAssembly const& _inlineAssembly) override;
	bool visit(IfStatement const& _ifStatement) override;
	void endVisit(TryStatement const& _tryStatement) override;
	bool visit(WhileStatement const& _whileStatement) override;
	bool visit(ForStatement const& _forStatement) override;
	void endVisit(Return const& _return) override;
	void endVisit(EmitStatement const& _emit) override;
	void endVisit(RevertStatement const& _revert) override;
	bool visit(VariableDeclarationStatement const& _variable) override;
	void endVisit(ExpressionStatement const& _statement) override;
	bool visit(Conditional const& _conditional) override;
	bool visit(Assignment const& _assignment) override;
	bool visit(TupleExpression const& _tuple) override;
	void endVisit(BinaryOperation const& _operation) override;
	bool visit(UnaryOperation const& _operation) override;
	bool visit(FunctionCall const& _functionCall) override;
	bool visit(FunctionCallOptions const& _functionCallOptions) override;
	void endVisit(NewExpression const& _newExpression) override;
	bool visit(MemberAccess const& _memberAccess) override;
	bool visit(IndexAccess const& _indexAccess) override;
	bool visit(IndexRangeAccess const& _indexRangeAccess) override;
	bool visit(Identifier const& _identifier) override;
	void endVisit(IdentifierPath const& _identifierPath) override;
	void endVisit(UserDefinedTypeName const& _userDefinedTypeName) override;
	void endVisit(ElementaryTypeNameExpression const& _expr) override;
	void endVisit(Literal const& _literal) override;
	void endVisit(UsingForDirective const& _usingForDirective) override;

	void checkErrorAndEventParameters(CallableDeclaration const& _callable);

	/// @returns the referenced declaration and throws on error.
	Declaration const& dereference(Identifier const& _identifier) const;
	/// @returns the referenced declaration and throws on error.
	Declaration const& dereference(IdentifierPath const& _path) const;

	std::vector<Declaration const*> cleanOverloadedDeclarations(
		Identifier const& _reference,
		std::vector<Declaration const*> const& _candidates
	);

	/// Runs type checks on @a _expression to infer its type and then checks that it is implicitly
	/// convertible to @a _expectedType.
	bool expectType(Expression const& _expression, Type const& _expectedType);
	/// Runs type checks on @a _expression to infer its type and then checks that it is an LValue.
	void requireLValue(Expression const& _expression, bool _ordinaryAssignment);

	bool useABICoderV2() const;

	/// @returns the current scope that can have function or type definitions.
	/// This is either a contract or a source unit.
	ASTNode const* currentDefinitionScope() const
	{
		if (m_currentContract)
			return m_currentContract;
		else
			return m_currentSourceUnit;
	}

	SourceUnit const* m_currentSourceUnit = nullptr;
	ContractDefinition const* m_currentContract = nullptr;

	langutil::EVMVersion m_evmVersion;

	langutil::ErrorReporter& m_errorReporter;
};

}
