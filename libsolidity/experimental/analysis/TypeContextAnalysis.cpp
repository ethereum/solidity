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


#include <libsolidity/experimental/analysis/TypeContextAnalysis.h>

#include <libsolidity/experimental/analysis/TypeClassRegistration.h>
#include <libsolidity/experimental/analysis/TypeRegistration.h>
#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/ast/TypeSystemHelper.h>

#include <liblangutil/Exceptions.h>

#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

TypeContextAnalysis::TypeContextAnalysis(Analysis& _analysis):
	m_analysis(_analysis),
	m_errorReporter(_analysis.errorReporter()),
	m_typeSystem(_analysis.typeSystem()),
	m_env(&m_typeSystem.env()),
	m_voidType(m_typeSystem.type(PrimitiveType::Void, {})),
	m_wordType(m_typeSystem.type(PrimitiveType::Word, {}))
{
}

bool TypeContextAnalysis::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool TypeContextAnalysis::visit(ForAllQuantifier const& _quantifier)
{
	solAssert(m_expressionContext == ExpressionContext::Term);

	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_quantifier.typeVariableDeclarations().accept(*this);
	}

	_quantifier.quantifiedDeclaration().accept(*this);
	return false;
}

bool TypeContextAnalysis::visit(FunctionDefinition const& _functionDefinition)
{
	solAssert(m_expressionContext == ExpressionContext::Term);

	_functionDefinition.parameterList().accept(*this);

	if (_functionDefinition.experimentalReturnExpression())
	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_functionDefinition.experimentalReturnExpression()->accept(*this);
	}

	solAssert(!_functionDefinition.returnParameterList());

	if (_functionDefinition.isImplemented())
		_functionDefinition.body().accept(*this);

	return false;
}

void TypeContextAnalysis::endVisit(ParameterList const& _parameterList)
{
	if (m_expressionContext == ExpressionContext::Term)
		// Terms will be handled by TypeInference.
		return;

	auto& listAnnotation = annotation(_parameterList);
	solAssert(!listAnnotation.type);
	listAnnotation.type = TypeSystemHelpers{m_typeSystem}.tupleType(
		_parameterList.parameters() | ranges::views::transform([&](auto _arg) { return typeAnnotation(*_arg); }) | ranges::to<std::vector<Type>>
	);
}

bool TypeContextAnalysis::visit(TypeClassDefinition const& _typeClassDefinition)
{
	solAssert(m_expressionContext == ExpressionContext::Term);

	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_typeClassDefinition.typeVariable().accept(*this);
	}

	solAssert(m_analysis.annotation<TypeClassRegistration>(_typeClassDefinition).typeClass.has_value());
	TypeClass typeClass = m_analysis.annotation<TypeClassRegistration>(_typeClassDefinition).typeClass.value();
	Type typeVar = m_typeSystem.typeClassVariable(typeClass);
	unify(typeAnnotation(_typeClassDefinition.typeVariable()), typeVar, _typeClassDefinition.location());

	for (auto subNode: _typeClassDefinition.subNodes())
		subNode->accept(*this);

	return false;
}

bool TypeContextAnalysis::visit(BinaryOperation const& _binaryOperation)
{
	auto& operationAnnotation = annotation(_binaryOperation);
	solAssert(!operationAnnotation.type);
	TypeSystemHelpers helper{m_typeSystem};
	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		// Terms will be handled by TypeInference.
		return false;
	case ExpressionContext::Type:
		if (_binaryOperation.getOperator() == Token::Colon)
		{
			_binaryOperation.leftExpression().accept(*this);
			{
				ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Sort};
				_binaryOperation.rightExpression().accept(*this);
			}
			Type leftType = typeAnnotation(_binaryOperation.leftExpression());
			unify(leftType, typeAnnotation(_binaryOperation.rightExpression()), _binaryOperation.location());
			operationAnnotation.type = leftType;
		}
		else if (_binaryOperation.getOperator() == Token::RightArrow)
		{
			_binaryOperation.leftExpression().accept(*this);
			_binaryOperation.rightExpression().accept(*this);
			operationAnnotation.type = helper.functionType(typeAnnotation(_binaryOperation.leftExpression()), typeAnnotation(_binaryOperation.rightExpression()));
		}
		else if (_binaryOperation.getOperator() == Token::BitOr)
		{
			_binaryOperation.leftExpression().accept(*this);
			_binaryOperation.rightExpression().accept(*this);
			operationAnnotation.type = helper.sumType({typeAnnotation(_binaryOperation.leftExpression()), typeAnnotation(_binaryOperation.rightExpression())});
		}
		else
		{
			m_errorReporter.typeError(1439_error, _binaryOperation.location(), "Invalid binary operations in type context.");
			operationAnnotation.type = m_typeSystem.freshTypeVariable({});
		}
		return false;
	case ExpressionContext::Sort:
		m_errorReporter.typeError(1017_error, _binaryOperation.location(), "Invalid binary operation in sort context.");
		operationAnnotation.type = m_typeSystem.freshTypeVariable({});
		return false;
	}

	util::unreachable();
}

bool TypeContextAnalysis::visit(VariableDeclaration const& _variableDeclaration)
{
	solAssert(!_variableDeclaration.value());
	auto& variableAnnotation = annotation(_variableDeclaration);
	solAssert(!variableAnnotation.type);

	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		if (_variableDeclaration.typeExpression())
		{
			ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
			_variableDeclaration.typeExpression()->accept(*this);
		}
		return false;
	case ExpressionContext::Type:
		variableAnnotation.type = m_typeSystem.freshTypeVariable({});
		if (_variableDeclaration.typeExpression())
		{
			ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Sort};
			_variableDeclaration.typeExpression()->accept(*this);
			unify(*variableAnnotation.type, typeAnnotation(*_variableDeclaration.typeExpression()), _variableDeclaration.typeExpression()->location());
		}
		return false;
	case ExpressionContext::Sort:
		m_errorReporter.typeError(2399_error, _variableDeclaration.location(), "Variable declaration in sort context.");
		variableAnnotation.type = m_typeSystem.freshTypeVariable({});
		return false;
	}
	util::unreachable();
}

void TypeContextAnalysis::endVisit(IfStatement const& _ifStatement)
{
	if (m_expressionContext != ExpressionContext::Term)
		m_errorReporter.typeError(2015_error, _ifStatement.location(), "If statement outside term context.");
}

void TypeContextAnalysis::endVisit(Assignment const& _assignment)
{
	if (m_expressionContext != ExpressionContext::Term)
		m_errorReporter.typeError(4337_error, _assignment.location(), "Assignment outside term context.");
}

experimental::Type TypeContextAnalysis::handleIdentifierByReferencedDeclaration(langutil::SourceLocation _location, Declaration const& _declaration)
{
	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		solAssert(false);
	case ExpressionContext::Type:
	{
		if (
			!dynamic_cast<VariableDeclaration const*>(&_declaration) &&
			!dynamic_cast<TypeDefinition const*>(&_declaration)
		)
		{
			SecondarySourceLocation ssl;
			ssl.append("Referenced node.", _declaration.location());
			m_errorReporter.fatalTypeError(2217_error, _location, ssl, "Attempt to type identifier referring to unexpected node.");
		}

		// TODO: Assert that this is a type class variable declaration?
		auto& declarationAnnotation = annotation(_declaration);
		if (!declarationAnnotation.type)
			_declaration.accept(*this);

		solAssert(declarationAnnotation.type);

		if (dynamic_cast<VariableDeclaration const*>(&_declaration))
			return *declarationAnnotation.type;
		else if (dynamic_cast<TypeDefinition const*>(&_declaration))
			return polymorphicInstance(*declarationAnnotation.type);
		else
			solAssert(false);
		break;
	}
	case ExpressionContext::Sort:
	{
		if (auto const* typeClassDefinition = dynamic_cast<TypeClassDefinition const*>(&_declaration))
		{
			solAssert(m_analysis.annotation<TypeClassRegistration>(*typeClassDefinition).typeClass.has_value());
			TypeClass typeClass = m_analysis.annotation<TypeClassRegistration>(*typeClassDefinition).typeClass.value();
			return m_typeSystem.freshTypeVariable(Sort{{typeClass}});
		}
		else
		{
			m_errorReporter.typeError(2599_error, _location, "Expected type class.");
			return m_typeSystem.freshTypeVariable({});
		}
		break;
	}
	}
	util::unreachable();
}

bool TypeContextAnalysis::visit(Identifier const& _identifier)
{
	auto& identifierAnnotation = annotation(_identifier);
	solAssert(!identifierAnnotation.type);

	if (m_expressionContext == ExpressionContext::Term)
		// Terms will be handled by TypeInference.
		return false;

	if (auto const* referencedDeclaration = _identifier.annotation().referencedDeclaration)
	{
		identifierAnnotation.type = handleIdentifierByReferencedDeclaration(_identifier.location(), *referencedDeclaration);
		return false;
	}

	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		solAssert(false);
	case ExpressionContext::Type:
		m_errorReporter.typeError(5934_error, _identifier.location(), "Undeclared type variable.");

		// Assign it a fresh variable anyway just so that we can continue analysis.
		identifierAnnotation.type = m_typeSystem.freshTypeVariable({});
		break;
	case ExpressionContext::Sort:
		// TMP: Is this reachable?
		// TODO: error handling
		solAssert(false);
	}

	return false;
}

void TypeContextAnalysis::endVisit(TupleExpression const& _tupleExpression)
{
	auto& expressionAnnotation = annotation(_tupleExpression);
	solAssert(!expressionAnnotation.type);

	TypeSystemHelpers helper{m_typeSystem};
	auto componentTypes = _tupleExpression.components() | ranges::views::transform([&](auto _expr) -> Type {
		auto& componentAnnotation = annotation(*_expr);
		solAssert(componentAnnotation.type);
		return *componentAnnotation.type;
	}) | ranges::to<std::vector<Type>>;
	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		// Terms will be handled by TypeInference.
		return;
	case ExpressionContext::Type:
		expressionAnnotation.type = helper.tupleType(componentTypes);
		break;
	case ExpressionContext::Sort:
	{
		Type type = m_typeSystem.freshTypeVariable({});
		for (auto componentType: componentTypes)
			unify(type, componentType, _tupleExpression.location());
		expressionAnnotation.type = type;
		break;
	}
	}
}

bool TypeContextAnalysis::visit(IdentifierPath const& _identifierPath)
{
	auto& identifierAnnotation = annotation(_identifierPath);
	solAssert(!identifierAnnotation.type);

	if (m_expressionContext == ExpressionContext::Term)
		// Terms will be handled by TypeInference.
		return false;

	if (auto const* referencedDeclaration = _identifierPath.annotation().referencedDeclaration)
	{
		identifierAnnotation.type = handleIdentifierByReferencedDeclaration(_identifierPath.location(), *referencedDeclaration);
		return false;
	}

	// TODO: error handling
	solAssert(false);
}

bool TypeContextAnalysis::visit(TypeClassInstantiation const& _typeClassInstantiation)
{
	auto& instantiationAnnotation = annotation(_typeClassInstantiation);
	if (instantiationAnnotation.type)
		return false;

	instantiationAnnotation.type = m_voidType;

	// TMP: _typeClassInstantiation.typeConstructor().accept(*this); ?
	// TMP: Visit class name?

	if (_typeClassInstantiation.argumentSorts())
	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_typeClassInstantiation.argumentSorts()->accept(*this);
	}

	for (auto subNode: _typeClassInstantiation.subNodes())
		subNode->accept(*this);

	return false;
}

void TypeContextAnalysis::endVisit(Builtin const& _builtin)
{
	// TODO: This special case should eventually become user-definable.
	annotation(_builtin).type = m_wordType;
}

bool TypeContextAnalysis::visit(MemberAccess const& _memberAccess)
{
	if (m_expressionContext != ExpressionContext::Term)
	{
		m_errorReporter.typeError(5195_error, _memberAccess.location(), "Member access outside term context.");
		annotation(_memberAccess).type = m_typeSystem.freshTypeVariable({});
	}

	return false;
}

bool TypeContextAnalysis::visit(TypeDefinition const& _typeDefinition)
{
	auto& typeDefinitionAnnotation = annotation(_typeDefinition);
	if (typeDefinitionAnnotation.type)
		return false;

	if (_typeDefinition.arguments())
	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_typeDefinition.arguments()->accept(*this);
	}

	std::vector<Type> arguments;
	if (_typeDefinition.arguments())
		for (ASTPointer<VariableDeclaration> argumentDeclaration: _typeDefinition.arguments()->parameters())
		{
			solAssert(argumentDeclaration);
			Type typeVar = typeAnnotation(*argumentDeclaration);
			solAssert(std::holds_alternative<TypeVariable>(typeVar));
			arguments.emplace_back(typeVar);
		}
	m_env->fixTypeVars(arguments);

	Type definedType = type(&_typeDefinition, arguments);
	typeDefinitionAnnotation.type = definedType;

	if (_typeDefinition.typeExpression())
	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_typeDefinition.typeExpression()->accept(*this);
	}

	return false;
}

void TypeContextAnalysis::endVisit(FunctionCall const& _functionCall)
{
	if (m_expressionContext == ExpressionContext::Term)
		// Term context will be handled by TypeInference.
		return;

	auto& functionCallAnnotation = annotation(_functionCall);
	solAssert(!functionCallAnnotation.type);

	Type functionType = typeAnnotation(_functionCall.expression());

	TypeSystemHelpers helper{m_typeSystem};
	std::vector<Type> argTypes;
	for (auto arg: _functionCall.arguments())
	{
		switch (m_expressionContext)
		{
		case ExpressionContext::Term:
			solAssert(false);
		case ExpressionContext::Type:
			argTypes.emplace_back(typeAnnotation(*arg));
			break;
		case ExpressionContext::Sort:
			m_errorReporter.typeError(9173_error, _functionCall.location(), "Function call in sort context.");
			functionCallAnnotation.type = m_typeSystem.freshTypeVariable({});
			break;
		}
	}

	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		solAssert(false);
	case ExpressionContext::Type:
	{
		Type argTuple = helper.tupleType(argTypes);
		Type resultType = m_typeSystem.freshTypeVariable({});
		Type genericFunctionType = helper.typeFunctionType(argTuple, resultType);
		unify(functionType, genericFunctionType, _functionCall.location());
		functionCallAnnotation.type = resultType;
		break;
	}
	case ExpressionContext::Sort:
		solAssert(false);
	}
}

void TypeContextAnalysis::endVisit(Literal const&)
{
	if (m_expressionContext != ExpressionContext::Term)
		solUnimplementedAssert(m_expressionContext == ExpressionContext::Term, "Literals in type context are not supported yet.");
}


experimental::Type TypeContextAnalysis::polymorphicInstance(Type const& _scheme)
{
	return m_env->fresh(_scheme);
}

// TMP: Do I still need it here?
void TypeContextAnalysis::unify(Type _a, Type _b, langutil::SourceLocation _location)
{
	auto unificationFailures = m_env->unify(_a, _b);

	for (auto failure: unificationFailures)
	{
		TypeEnvironmentHelpers envHelper{*m_env};
		std::visit(util::GenericVisitor{
			[&](TypeEnvironment::TypeMismatch _typeMismatch) {
				m_errorReporter.typeError(
					8456_error,
					_location,
					fmt::format(
						"Cannot unify {} and {}.",
						envHelper.typeToString(_typeMismatch.a),
						envHelper.typeToString(_typeMismatch.b)
					)
				);
			},
			[&](TypeEnvironment::SortMismatch _sortMismatch) {
				m_errorReporter.typeError(3111_error, _location, fmt::format(
					"{} does not have sort {}",
					envHelper.typeToString(_sortMismatch.type),
					TypeSystemHelpers{m_typeSystem}.sortToString(_sortMismatch.sort)
				));
			},
			[&](TypeEnvironment::RecursiveUnification _recursiveUnification) {
				m_errorReporter.typeError(
					6460_error,
					_location,
					fmt::format(
						"Recursive unification: {} occurs in {}.",
						envHelper.typeToString(_recursiveUnification.var),
						envHelper.typeToString(_recursiveUnification.type)
					)
				);
			}
		}, failure);
	}
}

experimental::Type TypeContextAnalysis::typeAnnotation(ASTNode const& _node) const
{
	auto result = annotation(_node).type;
	solAssert(result);
	return *result;
}
TypeConstructor TypeContextAnalysis::typeConstructor(Declaration const* _type) const
{
	if (auto const& constructor = m_analysis.annotation<TypeRegistration>(*_type).typeConstructor)
		return *constructor;
	m_errorReporter.fatalTypeError(5904_error, _type->location(), "Unregistered type.");
	util::unreachable();
}
experimental::Type TypeContextAnalysis::type(Declaration const* _type, std::vector<Type> _arguments) const
{
	return m_typeSystem.type(typeConstructor(_type), std::move(_arguments));
}

TypeContextAnalysis::Annotation& TypeContextAnalysis::annotation(ASTNode const& _node)
{
	return m_analysis.annotation<TypeContextAnalysis>(_node);
}

TypeContextAnalysis::Annotation const& TypeContextAnalysis::annotation(ASTNode const& _node) const
{
	return m_analysis.annotation<TypeContextAnalysis>(_node);
}

TypeContextAnalysis::GlobalAnnotation& TypeContextAnalysis::annotation()
{
	return m_analysis.annotation<TypeContextAnalysis>();
}
