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
#include <libsolidity/experimental/ast/TypeSystem.h>

#include <liblangutil/ErrorReporter.h>

namespace solidity::frontend::experimental
{

class Analysis;

class TypeInference: public ASTConstVisitor
{
public:
	TypeInference(Analysis& _analysis);

	bool analyze(SourceUnit const& _sourceUnit);

	struct Annotation
	{
		/// Expressions, variable declarations, function declarations.
		std::optional<Type> type;
	};
	struct TypeMember
	{
		Type type;
	};
	struct GlobalAnnotation
	{
		std::map<BuiltinClass, TypeClass> builtinClasses;
		std::map<std::string, BuiltinClass> builtinClassesByName;
		std::map<TypeClass, std::map<std::string, Type>> typeClassFunctions;
		std::map<Token, std::tuple<TypeClass, std::string>> operators;
		std::map<TypeConstructor, std::map<std::string, TypeMember>> members;
	};
	bool visit(Block const&) override { return true; }
	bool visit(VariableDeclarationStatement const&) override { return true; }
	void endVisit(VariableDeclarationStatement const& _variableDeclarationStatement) override;
	bool visit(VariableDeclaration const& _variableDeclaration) override;

	bool visit(FunctionDefinition const& _functionDefinition) override;
	void endVisit(FunctionDefinition const& _functionDefinition) override;
	bool visit(ParameterList const&) override { return true; }
	void endVisit(ParameterList const& _parameterList) override;
	bool visit(SourceUnit const&) override { return true; }
	bool visit(ContractDefinition const&) override { return true; }
	bool visit(InlineAssembly const& _inlineAssembly) override;
	bool visit(ImportDirective const&) override { return true; }
	bool visit(PragmaDirective const&) override { return false; }

	bool visit(IfStatement const&) override { return true; }
	void endVisit(IfStatement const& _ifStatement) override;
	bool visit(ExpressionStatement const&) override { return true; }
	bool visit(Assignment const&) override { return true; }
	void endVisit(Assignment const& _assignment) override;
	bool visit(Identifier const&) override;
	bool visit(IdentifierPath const&) override;
	bool visit(FunctionCall const& _functionCall) override;
	void endVisit(FunctionCall const& _functionCall) override;
	bool visit(Return const&) override { return true; }
	void endVisit(Return const& _return) override;

	bool visit(MemberAccess const& _memberAccess) override;
	void endVisit(MemberAccess const& _memberAccess) override;

	bool visit(TypeClassDefinition const& _typeClassDefinition) override;
	bool visit(TypeClassInstantiation const& _typeClassInstantiation) override;
	bool visit(TupleExpression const&) override { return true; }
	void endVisit(TupleExpression const& _tupleExpression) override;
	bool visit(TypeDefinition const& _typeDefinition) override;

	bool visitNode(ASTNode const& _node) override;

	bool visit(BinaryOperation const& _operation) override;

	bool visit(Literal const& _literal) override;
private:
	Analysis& m_analysis;
	langutil::ErrorReporter& m_errorReporter;
	TypeSystem& m_typeSystem;
	TypeEnvironment* m_env = nullptr;
	Type m_voidType;
	Type m_wordType;
	Type m_integerType;
	Type m_unitType;
	Type m_boolType;
	std::optional<Type> m_currentFunctionType;

	Type typeAnnotation(ASTNode const& _node) const;

	Annotation& annotation(ASTNode const& _node);
	Annotation const& annotation(ASTNode const& _node) const;
	GlobalAnnotation& annotation();

	void unify(Type _a, Type _b, langutil::SourceLocation _location = {});
	/// Creates a polymorphic instance of a global type scheme
	Type polymorphicInstance(Type const& _scheme);
	Type memberType(Type _type, std::string _memberName, langutil::SourceLocation _location = {});
	enum class ExpressionContext { Term, Type, Sort };
	Type handleIdentifierByReferencedDeclaration(langutil::SourceLocation _location, Declaration const& _declaration);
	TypeConstructor typeConstructor(Declaration const* _type) const;
	Type type(Declaration const* _type, std::vector<Type> _arguments) const;
	ExpressionContext m_expressionContext = ExpressionContext::Term;
	std::set<TypeClassInstantiation const*, ASTCompareByID<TypeClassInstantiation>> m_activeInstantiations;
};

}
