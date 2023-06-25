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


#include <libsolidity/analysis/experimental/TypeInference.h>
#include <libsolidity/analysis/experimental/TypeRegistration.h>
#include <libsolidity/analysis/experimental/Analysis.h>
#include <libsolidity/ast/experimental/TypeSystemHelper.h>

#include <libsolutil/Numeric.h>
#include <libsolutil/StringUtils.h>
#include <liblangutil/Exceptions.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

#include <boost/algorithm/string.hpp>
#include <range/v3/view/transform.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

TypeInference::TypeInference(Analysis& _analysis):
m_analysis(_analysis),
m_errorReporter(_analysis.errorReporter()),
m_typeSystem(_analysis.typeSystem())
{
	m_voidType = m_typeSystem.type(BuiltinType::Void, {});
	m_wordType = m_typeSystem.type(BuiltinType::Word, {});
	m_integerType = m_typeSystem.type(BuiltinType::Integer, {});
	m_unitType = m_typeSystem.type(BuiltinType::Unit, {});
	m_boolType = m_typeSystem.type(BuiltinType::Bool, {});
	m_env = &m_typeSystem.env();
}

bool TypeInference::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool TypeInference::visit(FunctionDefinition const& _functionDefinition)
{
	solAssert(m_expressionContext == ExpressionContext::Term);
	ScopedSaveAndRestore signatureRestore(m_currentFunctionType, nullopt);
	auto& functionAnnotation = annotation(_functionDefinition);
	if (functionAnnotation.type)
		return false;

	_functionDefinition.parameterList().accept(*this);
	if (_functionDefinition.returnParameterList())
		_functionDefinition.returnParameterList()->accept(*this);

	auto typeFromParameterList = [&](ParameterList const* _list) {
		if (!_list)
			return m_unitType;
		auto& listAnnotation = annotation(*_list);
		solAssert(listAnnotation.type);
		return *listAnnotation.type;
	};

	Type functionType = TypeSystemHelpers{m_typeSystem}.functionType(
		typeFromParameterList(&_functionDefinition.parameterList()),
		typeFromParameterList(_functionDefinition.returnParameterList().get())
	);

	m_currentFunctionType = functionType;

	if (_functionDefinition.isImplemented())
		_functionDefinition.body().accept(*this);

	functionAnnotation.type = functionType;

	m_errorReporter.info(0000_error, _functionDefinition.location(), m_env->typeToString(*functionAnnotation.type));

	return false;
}

void TypeInference::endVisit(Return const& _return)
{
	solAssert(m_currentFunctionType);
	if (_return.expression())
	{
		auto& returnExpressionAnnotation = annotation(*_return.expression());
		solAssert(returnExpressionAnnotation.type);
		Type functionReturnType = get<1>(TypeSystemHelpers{m_typeSystem}.destFunctionType(*m_currentFunctionType));
		unify(functionReturnType, *returnExpressionAnnotation.type, _return.location());
	}
}

bool TypeInference::visit(ParameterList const&)
{
	return true;
}

void TypeInference::endVisit(ParameterList const& _parameterList)
{
	auto& listAnnotation = annotation(_parameterList);
	solAssert(!listAnnotation.type);
	std::vector<Type> argTypes;
	for(auto arg: _parameterList.parameters())
	{
		auto& argAnnotation = annotation(*arg);
		solAssert(argAnnotation.type);
		argTypes.emplace_back(*argAnnotation.type);
	}
	listAnnotation.type = TypeSystemHelpers{m_typeSystem}.tupleType(argTypes);
}

bool TypeInference::visitNode(ASTNode const& _node)
{
	m_errorReporter.fatalTypeError(0000_error, _node.location(), "Unsupported AST node during type inference.");
	return false;
}

bool TypeInference::visit(TypeClassDefinition const& _typeClassDefinition)
{
	solAssert(m_expressionContext == ExpressionContext::Term);
	auto& typeClassAnnotation = annotation(_typeClassDefinition);
	if (typeClassAnnotation.type)
		return false;
	m_typeSystem.declareTypeConstructor(&_typeClassDefinition, _typeClassDefinition.name(), 0);
	typeClassAnnotation.type = TypeConstant{&_typeClassDefinition, {}};
	auto& typeVariableAnnotation = annotation(_typeClassDefinition.typeVariable());
	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_typeClassDefinition.typeVariable().accept(*this);
	}
	solAssert(typeVariableAnnotation.type);

	map<string, Type> functionTypes;

	Type typeVar = m_typeSystem.freshTypeVariable(false, {});

	auto& typeMembers = annotation().members[&_typeClassDefinition];

	for (auto subNode: _typeClassDefinition.subNodes())
	{
		subNode->accept(*this);
		auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(subNode.get());
		solAssert(functionDefinition);
		auto functionDefinitionType = annotation(*functionDefinition).type;
		solAssert(functionDefinitionType);
		auto functionType = m_env->fresh(*functionDefinitionType, true);
		functionTypes[functionDefinition->name()] = functionType;
		auto typeVars = TypeSystemHelpers{m_typeSystem}.typeVars(functionType);
		if(typeVars.size() != 1)
			m_errorReporter.fatalTypeError(0000_error, functionDefinition->location(), "Function in type class may only depend on the type class variable.");
		unify(typeVars.front(), typeVar);

		if (!typeMembers.emplace(functionDefinition->name(), TypeMember{functionType}).second)
			m_errorReporter.fatalTypeError(0000_error, functionDefinition->location(), "Function in type class declared multiple times.");
	}

	if (auto error = m_typeSystem.declareTypeClass(TypeClass{&_typeClassDefinition}, typeVar, std::move(functionTypes)))
		m_errorReporter.fatalTypeError(0000_error, _typeClassDefinition.location(), *error);

	solAssert(typeVariableAnnotation.type);
	TypeSystemHelpers helper{m_typeSystem};
	unify(*typeVariableAnnotation.type, helper.kindType(m_typeSystem.freshTypeVariable(false, Sort{{TypeClass{&_typeClassDefinition}}})), _typeClassDefinition.location());

	for (auto instantiation: m_analysis.annotation<TypeRegistration>(_typeClassDefinition).instantiations | ranges::views::values)
		// TODO: recursion-safety?
		instantiation->accept(*this);

	return false;
}

void TypeInference::unify(Type _a, Type _b, langutil::SourceLocation _location, TypeEnvironment* _env)
{
	if (!_env)
		_env = m_env;
	for (auto failure: _env->unify(_a, _b))
		std::visit(util::GenericVisitor{
			[&](TypeEnvironment::TypeMismatch _typeMismatch) {
				m_errorReporter.typeError(0000_error, _location, fmt::format("Cannot unify {} and {}.", _env->typeToString(_typeMismatch.a), _env->typeToString(_typeMismatch.b)));
			},
			[&](TypeEnvironment::SortMismatch _sortMismatch) {
				m_errorReporter.typeError(0000_error, _location, fmt::format(
					"{} does not have sort {}",
					_env->typeToString(_sortMismatch.type),
					TypeSystemHelpers{m_typeSystem}.sortToString(_sortMismatch.sort)
				));
			}
		}, failure);

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
			// Hack until we can disallow any shadowing: If we found an internal reference,
			// clear the external references, so that codegen does not use it.
			_inlineAssembly.annotation().externalReferences.erase(& _identifier);
			return false;
		}
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		if (ref == _inlineAssembly.annotation().externalReferences.end())
			return false;
		InlineAssemblyAnnotation::ExternalIdentifierInfo& identifierInfo = ref->second;
		Declaration const* declaration = identifierInfo.declaration;
		solAssert(!!declaration, "");

		solAssert(identifierInfo.suffix == "", "");

		auto& declarationAnnotation = annotation(*declaration);
		solAssert(declarationAnnotation.type);
		unify(*declarationAnnotation.type, m_wordType, originLocationOf(_identifier));
		identifierInfo.valueSize = 1;
		return true;
	};
	solAssert(!_inlineAssembly.annotation().analysisInfo, "");
	_inlineAssembly.annotation().analysisInfo = make_shared<yul::AsmAnalysisInfo>();
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

bool TypeInference::visit(ElementaryTypeNameExpression const& _expression)
{
	auto& expressionAnnotation = annotation(_expression);
	if (m_expressionContext != ExpressionContext::Type)
	{
		m_errorReporter.typeError(0000_error, _expression.location(), "Elementary type name expression only supported in type context.");
		expressionAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
		return false;
	}
	TypeSystemHelpers helper{m_typeSystem};
	switch(_expression.type().typeName().token())
	{
	case Token::Word:
		expressionAnnotation.type = helper.kindType(m_wordType);
		break;
	case Token::Void:
		expressionAnnotation.type = helper.kindType(m_voidType);
		break;
	case Token::Integer:
		expressionAnnotation.type = helper.kindType(m_integerType);
		break;
	case Token::Unit:
		expressionAnnotation.type = helper.kindType(m_unitType);
		break;
	case Token::Bool:
		expressionAnnotation.type = helper.kindType(m_boolType);
		break;
	case Token::Pair:
	{
		auto leftType = m_typeSystem.freshTypeVariable(false, {});
		auto rightType = m_typeSystem.freshTypeVariable(false, {});
		expressionAnnotation.type =
			helper.functionType(
			helper.kindType(helper.tupleType({leftType, rightType})),
			helper.kindType(m_typeSystem.type(BuiltinType::Pair, {leftType, rightType}))
			);
		break;
	}
	case Token::Fun:
	{
		auto argType = m_typeSystem.freshTypeVariable(false, {});
		auto resultType = m_typeSystem.freshTypeVariable(false, {});
		expressionAnnotation.type =
			helper.functionType(
				helper.kindType(helper.tupleType({argType, resultType})),
				helper.kindType(m_typeSystem.type(BuiltinType::Function, {argType, resultType}))
			);
		break;
	}
	default:
		m_errorReporter.typeError(0000_error, _expression.location(), "Only elementary types are supported.");
		expressionAnnotation.type = helper.kindType(m_typeSystem.freshTypeVariable(false, {}));
		break;
	}
	return false;
}

bool TypeInference::visit(BinaryOperation const& _binaryOperation)
{
	auto& operationAnnotation = annotation(_binaryOperation);
	auto& leftAnnotation = annotation(_binaryOperation.leftExpression());
	auto& rightAnnotation = annotation(_binaryOperation.rightExpression());
	TypeSystemHelpers helper{m_typeSystem};
	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		if (auto* operatorInfo = util::valueOrNullptr(m_analysis.annotation<TypeRegistration>().operators, _binaryOperation.getOperator()))
		{
			auto [typeClass, functionName] = *operatorInfo;
			Type functionType = m_env->fresh(m_typeSystem.typeClassInfo(typeClass)->functions.at(functionName), true);

			_binaryOperation.leftExpression().accept(*this);
			solAssert(leftAnnotation.type);
			_binaryOperation.rightExpression().accept(*this);
			solAssert(rightAnnotation.type);

			Type argTuple = helper.tupleType({*leftAnnotation.type, *rightAnnotation.type});
			Type genericFunctionType = helper.functionType(argTuple, m_typeSystem.freshTypeVariable(false, {}));
			unify(functionType, genericFunctionType, _binaryOperation.location());

			operationAnnotation.type = m_env->resolve(std::get<1>(helper.destFunctionType(m_env->resolve(genericFunctionType))));

			return false;
		}
		else
		{
			m_errorReporter.typeError(0000_error, _binaryOperation.location(), "Binary operations in term context not yet supported.");
			operationAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
			return false;
		}
	case ExpressionContext::Type:
		if (_binaryOperation.getOperator() == Token::Colon)
		{
			_binaryOperation.leftExpression().accept(*this);
			{
				ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Sort};
				_binaryOperation.rightExpression().accept(*this);
			}
			solAssert(leftAnnotation.type);
			solAssert(rightAnnotation.type);
			unify(*leftAnnotation.type, *rightAnnotation.type, _binaryOperation.location());
			operationAnnotation.type = leftAnnotation.type;
		}
		else if (_binaryOperation.getOperator() == Token::RightArrow)
		{
			_binaryOperation.leftExpression().accept(*this);
			_binaryOperation.rightExpression().accept(*this);
			auto getType = [&](std::optional<Type> _type) -> Type {
				solAssert(_type);
				if (helper.isKindType(*_type))
					return helper.destKindType(*_type);
				else
				{
					m_errorReporter.typeError(0000_error, _binaryOperation.leftExpression().location(), "Expected type but got " + m_env->typeToString(*leftAnnotation.type));
					return m_typeSystem.freshTypeVariable(false, {});
				}
			};
			Type leftType = getType(leftAnnotation.type);
			Type rightType = getType(rightAnnotation.type);
			operationAnnotation.type = helper.kindType(helper.functionType(leftType, rightType));
		}
		else
		{
			m_errorReporter.typeError(0000_error, _binaryOperation.location(), "Invalid binary operations in type context.");
			operationAnnotation.type = helper.kindType(m_typeSystem.freshTypeVariable(false, {}));
		}
		return false;
	case ExpressionContext::Sort:
		m_errorReporter.typeError(0000_error, _binaryOperation.location(), "Invalid binary operation in sort context.");
		operationAnnotation.type = helper.kindType(m_typeSystem.freshTypeVariable(false, {}));
		return false;
	}
	return false;
}

void TypeInference::endVisit(VariableDeclarationStatement const& _variableDeclarationStatement)
{
	solAssert(m_expressionContext == ExpressionContext::Term);
	if (_variableDeclarationStatement.declarations().size () != 1)
	{
		m_errorReporter.typeError(0000_error, _variableDeclarationStatement.location(), "Multi variable declaration not supported.");
		return;
	}
	auto& variableAnnotation = annotation(*_variableDeclarationStatement.declarations().front());
	solAssert(variableAnnotation.type);
	if (_variableDeclarationStatement.initialValue())
	{
		auto& expressionAnnotation = annotation(*_variableDeclarationStatement.initialValue());
		solAssert(expressionAnnotation.type);
		unify(*variableAnnotation.type, *expressionAnnotation.type, _variableDeclarationStatement.location());
	}
}

bool TypeInference::visit(VariableDeclaration const& _variableDeclaration)
{
	solAssert(!_variableDeclaration.value());
	auto& variableAnnotation = annotation(_variableDeclaration);
	solAssert(!variableAnnotation.type);

	TypeSystemHelpers helper{m_typeSystem};
	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		if (_variableDeclaration.typeExpression())
		{
			ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
			_variableDeclaration.typeExpression()->accept(*this);
			auto& typeExpressionAnnotation = annotation(*_variableDeclaration.typeExpression());
			solAssert(typeExpressionAnnotation.type);
			if (helper.isKindType(*typeExpressionAnnotation.type))
				variableAnnotation.type = m_env->fresh(helper.destKindType(*typeExpressionAnnotation.type), false);
			else
			{
				m_errorReporter.typeError(0000_error, _variableDeclaration.typeExpression()->location(), "Expected type, but got " + m_env->typeToString(*typeExpressionAnnotation.type));
				variableAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
			}
			return false;
		}

		variableAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
		return false;
	case ExpressionContext::Type:
		variableAnnotation.type = helper.kindType(m_typeSystem.freshTypeVariable(false, {}));
		if (_variableDeclaration.typeExpression())
		{
			ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Sort};
			_variableDeclaration.typeExpression()->accept(*this);
			auto& typeExpressionAnnotation = annotation(*_variableDeclaration.typeExpression());
			solAssert(typeExpressionAnnotation.type);

			unify(*variableAnnotation.type, *typeExpressionAnnotation.type, _variableDeclaration.typeExpression()->location());
		}
		return false;
	case ExpressionContext::Sort:
		m_errorReporter.typeError(0000_error, _variableDeclaration.location(), "Variable declaration in sort context.");
		return false;
	}
	solAssert(false);
	return false;
}

bool TypeInference::visit(Assignment const&)
{
	return true;
}

void TypeInference::endVisit(Assignment const& _assignment)
{
	auto& assignmentAnnotation = annotation(_assignment);
	solAssert(!assignmentAnnotation.type);

	if (m_expressionContext != ExpressionContext::Term)
	{
		m_errorReporter.typeError(0000_error, _assignment.location(), "Assignment outside term context.");
		assignmentAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
		return;
	}

	auto& lhsAnnotation = annotation(_assignment.leftHandSide());
	solAssert(lhsAnnotation.type);
	auto& rhsAnnotation = annotation(_assignment.rightHandSide());
	solAssert(rhsAnnotation.type);
	unify(*lhsAnnotation.type, *rhsAnnotation.type, _assignment.location());
	assignmentAnnotation.type = m_env->resolve(*lhsAnnotation.type);
}

TypeInference::Annotation& TypeInference::annotation(ASTNode const& _node)
{
	return m_analysis.annotation<TypeInference>(_node);
}

TypeInference::GlobalAnnotation& TypeInference::annotation()
{
	return m_analysis.annotation<TypeInference>();
}

experimental::Type TypeInference::handleIdentifierByReferencedDeclaration(langutil::SourceLocation _location, Declaration const& _declaration)
{
	TypeSystemHelpers helper{m_typeSystem};
	switch(m_expressionContext)
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
			m_errorReporter.fatalTypeError(0000_error, _location, ssl, "Attempt to type identifier referring to unexpected node.");
		}

		auto& declarationAnnotation = annotation(_declaration);
		if (!declarationAnnotation.type)
			_declaration.accept(*this);

		solAssert(declarationAnnotation.type);

		if (dynamic_cast<FunctionDefinition const*>(&_declaration))
			return m_env->fresh(*declarationAnnotation.type, true);
		else if (dynamic_cast<VariableDeclaration const*>(&_declaration))
			return *declarationAnnotation.type;
		else if (dynamic_cast<TypeClassDefinition const*>(&_declaration))
			return *declarationAnnotation.type;
		else if (dynamic_cast<TypeDefinition const*>(&_declaration))
			return *declarationAnnotation.type;
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
			m_errorReporter.fatalTypeError(0000_error, _location, ssl, "Attempt to type identifier referring to unexpected node.");
		}

		// TODO: Assert that this is a type class variable declaration.
		auto& declarationAnnotation = annotation(_declaration);
		if (!declarationAnnotation.type)
			_declaration.accept(*this);

		solAssert(declarationAnnotation.type);

		if (dynamic_cast<VariableDeclaration const*>(&_declaration))
		{
			// TODO: helper.destKindType(*declarationAnnotation.type);
			return *declarationAnnotation.type;
		}
		else if (dynamic_cast<TypeDefinition const*>(&_declaration))
		{
			// TODO: helper.destKindType(*declarationAnnotation.type);
			return m_env->fresh(*declarationAnnotation.type, true);
		}
		else
			solAssert(false);
		break;
	}
	case ExpressionContext::Sort:
	{
		if (auto const* typeClass = dynamic_cast<TypeClassDefinition const*>(&_declaration))
		{
			ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Term};
			typeClass->accept(*this);
			return helper.kindType(m_typeSystem.freshTypeVariable(false, Sort{{TypeClass{typeClass}}}));
		}
		else
		{
			m_errorReporter.typeError(0000_error, _location, "Expected type class.");
			return helper.kindType(m_typeSystem.freshTypeVariable(false, {}));
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

	TypeSystemHelpers helper{m_typeSystem};
	switch(m_expressionContext)
	{
	case ExpressionContext::Term:
		// TODO: error handling
		solAssert(false);
		break;
	case ExpressionContext::Type:
	{
		// TODO: register free type variable name!
		identifierAnnotation.type = helper.kindType(m_typeSystem.freshTypeVariable(false, {}));
		return false;
	}
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
		switch(m_expressionContext)
		{
		case ExpressionContext::Term:
			return *componentAnnotation.type;
		case ExpressionContext::Type:
			if (helper.isKindType(*componentAnnotation.type))
				return helper.destKindType(*componentAnnotation.type);
			else
			{
				m_errorReporter.typeError(0000_error, _expr->location(), "Expected type, but got " + m_env->typeToString(*componentAnnotation.type));
				return m_typeSystem.freshTypeVariable(false, {});
			}
		case ExpressionContext::Sort:
			return *componentAnnotation.type;
		}
		solAssert(false);
	}) | ranges::to<vector<Type>>;
	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		expressionAnnotation.type = helper.tupleType(componentTypes);
		break;
	case ExpressionContext::Type:
		expressionAnnotation.type = helper.kindType(helper.tupleType(componentTypes));
		break;
	case ExpressionContext::Sort:
	{
		Type type = helper.kindType(m_typeSystem.freshTypeVariable(false, {}));
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
	auto& instantiationAnnotation = annotation(_typeClassInstantiation);
	if (instantiationAnnotation.type)
		return false;
	instantiationAnnotation.type = m_voidType;
	std::optional<TypeClass> typeClass = std::visit(util::GenericVisitor{
		[&](ASTPointer<IdentifierPath> _typeClassName) -> std::optional<TypeClass> {
			if (auto const* typeClass = dynamic_cast<TypeClassDefinition const*>(_typeClassName->annotation().referencedDeclaration))
			{
				auto const* typeClassInfo = m_typeSystem.typeClassInfo(TypeClass{typeClass});
				if (!typeClassInfo)
				{
					// visiting the type class will re-visit this instantiation
					typeClass->accept(*this);
				}
				return TypeClass{typeClass};
			}
			else
			{
				m_errorReporter.typeError(0000_error, _typeClassInstantiation.typeClass().location(), "Expected type class.");
				return nullopt;
			}
		},
		[&](Token _token) -> std::optional<TypeClass> {
			if (auto typeClass = typeClassFromToken(_token))
				return typeClass;
			else
			{
				m_errorReporter.typeError(0000_error, _typeClassInstantiation.location(), "Invalid type class name.");
				return nullopt;
			}
		}
	}, _typeClassInstantiation.typeClass().name());
	if (!typeClass)
		return false;

	auto typeConstructor = typeConstructorFromTypeName(_typeClassInstantiation.typeConstructor());
	if (!typeConstructor)
	{
		m_errorReporter.typeError(0000_error, _typeClassInstantiation.typeConstructor().location(), "Invalid type constructor.");
		return false;
	}

	vector<Type> arguments;
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
			for (auto& argument: arguments)
				argument = TypeSystemHelpers{m_typeSystem}.destKindType(argument);
			arity.argumentSorts = arguments | ranges::views::transform([&](Type _type) {
				return m_env->sort(_type);
			}) | ranges::to<vector<Sort>>;
		}
	}

	Type type{TypeConstant{*typeConstructor, arguments}};

	map<string, Type> functionTypes;

	for (auto subNode: _typeClassInstantiation.subNodes())
	{
		auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(subNode.get());
		solAssert(functionDefinition);
		subNode->accept(*this);
		solAssert(annotation(*functionDefinition).type);
		functionTypes[functionDefinition->name()] = *annotation(*functionDefinition).type;
	}

	if (auto error = m_typeSystem.instantiateClass(type, arity, std::move(functionTypes)))
		m_errorReporter.typeError(0000_error, _typeClassInstantiation.location(), *error);

	return false;
}

void TypeInference::endVisit(MemberAccess const& _memberAccess)
{
	auto &memberAccessAnnotation = annotation(_memberAccess);
	solAssert(!memberAccessAnnotation.type);
	auto& expressionAnnotation = annotation(_memberAccess.expression());
	solAssert(expressionAnnotation.type);
	TypeSystemHelpers helper{m_typeSystem};
	if (helper.isTypeConstant(*expressionAnnotation.type))
	{
		Type expressionType = *expressionAnnotation.type;
		// TODO: unify this, s.t. this is always or never a kind type.
		if (helper.isKindType(expressionType))
			expressionType = helper.destKindType(expressionType);
		auto constructor = std::get<0>(helper.destTypeConstant(expressionType));
		if (auto* typeMember = util::valueOrNullptr(annotation().members.at(constructor), _memberAccess.memberName()))
		{
			Type type = m_env->fresh(typeMember->type, true);
			annotation(_memberAccess).type = type;
			return;
		}
		else
		{
			m_errorReporter.typeError(0000_error, _memberAccess.memberLocation(), "Member not found.");
			annotation(_memberAccess).type = m_typeSystem.freshTypeVariable(false, {});
			return;
		}
	}
	m_errorReporter.typeError(0000_error, _memberAccess.expression().location(), "Unsupported member access expression.");
	annotation(_memberAccess).type = m_typeSystem.freshTypeVariable(false, {});
}

bool TypeInference::visit(MemberAccess const& _memberAccess)
{
	if (m_expressionContext != ExpressionContext::Term)
	{
		m_errorReporter.typeError(0000_error, _memberAccess.location(), "Member access outside term context.");
		annotation(_memberAccess).type = m_typeSystem.freshTypeVariable(false, {});
		return false;
	}
	return true;
}

bool TypeInference::visit(TypeDefinition const& _typeDefinition)
{
	TypeSystemHelpers helper{m_typeSystem};
	auto& typeDefinitionAnnotation = annotation(_typeDefinition);
	if (typeDefinitionAnnotation.type)
		 return false;

	std::optional<Type> underlyingType;
	if (_typeDefinition.typeExpression())
	{
		 ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		 _typeDefinition.typeExpression()->accept(*this);
		 underlyingType = annotation(*_typeDefinition.typeExpression()).type;
		 // TODO: settle the kind type mess.
		 if (underlyingType && helper.isKindType(*underlyingType))
			underlyingType = helper.destKindType(*underlyingType);
	}

	vector<Type> arguments;
	if (_typeDefinition.arguments())
		 for (size_t i = 0; i < _typeDefinition.arguments()->parameters().size(); ++i)
			arguments.emplace_back(m_typeSystem.freshTypeVariable(true, {}));

	Type type = m_typeSystem.type(TypeConstructor{&_typeDefinition}, arguments);
	if (arguments.empty())
		 typeDefinitionAnnotation.type = helper.kindType(m_typeSystem.type(TypeConstructor{&_typeDefinition}, arguments));
	else
		typeDefinitionAnnotation.type =
			 helper.functionType(
				 helper.kindType(helper.tupleType(arguments)),
				helper.kindType(type)
			);

	auto& typeMembers = annotation().members[&_typeDefinition];

	if (underlyingType)
	{
		typeMembers.emplace("abs", TypeMember{helper.functionType(*underlyingType, type)});
		typeMembers.emplace("rep", TypeMember{helper.functionType(type, *underlyingType)});
	}

	return false;
}

bool TypeInference::visit(FunctionCall const&) { return true; }
void TypeInference::endVisit(FunctionCall const& _functionCall)
{
	auto& functionCallAnnotation = annotation(_functionCall);
	solAssert(!functionCallAnnotation.type);

	auto& expressionAnnotation = annotation(_functionCall.expression());
	solAssert(expressionAnnotation.type);

	Type functionType = *expressionAnnotation.type;

	TypeSystemHelpers helper{m_typeSystem};
	std::vector<Type> argTypes;
	for(auto arg: _functionCall.arguments())
	{
		 auto& argAnnotation = annotation(*arg);
		 solAssert(argAnnotation.type);
		 switch(m_expressionContext)
		 {
		 case ExpressionContext::Term:
			argTypes.emplace_back(*argAnnotation.type);
			break;
		 case ExpressionContext::Type:
			if (helper.isKindType(*argAnnotation.type))
				argTypes.emplace_back(helper.destKindType(*argAnnotation.type));
			else
			{
				m_errorReporter.typeError(0000_error, arg->location(), "Expected type, but got " + m_env->typeToString(*argAnnotation.type));
				argTypes.emplace_back(m_typeSystem.freshTypeVariable(false, {}));
			}
			break;
		 case ExpressionContext::Sort:
			m_errorReporter.typeError(0000_error, _functionCall.location(), "Function call in sort context.");
			functionCallAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
			break;
		 }
	}

	switch(m_expressionContext)
	{
	case ExpressionContext::Term:
	{
		 Type argTuple = helper.tupleType(argTypes);
		 Type genericFunctionType = helper.functionType(argTuple, m_typeSystem.freshTypeVariable(false, {}));
		 unify(functionType, genericFunctionType, _functionCall.location());

		 functionCallAnnotation.type = m_env->resolve(std::get<1>(helper.destFunctionType(m_env->resolve(genericFunctionType))));
		 break;
	}
	case ExpressionContext::Type:
	{
		 Type argTuple = helper.kindType(helper.tupleType(argTypes));
		 Type genericFunctionType = helper.functionType(argTuple, m_typeSystem.freshKindVariable(false, {}));
		 unify(functionType, genericFunctionType, _functionCall.location());

		 functionCallAnnotation.type = m_env->resolve(std::get<1>(helper.destFunctionType(m_env->resolve(genericFunctionType))));
		 break;
	}
	case ExpressionContext::Sort:
		 solAssert(false);
	}
}

// TODO: clean up rational parsing
namespace
{

optional<rational> parseRational(string const& _value)
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
				return nullopt;

			// Only decimal notation allowed here, leading zeros would switch to octal.
			auto fractionalBegin = find_if_not(
				radixPoint + 1,
				_value.end(),
				[](char const& a) { return a == '0'; }
			);

			rational numerator;
			rational denominator(1);

			denominator = bigint(string(fractionalBegin, _value.end()));
			denominator /= boost::multiprecision::pow(
				bigint(10),
				static_cast<unsigned>(distance(radixPoint + 1, _value.end()))
			);
			numerator = bigint(string(_value.begin(), radixPoint));
			value = numerator + denominator;
		 }
		 else
			value = bigint(_value);
		 return value;
	}
	catch (...)
	{
		 return nullopt;
	}
}

/// Checks whether _mantissa * (10 ** _expBase10) fits into 4096 bits.
bool fitsPrecisionBase10(bigint const& _mantissa, uint32_t _expBase10)
{
	double const log2Of10AwayFromZero = 3.3219280948873624;
	return fitsPrecisionBaseX(_mantissa, log2Of10AwayFromZero, _expBase10);
}

optional<rational> rationalValue(Literal const& _literal)
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
			optional<rational> mantissa = parseRational(string(valueString.begin(), expPoint));

			if (!mantissa)
				return nullopt;
			value = *mantissa;

			// 0E... is always zero.
			if (value == 0)
				return nullopt;

			bigint exp = bigint(string(expPoint + 1, valueString.end()));

			if (exp > numeric_limits<int32_t>::max() || exp < numeric_limits<int32_t>::min())
				return nullopt;

			uint32_t expAbs = bigint(abs(exp)).convert_to<uint32_t>();

			if (exp < 0)
			{
				if (!fitsPrecisionBase10(abs(value.denominator()), expAbs))
					return nullopt;
				value /= boost::multiprecision::pow(
					bigint(10),
					expAbs
				);
			}
			else if (exp > 0)
			{
				if (!fitsPrecisionBase10(abs(value.numerator()), expAbs))
					return nullopt;
				value *= boost::multiprecision::pow(
					bigint(10),
					expAbs
				);
			}
		 }
		 else
		 {
			// parse as rational number
			optional<rational> tmp = parseRational(valueString);
			if (!tmp)
				return nullopt;
			value = *tmp;
		 }
	}
	catch (...)
	{
		 return nullopt;
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
		 m_errorReporter.typeError(0000_error, _literal.location(), "Only number literals are supported.");
		 return false;
	}
	optional<rational> value = rationalValue(_literal);
	if (!value)
	{
		 m_errorReporter.typeError(0000_error, _literal.location(), "Invalid number literals.");
		 return false;
	}
	if (value->denominator() != 1)
	{
		 m_errorReporter.typeError(0000_error, _literal.location(), "Only integers are supported.");
		 return false;
	}
	literalAnnotation.type = m_typeSystem.freshTypeVariable(false, Sort{{TypeClass{BuiltinClass::Integer}}});
	return false;
}
