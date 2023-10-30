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


#include <libsolidity/experimental/analysis/TypeInference.h>

#include <libsolidity/experimental/analysis/TypeClassRegistration.h>
#include <libsolidity/experimental/analysis/TypeRegistration.h>
#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/ast/TypeSystemHelper.h>

#include <libsolutil/Numeric.h>
#include <libsolutil/StringUtils.h>
#include <liblangutil/Exceptions.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

#include <boost/algorithm/string.hpp>
#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

TypeInference::TypeInference(Analysis& _analysis):
	m_analysis(_analysis),
	m_errorReporter(_analysis.errorReporter()),
	m_typeSystem(_analysis.typeSystem()),
	m_env(&m_typeSystem.env()),
	m_voidType(m_typeSystem.type(PrimitiveType::Void, {})),
	m_wordType(m_typeSystem.type(PrimitiveType::Word, {})),
	m_integerType(m_typeSystem.type(PrimitiveType::Integer, {})),
	m_unitType(m_typeSystem.type(PrimitiveType::Unit, {})),
	m_boolType(m_typeSystem.type(PrimitiveType::Bool, {}))
{
	TypeSystemHelpers helper{m_typeSystem};

	auto declareBuiltinClass = [&](std::string _name, BuiltinClass _class) -> TypeClass {
		auto result = m_typeSystem.declareTypeClass(_name, nullptr);
		if (auto error = std::get_if<std::string>(&result))
			solAssert(!error, *error);
		TypeClass declaredClass = std::get<TypeClass>(result);
		// TODO: validation?
		solAssert(annotation().builtinClassesByName.emplace(_name, _class).second);
		return annotation().builtinClasses.emplace(_class, declaredClass).first->second;
	};

	auto registeredTypeClass = [&](BuiltinClass _builtinClass) -> TypeClass {
		return annotation().builtinClasses.at(_builtinClass);
	};

	auto defineConversion = [&](BuiltinClass _builtinClass, PrimitiveType _fromType, std::string _functionName) {
		annotation().typeClassFunctions[registeredTypeClass(_builtinClass)] = {{
			std::move(_functionName),
			helper.functionType(
				m_typeSystem.type(_fromType, {}),
				m_typeSystem.typeClassInfo(registeredTypeClass(_builtinClass)).typeVariable
			),
		}};
	};

	auto defineBinaryMonoidalOperator = [&](BuiltinClass _builtinClass, Token _token, std::string _functionName) {
		Type typeVar = m_typeSystem.typeClassInfo(registeredTypeClass(_builtinClass)).typeVariable;
		annotation().operators.emplace(_token, std::make_tuple(registeredTypeClass(_builtinClass), _functionName));
		annotation().typeClassFunctions[registeredTypeClass(_builtinClass)] = {{
			std::move(_functionName),
			helper.functionType(
				helper.tupleType({typeVar, typeVar}),
				typeVar
			)
		}};
	};

	auto defineBinaryCompareOperator = [&](BuiltinClass _builtinClass, Token _token, std::string _functionName) {
		Type typeVar = m_typeSystem.typeClassInfo(registeredTypeClass(_builtinClass)).typeVariable;
		annotation().operators.emplace(_token, std::make_tuple(registeredTypeClass(_builtinClass), _functionName));
		annotation().typeClassFunctions[registeredTypeClass(_builtinClass)] = {{
			std::move(_functionName),
			helper.functionType(
				helper.tupleType({typeVar, typeVar}),
				m_typeSystem.type(PrimitiveType::Bool, {})
			)
		}};
	};

	declareBuiltinClass("integer", BuiltinClass::Integer);
	declareBuiltinClass("*", BuiltinClass::Mul);
	declareBuiltinClass("+", BuiltinClass::Add);
	declareBuiltinClass("==", BuiltinClass::Equal);
	declareBuiltinClass("<", BuiltinClass::Less);
	declareBuiltinClass("<=", BuiltinClass::LessOrEqual);
	declareBuiltinClass(">", BuiltinClass::Greater);
	declareBuiltinClass(">=", BuiltinClass::GreaterOrEqual);

	defineConversion(BuiltinClass::Integer, PrimitiveType::Integer, "fromInteger");

	defineBinaryMonoidalOperator(BuiltinClass::Mul, Token::Mul, "mul");
	defineBinaryMonoidalOperator(BuiltinClass::Add, Token::Add, "add");

	defineBinaryCompareOperator(BuiltinClass::Equal, Token::Equal, "eq");
	defineBinaryCompareOperator(BuiltinClass::Less, Token::LessThan, "lt");
	defineBinaryCompareOperator(BuiltinClass::LessOrEqual, Token::LessThanOrEqual, "leq");
	defineBinaryCompareOperator(BuiltinClass::Greater, Token::GreaterThan, "gt");
	defineBinaryCompareOperator(BuiltinClass::GreaterOrEqual, Token::GreaterThanOrEqual, "geq");
}

bool TypeInference::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool TypeInference::visit(FunctionDefinition const& _functionDefinition)
{
	solAssert(m_expressionContext == ExpressionContext::Term);
	auto& functionAnnotation = annotation(_functionDefinition);
	if (functionAnnotation.type)
		return false;

	ScopedSaveAndRestore signatureRestore(m_currentFunctionType, std::nullopt);

	Type argumentsType = m_typeSystem.freshTypeVariable({});
	Type returnType = m_typeSystem.freshTypeVariable({});
	Type functionType = TypeSystemHelpers{m_typeSystem}.functionType(argumentsType, returnType);

	m_currentFunctionType = functionType;
	functionAnnotation.type = functionType;


	_functionDefinition.parameterList().accept(*this);
	unify(argumentsType, typeAnnotation(_functionDefinition.parameterList()), _functionDefinition.parameterList().location());
	if (_functionDefinition.experimentalReturnExpression())
	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_functionDefinition.experimentalReturnExpression()->accept(*this);
		unify(
			returnType,
			typeAnnotation(*_functionDefinition.experimentalReturnExpression()),
			_functionDefinition.experimentalReturnExpression()->location()
		);
	}
	else
		unify(returnType, m_unitType, _functionDefinition.location());

	if (_functionDefinition.isImplemented())
		_functionDefinition.body().accept(*this);

	return false;
}

void TypeInference::endVisit(FunctionDefinition const& _functionDefinition)
{
	solAssert(m_expressionContext == ExpressionContext::Term);
	solAssert(annotation(_functionDefinition).type.has_value());

	Type& functionType = *annotation(_functionDefinition).type;
	m_env->fixTypeVars(TypeEnvironmentHelpers{*m_env}.typeVars(functionType));
}

void TypeInference::endVisit(Return const& _return)
{
	solAssert(m_currentFunctionType);
	Type functionReturnType = std::get<1>(TypeSystemHelpers{m_typeSystem}.destFunctionType(*m_currentFunctionType));
	if (_return.expression())
		unify(functionReturnType, typeAnnotation(*_return.expression()), _return.location());
	else
		unify(functionReturnType, m_unitType, _return.location());
}

void TypeInference::endVisit(ParameterList const& _parameterList)
{
	auto& listAnnotation = annotation(_parameterList);
	solAssert(!listAnnotation.type);
	listAnnotation.type = TypeSystemHelpers{m_typeSystem}.tupleType(
		_parameterList.parameters() | ranges::views::transform([&](auto _arg) { return typeAnnotation(*_arg); }) | ranges::to<std::vector<Type>>
	);
}

bool TypeInference::visit(TypeClassDefinition const& _typeClassDefinition)
{
	solAssert(m_expressionContext == ExpressionContext::Term);
	auto& typeClassDefinitionAnnotation = annotation(_typeClassDefinition);
	if (typeClassDefinitionAnnotation.type)
		return false;

	typeClassDefinitionAnnotation.type = type(&_typeClassDefinition, {});

	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_typeClassDefinition.typeVariable().accept(*this);
	}

	std::map<std::string, Type> functionTypes;

	solAssert(m_analysis.annotation<TypeClassRegistration>(_typeClassDefinition).typeClass.has_value());
	TypeClass typeClass = m_analysis.annotation<TypeClassRegistration>(_typeClassDefinition).typeClass.value();
	Type typeVar = m_typeSystem.typeClassVariable(typeClass);
	unify(typeAnnotation(_typeClassDefinition.typeVariable()), typeVar, _typeClassDefinition.location());

	auto& typeMembersAnnotation = annotation().members[typeConstructor(&_typeClassDefinition)];

	for (auto subNode: _typeClassDefinition.subNodes())
	{
		subNode->accept(*this);
		auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(subNode.get());
		solAssert(functionDefinition);
		auto functionType = typeAnnotation(*functionDefinition);
		if (!functionTypes.emplace(functionDefinition->name(), functionType).second)
			m_errorReporter.fatalTypeError(3195_error, functionDefinition->location(), "Function in type class declared multiple times.");
		auto typeVars = TypeEnvironmentHelpers{*m_env}.typeVars(functionType);
		if (typeVars.size() != 1)
			m_errorReporter.fatalTypeError(8379_error, functionDefinition->location(), "Function in type class may only depend on the type class variable.");
		unify(typeVars.front(), typeVar, functionDefinition->location());
		typeMembersAnnotation[functionDefinition->name()] = TypeMember{functionType};
	}

	annotation().typeClassFunctions[typeClass] = std::move(functionTypes);

	for (auto [functionName, functionType]: functionTypes)
	{
		TypeEnvironmentHelpers helper{*m_env};
		auto typeVars = helper.typeVars(functionType);
		if (typeVars.empty())
			m_errorReporter.typeError(1723_error, _typeClassDefinition.location(), "Function " + functionName + " does not depend on class variable.");
		if (typeVars.size() > 2)
			m_errorReporter.typeError(6387_error, _typeClassDefinition.location(), "Function " + functionName + " depends on multiple type variables.");
		if (!m_env->typeEquals(typeVars.front(), typeVar))
			m_errorReporter.typeError(1807_error, _typeClassDefinition.location(), "Function " + functionName + " depends on invalid type variable.");
	}

	for (auto instantiation: m_analysis.annotation<TypeRegistration>(_typeClassDefinition).instantiations | ranges::views::values)
		// TODO: recursion-safety? Order of instantiation?
		instantiation->accept(*this);

	return false;
}

bool TypeInference::visit(InlineAssembly const& _inlineAssembly)
{
	// External references have already been resolved in a prior stage and stored in the annotation.
	// We run the resolve step again regardless.
	yul::ExternalIdentifierAccess::Resolver identifierAccess = [&](
		yul::Identifier const& _identifier,
		yul::IdentifierContext _context,
		bool
	) -> bool
	{
		if (_context == yul::IdentifierContext::NonExternal)
		{
			// TODO: do we need this?
			// Hack until we can disallow any shadowing: If we found an internal reference,
			// clear the external references, so that codegen does not use it.
			_inlineAssembly.annotation().externalReferences.erase(& _identifier);
			return false;
		}
		InlineAssemblyAnnotation::ExternalIdentifierInfo* identifierInfo = util::valueOrNullptr(_inlineAssembly.annotation().externalReferences, &_identifier);
		if (!identifierInfo)
			return false;
		Declaration const* declaration = identifierInfo->declaration;
		solAssert(!!declaration, "");
		solAssert(identifierInfo->suffix == "", "");

		unify(typeAnnotation(*declaration), m_wordType, originLocationOf(_identifier));
		identifierInfo->valueSize = 1;
		return true;
	};
	solAssert(!_inlineAssembly.annotation().analysisInfo, "");
	_inlineAssembly.annotation().analysisInfo = std::make_shared<yul::AsmAnalysisInfo>();
	yul::AsmAnalyzer analyzer(
		*_inlineAssembly.annotation().analysisInfo,
		m_errorReporter,
		_inlineAssembly.dialect(),
		identifierAccess
	);
	if (!analyzer.analyze(_inlineAssembly.operations()))
		solAssert(m_errorReporter.hasErrors());
	return false;
}

bool TypeInference::visit(BinaryOperation const& _binaryOperation)
{
	auto& operationAnnotation = annotation(_binaryOperation);
	solAssert(!operationAnnotation.type);
	TypeSystemHelpers helper{m_typeSystem};
	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		if (auto* operatorInfo = util::valueOrNullptr(annotation().operators, _binaryOperation.getOperator()))
		{
			auto [typeClass, functionName] = *operatorInfo;
			// TODO: error robustness?
			Type functionType = m_env->fresh(annotation().typeClassFunctions.at(typeClass).at(functionName));

			_binaryOperation.leftExpression().accept(*this);
			_binaryOperation.rightExpression().accept(*this);

			Type argTuple = helper.tupleType({typeAnnotation(_binaryOperation.leftExpression()), typeAnnotation(_binaryOperation.rightExpression())});
			Type resultType = m_typeSystem.freshTypeVariable({});
			Type genericFunctionType = helper.functionType(argTuple, resultType);
			unify(functionType, genericFunctionType, _binaryOperation.location());

			operationAnnotation.type = resultType;
		}
		else if (_binaryOperation.getOperator() == Token::Colon)
		{
			_binaryOperation.leftExpression().accept(*this);
			{
				ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
				_binaryOperation.rightExpression().accept(*this);
			}
			Type leftType = typeAnnotation(_binaryOperation.leftExpression());
			unify(leftType, typeAnnotation(_binaryOperation.rightExpression()), _binaryOperation.location());
			operationAnnotation.type = leftType;
		}
		else
		{
			m_errorReporter.typeError(4504_error, _binaryOperation.location(), "Binary operation in term context not yet supported.");
			operationAnnotation.type = m_typeSystem.freshTypeVariable({});
		}
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
	return false;
}

void TypeInference::endVisit(VariableDeclarationStatement const& _variableDeclarationStatement)
{
	solAssert(m_expressionContext == ExpressionContext::Term);
	if (_variableDeclarationStatement.declarations().size () != 1)
	{
		m_errorReporter.typeError(2655_error, _variableDeclarationStatement.location(), "Multi variable declaration not supported.");
		return;
	}
	Type variableType = typeAnnotation(*_variableDeclarationStatement.declarations().front());
	if (_variableDeclarationStatement.initialValue())
		unify(variableType, typeAnnotation(*_variableDeclarationStatement.initialValue()), _variableDeclarationStatement.location());
}

bool TypeInference::visit(VariableDeclaration const& _variableDeclaration)
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
			variableAnnotation.type = typeAnnotation(*_variableDeclaration.typeExpression());
			return false;
		}
		variableAnnotation.type = m_typeSystem.freshTypeVariable({});
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

void TypeInference::endVisit(IfStatement const& _ifStatement)
{
	auto& ifAnnotation = annotation(_ifStatement);
	solAssert(!ifAnnotation.type);

	if (m_expressionContext != ExpressionContext::Term)
	{
		m_errorReporter.typeError(2015_error, _ifStatement.location(), "If statement outside term context.");
		ifAnnotation.type = m_typeSystem.freshTypeVariable({});
		return;
	}

	unify(typeAnnotation(_ifStatement.condition()), m_boolType, _ifStatement.condition().location());

	ifAnnotation.type = m_unitType;
}

void TypeInference::endVisit(Assignment const& _assignment)
{
	auto& assignmentAnnotation = annotation(_assignment);
	solAssert(!assignmentAnnotation.type);

	if (m_expressionContext != ExpressionContext::Term)
	{
		m_errorReporter.typeError(4337_error, _assignment.location(), "Assignment outside term context.");
		assignmentAnnotation.type = m_typeSystem.freshTypeVariable({});
		return;
	}

	Type leftType = typeAnnotation(_assignment.leftHandSide());
	unify(leftType, typeAnnotation(_assignment.rightHandSide()), _assignment.location());
	assignmentAnnotation.type = leftType;
}

experimental::Type TypeInference::handleIdentifierByReferencedDeclaration(langutil::SourceLocation _location, Declaration const& _declaration)
{
	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
	{
		if (
			!dynamic_cast<FunctionDefinition const*>(&_declaration) &&
			!dynamic_cast<VariableDeclaration const*>(&_declaration) &&
			!dynamic_cast<TypeClassDefinition const*>(&_declaration) &&
			!dynamic_cast<TypeDefinition const*>(&_declaration)
		)
		{
			SecondarySourceLocation ssl;
			ssl.append("Referenced node.", _declaration.location());
			m_errorReporter.fatalTypeError(3101_error, _location, ssl, "Attempt to type identifier referring to unexpected node.");
		}

		auto& declarationAnnotation = annotation(_declaration);
		if (!declarationAnnotation.type)
			_declaration.accept(*this);

		solAssert(declarationAnnotation.type);

		if (dynamic_cast<VariableDeclaration const*>(&_declaration))
			return *declarationAnnotation.type;
		else if (dynamic_cast<FunctionDefinition const*>(&_declaration))
			return polymorphicInstance(*declarationAnnotation.type);
		else if (dynamic_cast<TypeClassDefinition const*>(&_declaration))
		{
			solAssert(TypeEnvironmentHelpers{*m_env}.typeVars(*declarationAnnotation.type).empty());
			return *declarationAnnotation.type;
		}
		else if (dynamic_cast<TypeDefinition const*>(&_declaration))
		{
			// TODO: can we avoid this?
			Type type = *declarationAnnotation.type;
			if (TypeSystemHelpers{m_typeSystem}.isTypeFunctionType(type))
				type = std::get<1>(TypeSystemHelpers{m_typeSystem}.destTypeFunctionType(type));
			return polymorphicInstance(type);
		}
		else
			solAssert(false);
		break;
	}
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
			ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Term};
			typeClassDefinition->accept(*this);

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

bool TypeInference::visit(Identifier const& _identifier)
{
	auto& identifierAnnotation = annotation(_identifier);
	solAssert(!identifierAnnotation.type);

	if (auto const* referencedDeclaration = _identifier.annotation().referencedDeclaration)
	{
		identifierAnnotation.type = handleIdentifierByReferencedDeclaration(_identifier.location(), *referencedDeclaration);
		return false;
	}

	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		// TODO: error handling
		solAssert(false);
		break;
	case ExpressionContext::Type:
		// TODO: register free type variable name!
		identifierAnnotation.type = m_typeSystem.freshTypeVariable({});
		return false;
	case ExpressionContext::Sort:
		// TODO: error handling
		solAssert(false);
		break;
	}

	return false;
}

void TypeInference::endVisit(TupleExpression const& _tupleExpression)
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

bool TypeInference::visit(IdentifierPath const& _identifierPath)
{
	auto& identifierAnnotation = annotation(_identifierPath);
	solAssert(!identifierAnnotation.type);

	if (auto const* referencedDeclaration = _identifierPath.annotation().referencedDeclaration)
	{
		identifierAnnotation.type = handleIdentifierByReferencedDeclaration(_identifierPath.location(), *referencedDeclaration);
		return false;
	}

	// TODO: error handling
	solAssert(false);
}

bool TypeInference::visit(TypeClassInstantiation const& _typeClassInstantiation)
{
	ScopedSaveAndRestore activeInstantiations{m_activeInstantiations, m_activeInstantiations + std::set<TypeClassInstantiation const*>{&_typeClassInstantiation}};
	// Note: recursion is resolved due to special handling during unification.
	auto& instantiationAnnotation = annotation(_typeClassInstantiation);
	if (instantiationAnnotation.type)
		return false;
	instantiationAnnotation.type = m_voidType;
	std::optional<TypeClass> typeClass = std::visit(util::GenericVisitor{
		[&](ASTPointer<IdentifierPath> _typeClassName) -> std::optional<TypeClass> {
			if (auto const* typeClassDefinition = dynamic_cast<TypeClassDefinition const*>(_typeClassName->annotation().referencedDeclaration))
			{
				// visiting the type class will re-visit this instantiation
				typeClassDefinition->accept(*this);
				// TODO: more error handling? Should be covered by the visit above.
				solAssert(m_analysis.annotation<TypeClassRegistration>(*typeClassDefinition).typeClass.has_value());
				return m_analysis.annotation<TypeClassRegistration>(*typeClassDefinition).typeClass.value();
			}
			else
			{
				m_errorReporter.typeError(9817_error, _typeClassInstantiation.typeClass().location(), "Expected type class.");
				return std::nullopt;
			}
		},
		[&](Token _token) -> std::optional<TypeClass> {
			if (auto builtinClass = builtinClassFromToken(_token))
				if (auto typeClass = util::valueOrNullptr(annotation().builtinClasses, *builtinClass))
					return *typeClass;
			m_errorReporter.typeError(2658_error, _typeClassInstantiation.location(), "Invalid type class name.");
			return std::nullopt;
		}
	}, _typeClassInstantiation.typeClass().name());
	if (!typeClass)
		return false;

	// TODO: _typeClassInstantiation.typeConstructor().accept(*this); ?
	auto typeConstructor = m_analysis.annotation<TypeRegistration>(_typeClassInstantiation.typeConstructor()).typeConstructor;
	if (!typeConstructor)
	{
		m_errorReporter.typeError(2138_error, _typeClassInstantiation.typeConstructor().location(), "Invalid type constructor.");
		return false;
	}

	std::vector<Type> arguments;
	Arity arity{
		{},
		*typeClass
	};

	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		if (_typeClassInstantiation.argumentSorts())
		{
			_typeClassInstantiation.argumentSorts()->accept(*this);
			auto& argumentSortAnnotation = annotation(*_typeClassInstantiation.argumentSorts());
			solAssert(argumentSortAnnotation.type);
			arguments = TypeSystemHelpers{m_typeSystem}.destTupleType(*argumentSortAnnotation.type);
			arity.argumentSorts = arguments | ranges::views::transform([&](Type _type) {
				return m_env->sort(_type);
			}) | ranges::to<std::vector<Sort>>;
		}
	}
	m_env->fixTypeVars(arguments);

	Type type{TypeConstant{*typeConstructor, arguments}};

	std::map<std::string, Type> functionTypes;

	for (auto subNode: _typeClassInstantiation.subNodes())
	{
		auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(subNode.get());
		solAssert(functionDefinition);
		subNode->accept(*this);
		if (!functionTypes.emplace(functionDefinition->name(), typeAnnotation(*functionDefinition)).second)
			m_errorReporter.typeError(3654_error, subNode->location(), "Duplicate definition of function " + functionDefinition->name() + " during type class instantiation.");
	}

	if (auto error = m_typeSystem.instantiateClass(type, arity))
		m_errorReporter.typeError(5094_error, _typeClassInstantiation.location(), *error);

	auto const& classFunctions = annotation().typeClassFunctions.at(*typeClass);

	solAssert(std::holds_alternative<TypeVariable>(m_typeSystem.typeClassVariable(*typeClass)));
	TypeVariable classVar = std::get<TypeVariable>(m_typeSystem.typeClassVariable(*typeClass));

	for (auto [name, classFunctionType]: classFunctions)
	{
		if (!functionTypes.count(name))
		{
			m_errorReporter.typeError(6948_error, _typeClassInstantiation.location(), "Missing function: " + name);
			continue;
		}
		Type instantiatedClassFunctionType = TypeEnvironmentHelpers{*m_env}.substitute(classFunctionType, classVar, type);

		Type instanceFunctionType = functionTypes.at(name);
		functionTypes.erase(name);

		if (!m_env->typeEquals(instanceFunctionType, instantiatedClassFunctionType))
			m_errorReporter.typeError(
				7428_error,
				_typeClassInstantiation.location(),
				fmt::format(
					"Instantiation function '{}' does not match the declaration in the type class ({} != {}).",
					name,
					TypeEnvironmentHelpers{*m_env}.typeToString(instanceFunctionType),
					TypeEnvironmentHelpers{*m_env}.typeToString(instantiatedClassFunctionType)
				)
			);
	}

	if (!functionTypes.empty())
		m_errorReporter.typeError(4873_error, _typeClassInstantiation.location(), "Additional functions in class instantiation.");

	return false;
}

bool TypeInference::visit(MemberAccess const& _memberAccess)
{
	if (m_expressionContext != ExpressionContext::Term)
	{
		m_errorReporter.typeError(5195_error, _memberAccess.location(), "Member access outside term context.");
		annotation(_memberAccess).type = m_typeSystem.freshTypeVariable({});
		return false;
	}
	return true;
}

experimental::Type TypeInference::memberType(Type _type, std::string _memberName, langutil::SourceLocation _location)
{
	Type resolvedType = m_env->resolve(_type);
	TypeSystemHelpers helper{m_typeSystem};
	if (helper.isTypeConstant(resolvedType))
	{
		auto constructor = std::get<0>(helper.destTypeConstant(resolvedType));
		if (auto* typeMember = util::valueOrNullptr(annotation().members.at(constructor), _memberName))
			return polymorphicInstance(typeMember->type);
		else
		{
			m_errorReporter.typeError(5755_error, _location, fmt::format("Member {} not found in type {}.", _memberName, TypeEnvironmentHelpers{*m_env}.typeToString(_type)));
			return m_typeSystem.freshTypeVariable({});
		}
	}
	else
	{
		m_errorReporter.typeError(5104_error, _location, "Unsupported member access expression.");
		return m_typeSystem.freshTypeVariable({});
	}
}

void TypeInference::endVisit(MemberAccess const& _memberAccess)
{
	auto& memberAccessAnnotation = annotation(_memberAccess);
	solAssert(!memberAccessAnnotation.type);
	Type expressionType = typeAnnotation(_memberAccess.expression());
	memberAccessAnnotation.type = memberType(expressionType, _memberAccess.memberName(), _memberAccess.location());
}

bool TypeInference::visit(TypeDefinition const& _typeDefinition)
{
	bool isBuiltIn = dynamic_cast<Builtin const*>(_typeDefinition.typeExpression());

	TypeSystemHelpers helper{m_typeSystem};
	auto& typeDefinitionAnnotation = annotation(_typeDefinition);
	if (typeDefinitionAnnotation.type)
		return false;

	if (_typeDefinition.arguments())
		_typeDefinition.arguments()->accept(*this);

	std::vector<Type> arguments;
	if (_typeDefinition.arguments())
		for (size_t i = 0; i < _typeDefinition.arguments()->parameters().size(); ++i)
			arguments.emplace_back(m_typeSystem.freshTypeVariable({}));

	Type definedType = type(&_typeDefinition, arguments);
	if (arguments.empty())
		typeDefinitionAnnotation.type = definedType;
	else
		typeDefinitionAnnotation.type = helper.typeFunctionType(helper.tupleType(arguments), definedType);

	std::optional<Type> underlyingType;

	if (isBuiltIn)
		// TODO: This special case should eventually become user-definable.
		underlyingType = m_wordType;
	else if (_typeDefinition.typeExpression())
	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_typeDefinition.typeExpression()->accept(*this);

		underlyingType = annotation(*_typeDefinition.typeExpression()).type;
	}

	TypeConstructor constructor = typeConstructor(&_typeDefinition);
	auto [members, newlyInserted] = annotation().members.emplace(constructor, std::map<std::string, TypeMember>{});
	solAssert(newlyInserted, fmt::format("Members of type '{}' are already defined.", m_typeSystem.constructorInfo(constructor).name));
	if (underlyingType)
	{
		members->second.emplace("abs", TypeMember{helper.functionType(*underlyingType, definedType)});
		members->second.emplace("rep", TypeMember{helper.functionType(definedType, *underlyingType)});
	}

	if (helper.isPrimitiveType(definedType, PrimitiveType::Pair))
	{
		solAssert(isBuiltIn);
		solAssert(arguments.size() == 2);
		members->second.emplace("first", TypeMember{helper.functionType(definedType, arguments[0])});
		members->second.emplace("second", TypeMember{helper.functionType(definedType, arguments[1])});
	}

	return false;
}

bool TypeInference::visit(FunctionCall const&) { return true; }
void TypeInference::endVisit(FunctionCall const& _functionCall)
{
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
	{
		Type argTuple = helper.tupleType(argTypes);
		Type resultType = m_typeSystem.freshTypeVariable({});
		Type genericFunctionType = helper.functionType(argTuple, resultType);
		unify(functionType, genericFunctionType, _functionCall.location());
		functionCallAnnotation.type = resultType;
		break;
	}
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

// TODO: clean up rational parsing
namespace
{

std::optional<rational> parseRational(std::string const& _value)
{
	rational value;
	try
	{
		auto radixPoint = find(_value.begin(), _value.end(), '.');

		if (radixPoint != _value.end())
		{
			if (
				!all_of(radixPoint + 1, _value.end(), util::isDigit) ||
				!all_of(_value.begin(), radixPoint, util::isDigit)
			)
				return std::nullopt;

			// Only decimal notation allowed here, leading zeros would switch to octal.
			auto fractionalBegin = find_if_not(
				radixPoint + 1,
				_value.end(),
				[](char const& a) { return a == '0'; }
			);

			rational numerator;
			rational denominator(1);

			denominator = bigint(std::string(fractionalBegin, _value.end()));
			denominator /= boost::multiprecision::pow(
				bigint(10),
				static_cast<unsigned>(distance(radixPoint + 1, _value.end()))
			);
			numerator = bigint(std::string(_value.begin(), radixPoint));
			value = numerator + denominator;
		}
		else
			value = bigint(_value);
		return value;
	}
	catch (...)
	{
		return std::nullopt;
	}
}

/// Checks whether _mantissa * (10 ** _expBase10) fits into 4096 bits.
bool fitsPrecisionBase10(bigint const& _mantissa, uint32_t _expBase10)
{
	double const log2Of10AwayFromZero = 3.3219280948873624;
	return fitsPrecisionBaseX(_mantissa, log2Of10AwayFromZero, _expBase10);
}

std::optional<rational> rationalValue(Literal const& _literal)
{
	rational value;
	try
	{
		ASTString valueString = _literal.valueWithoutUnderscores();

		auto expPoint = find(valueString.begin(), valueString.end(), 'e');
		if (expPoint == valueString.end())
			expPoint = find(valueString.begin(), valueString.end(), 'E');

		if (boost::starts_with(valueString, "0x"))
		{
			// process as hex
			value = bigint(valueString);
		}
		else if (expPoint != valueString.end())
		{
			// Parse mantissa and exponent. Checks numeric limit.
			std::optional<rational> mantissa = parseRational(std::string(valueString.begin(), expPoint));

			if (!mantissa)
				return std::nullopt;
			value = *mantissa;

			// 0E... is always zero.
			if (value == 0)
				return std::nullopt;

			bigint exp = bigint(std::string(expPoint + 1, valueString.end()));

			if (exp > std::numeric_limits<int32_t>::max() || exp < std::numeric_limits<int32_t>::min())
				return std::nullopt;

			uint32_t expAbs = bigint(abs(exp)).convert_to<uint32_t>();

			if (exp < 0)
			{
				if (!fitsPrecisionBase10(abs(value.denominator()), expAbs))
					return std::nullopt;
				value /= boost::multiprecision::pow(
					bigint(10),
					expAbs
				);
			}
			else if (exp > 0)
			{
				if (!fitsPrecisionBase10(abs(value.numerator()), expAbs))
					return std::nullopt;
				value *= boost::multiprecision::pow(
					bigint(10),
					expAbs
				);
			}
		}
		else
		{
			// parse as rational number
			std::optional<rational> tmp = parseRational(valueString);
			if (!tmp)
				return std::nullopt;
			value = *tmp;
		}
	}
	catch (...)
	{
		return std::nullopt;
	}
	switch (_literal.subDenomination())
	{
	case Literal::SubDenomination::None:
	case Literal::SubDenomination::Wei:
	case Literal::SubDenomination::Second:
		break;
	case Literal::SubDenomination::Gwei:
		value *= bigint("1000000000");
		break;
	case Literal::SubDenomination::Ether:
		value *= bigint("1000000000000000000");
		break;
	case Literal::SubDenomination::Minute:
		value *= bigint("60");
		break;
	case Literal::SubDenomination::Hour:
		value *= bigint("3600");
		break;
	case Literal::SubDenomination::Day:
		value *= bigint("86400");
		break;
	case Literal::SubDenomination::Week:
		value *= bigint("604800");
		break;
	case Literal::SubDenomination::Year:
		value *= bigint("31536000");
		break;
	}

	return value;
}
}

bool TypeInference::visit(Literal const& _literal)
{
	auto& literalAnnotation = annotation(_literal);
	if (_literal.token() != Token::Number)
	{
		m_errorReporter.typeError(4316_error, _literal.location(), "Only number literals are supported.");
		return false;
	}
	std::optional<rational> value = rationalValue(_literal);
	if (!value)
	{
		m_errorReporter.typeError(6739_error, _literal.location(), "Invalid number literals.");
		return false;
	}
	if (value->denominator() != 1)
	{
		m_errorReporter.typeError(2345_error, _literal.location(), "Only integers are supported.");
		return false;
	}
	literalAnnotation.type = m_typeSystem.freshTypeVariable(Sort{{annotation().builtinClasses.at(BuiltinClass::Integer)}});
	return false;
}


namespace
{
// TODO: put at a nice place to deduplicate.
TypeRegistration::TypeClassInstantiations const& typeClassInstantiations(Analysis const& _analysis, TypeClass _class)
{
	auto const* typeClassDeclaration = _analysis.typeSystem().typeClassDeclaration(_class);
	if (typeClassDeclaration)
		return _analysis.annotation<TypeRegistration>(*typeClassDeclaration).instantiations;
	// TODO: better mechanism than fetching by name.
	auto const& annotation = _analysis.annotation<TypeRegistration>();
	auto const& inferenceAnnotation = _analysis.annotation<TypeInference>();
	return annotation.builtinClassInstantiations.at(
		inferenceAnnotation.builtinClassesByName.at(
			_analysis.typeSystem().typeClassName(_class)
		)
	);
}
}

experimental::Type TypeInference::polymorphicInstance(Type const& _scheme)
{
	return m_env->fresh(_scheme);
}

void TypeInference::unify(Type _a, Type _b, langutil::SourceLocation _location)
{
	TypeSystemHelpers helper{m_typeSystem};
	auto unificationFailures = m_env->unify(_a, _b);

	if (!m_activeInstantiations.empty())
	{
		// TODO: This entire logic is superfluous - I thought mutually recursive dependencies between
		// class instantiations are a problem, but in fact they're not, they just resolve to mutually recursive
		// functions that are fine. So instead, all instantiations can be registered with the type system directly
		// when visiting the type class (assuming that they all work out) - and then all instantiations can be checked
		// individually, which should still catch all actual issues (while allowing recursions).
		// Original comment: Attempt to resolve interdependencies between type class instantiations.
		std::vector<TypeClassInstantiation const*> missingInstantiations;
		bool recursion = false;
		bool onlyMissingInstantiations = [&]() {
			for (auto failure: unificationFailures)
			{
				if (auto* sortMismatch = std::get_if<TypeEnvironment::SortMismatch>(&failure))
					if (helper.isTypeConstant(sortMismatch->type))
					{
						TypeConstructor constructor = std::get<0>(helper.destTypeConstant(sortMismatch->type));
						for (auto typeClass: sortMismatch->sort.classes)
						{
							if (auto const* instantiation = util::valueOrDefault(typeClassInstantiations(m_analysis, typeClass), constructor, nullptr))
							{
								if (m_activeInstantiations.count(instantiation))
								{
									langutil::SecondarySourceLocation ssl;
									for (auto activeInstantiation: m_activeInstantiations)
										ssl.append("Involved instantiation", activeInstantiation->location());
									m_errorReporter.typeError(
										3573_error,
										_location,
										ssl,
										"Recursion during type class instantiation."
									);
									recursion = true;
									return false;
								}
								missingInstantiations.emplace_back(instantiation);
							}
							else
								return false;
						}
						continue;
					}
				return false;
			}
			return true;
		}();

		if (recursion)
			return;

		if (onlyMissingInstantiations)
		{
			for (auto instantiation: missingInstantiations)
				instantiation->accept(*this);
			unificationFailures = m_env->unify(_a, _b);
		}
	}

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

experimental::Type TypeInference::typeAnnotation(ASTNode const& _node) const
{
	auto result = annotation(_node).type;
	solAssert(result);
	return *result;
}
TypeConstructor TypeInference::typeConstructor(Declaration const* _type) const
{
	if (auto const& constructor = m_analysis.annotation<TypeRegistration>(*_type).typeConstructor)
		return *constructor;
	m_errorReporter.fatalTypeError(5904_error, _type->location(), "Unregistered type.");
	util::unreachable();
}
experimental::Type TypeInference::type(Declaration const* _type, std::vector<Type> _arguments) const
{
	return m_typeSystem.type(typeConstructor(_type), std::move(_arguments));
}

bool TypeInference::visitNode(ASTNode const& _node)
{
	m_errorReporter.fatalTypeError(5348_error, _node.location(), "Unsupported AST node during type inference.");
	return false;
}

TypeInference::Annotation& TypeInference::annotation(ASTNode const& _node)
{
	return m_analysis.annotation<TypeInference>(_node);
}

TypeInference::Annotation const& TypeInference::annotation(ASTNode const& _node) const
{
	return m_analysis.annotation<TypeInference>(_node);
}

TypeInference::GlobalAnnotation& TypeInference::annotation()
{
	return m_analysis.annotation<TypeInference>();
}
