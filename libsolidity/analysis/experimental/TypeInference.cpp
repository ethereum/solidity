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
#include <libsolidity/analysis/experimental/Analysis.h>
#include <liblangutil/Exceptions.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

#include <range/v3/view/transform.hpp>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

TypeInference::TypeInference(Analysis& _analysis):
m_analysis(_analysis),
m_errorReporter(_analysis.errorReporter()),
m_typeSystem(_analysis.typeSystem())
{
	m_voidType = m_typeSystem.builtinType(BuiltinType::Void, {});
	m_wordType = m_typeSystem.builtinType(BuiltinType::Word, {});
	m_integerType = m_typeSystem.builtinType(BuiltinType::Integer, {});
	m_unitType = m_typeSystem.builtinType(BuiltinType::Unit, {});
	m_env = &m_typeSystem.env();
}

bool TypeInference::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool TypeInference::visit(FunctionDefinition const& _functionDefinition)
{
	ScopedSaveAndRestore signatureRestore(m_currentFunctionType, nullopt);
	auto& functionAnnotation = annotation(_functionDefinition);
	if (functionAnnotation.type)
		return false;

	_functionDefinition.parameterList().accept(*this);
	if (_functionDefinition.returnParameterList())
		_functionDefinition.returnParameterList()->accept(*this);

	auto typeFromParameterList = [&](ParameterList const* _list) {
		if (!_list)
			return m_typeSystem.builtinType(BuiltinType::Unit, {});
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
	auto& typeVariableAnnotation = annotation(_typeClassDefinition.typeVariable());
	if (typeVariableAnnotation.type)
		return false;
	_typeClassDefinition.typeVariable().accept(*this);

	for (auto const& subNode: _typeClassDefinition.subNodes())
		subNode->accept(*this);

	solAssert(typeVariableAnnotation.type);
	unify(*typeVariableAnnotation.type, m_typeSystem.freshTypeVariable(false, Sort{{TypeClass{&_typeClassDefinition}}}), _typeClassDefinition.location());
	return false;
}

/*
experimental::Type TypeInference::fromTypeName(TypeName const& _typeName)
{
	if (auto const* elementaryTypeName = dynamic_cast<ElementaryTypeName const*>(&_typeName))
	{
		switch(elementaryTypeName->typeName().token())
		{
		case Token::Word:
			return m_wordType;
		case Token::Void:
			return m_voidType;
		case Token::Integer:
			return m_integerType;
		default:
			m_errorReporter.typeError(0000_error, _typeName.location(), "Only elementary types are supported.");
			break;
		}
	}
	else if (auto const* userDefinedTypeName = dynamic_cast<UserDefinedTypeName const*>(&_typeName))
	{
		auto const* declaration = userDefinedTypeName->pathNode().annotation().referencedDeclaration;
		solAssert(declaration);
		if (auto const* variableDeclaration = dynamic_cast<VariableDeclaration const*>(declaration))
		{
			if (auto const* typeClass = dynamic_cast<TypeClassDefinition const*>(variableDeclaration->scope()))
			{
				typeClass->accept(*this);
				auto varType = annotation(typeClass->typeVariable()).type;
				solAssert(varType);
				return *varType;
			}
			else
				m_errorReporter.typeError(0000_error, _typeName.location(), "Type name referencing a variable declaration.");
		}
		else
			m_errorReporter.typeError(0000_error, _typeName.location(), "Unsupported user defined type name.");
	}
	else
		m_errorReporter.typeError(0000_error, _typeName.location(), "Unsupported type name.");
	return m_typeSystem.freshTypeVariable(false, {});
}
 */

void TypeInference::unify(Type _a, Type _b, langutil::SourceLocation _location)
{
	for (auto failure: m_env->unify(_a, _b))
		std::visit(util::GenericVisitor{
			[&](TypeEnvironment::TypeMismatch _typeMismatch) {
				m_errorReporter.typeError(0000_error, _location, fmt::format("Cannot unify {} and {}.", m_env->typeToString(_typeMismatch.a), m_env->typeToString(_typeMismatch.b)));
			},
			[&](TypeEnvironment::SortMismatch _sortMismatch) {
				m_errorReporter.typeError(0000_error, _location, fmt::format(
					"Cannot unify {} and {}: {} does not have sort {}",
					m_env->typeToString(_a),
					m_env->typeToString(_b),
					m_env->typeToString(_sortMismatch.type),
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
	switch(_expression.type().typeName().token())
	{
	case Token::Word:
		expressionAnnotation.type = m_wordType;
		break;
	case Token::Void:
		expressionAnnotation.type = m_voidType;
		break;
	case Token::Integer:
		expressionAnnotation.type = m_integerType;
		break;
	case Token::Unit:
		expressionAnnotation.type = m_unitType;
		break;
	case Token::Pair:
	{
		auto leftType = m_typeSystem.freshTypeVariable(true, {});
		auto rightType = m_typeSystem.freshTypeVariable(true, {});
		TypeSystemHelpers helper{m_typeSystem};
		expressionAnnotation.type =
			helper.functionType(
					  helper.tupleType({leftType, rightType}),
						m_typeSystem.type(BuiltinType::Pair, {leftType, rightType})
			);
		break;
	}
	case Token::Fun:
	{
		auto argType = m_typeSystem.freshTypeVariable(true, {});
		auto resultType = m_typeSystem.freshTypeVariable(true, {});
		TypeSystemHelpers helper{m_typeSystem};
		expressionAnnotation.type =
			helper.functionType(
				helper.tupleType({argType, resultType}),
				m_typeSystem.type(BuiltinType::Function, {argType, resultType})
			);
		break;
	}
	default:
		m_errorReporter.typeError(0000_error, _expression.location(), "Only elementary types are supported.");
		expressionAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
		break;
	}
	return false;
}

bool TypeInference::visit(BinaryOperation const& _binaryOperation)
{
	auto& operationAnnotation = annotation(_binaryOperation);
	auto& leftAnnotation = annotation(_binaryOperation.leftExpression());
	auto& rightAnnotation = annotation(_binaryOperation.rightExpression());
	switch (m_expressionContext)
	{
	case ExpressionContext::Term:
		m_errorReporter.typeError(0000_error, _binaryOperation.location(), "Binary operations in term context not yet supported.");
		operationAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
		return false;
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
		else
		{
			m_errorReporter.typeError(0000_error, _binaryOperation.location(), "Binary operations other than colon in type context not yet supported.");
			operationAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
		}
		return false;
	case ExpressionContext::Sort:
		m_errorReporter.typeError(0000_error, _binaryOperation.location(), "Invalid binary operation in sort context.");
		operationAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
		return false;
	}
	return false;
}

bool TypeInference::visit(VariableDeclaration const& _variableDeclaration)
{
	solAssert(!_variableDeclaration.value());
	auto& variableAnnotation = annotation(_variableDeclaration);
	solAssert(!variableAnnotation.type);

	if (_variableDeclaration.typeExpression())
	{
		ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		_variableDeclaration.typeExpression()->accept(*this);
		auto& typeExpressionAnnotation = annotation(*_variableDeclaration.typeExpression());
		solAssert(typeExpressionAnnotation.type);
		variableAnnotation.type = m_env->fresh(*typeExpressionAnnotation.type, false);
		return false;
	}

	variableAnnotation.type = m_typeSystem.freshTypeVariable(false, {});

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

bool TypeInference::visit(Identifier const& _identifier)
{
	auto& identifierAnnotation = annotation(_identifier);
	solAssert(!identifierAnnotation.type);

	auto const* referencedDeclaration = _identifier.annotation().referencedDeclaration;

	switch(m_expressionContext)
	{
	case ExpressionContext::Term:
	{
		solAssert(referencedDeclaration);

		if (
			!dynamic_cast<FunctionDefinition const*>(referencedDeclaration) &&
			!dynamic_cast<VariableDeclaration const*>(referencedDeclaration)
		)
		{
			SecondarySourceLocation ssl;
			ssl.append("Referenced node.", referencedDeclaration->location());
			m_errorReporter.fatalTypeError(0000_error, _identifier.location(), ssl, "Attempt to type identifier referring to unexpected node.");
		}

		auto& declarationAnnotation = annotation(*referencedDeclaration);
		if (!declarationAnnotation.type)
			referencedDeclaration->accept(*this);

		solAssert(declarationAnnotation.type);

		if (dynamic_cast<FunctionDefinition const*>(referencedDeclaration))
			identifierAnnotation.type = m_env->fresh(*declarationAnnotation.type, true);
		else if (dynamic_cast<VariableDeclaration const*>(referencedDeclaration))
			identifierAnnotation.type = declarationAnnotation.type;
		else
			solAssert(false);
		break;
	}
	case ExpressionContext::Type:
	{
		if (referencedDeclaration)
		{
			if (
				!dynamic_cast<VariableDeclaration const*>(referencedDeclaration) &&
				!dynamic_cast<TypeDefinition const*>(referencedDeclaration)
			)
			{
				SecondarySourceLocation ssl;
				ssl.append("Referenced node.", referencedDeclaration->location());
				m_errorReporter.fatalTypeError(0000_error, _identifier.location(), ssl, "Attempt to type identifier referring to unexpected node.");
			}

			// TODO: Assert that this is a type class variable declaration.
			auto& declarationAnnotation = annotation(*referencedDeclaration);
			if (!declarationAnnotation.type)
				referencedDeclaration->accept(*this);

			solAssert(declarationAnnotation.type);

			if (dynamic_cast<VariableDeclaration const*>(referencedDeclaration))
				identifierAnnotation.type = declarationAnnotation.type;
			else if (dynamic_cast<TypeDefinition const*>(referencedDeclaration))
				identifierAnnotation.type = m_env->fresh(*declarationAnnotation.type, true);
			else
				solAssert(false);
		}
		else
		{
			// TODO: register free type variable name!
			identifierAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
			return false;
		}
		break;
	}
	case ExpressionContext::Sort:
	{
		solAssert(referencedDeclaration);

		auto const* typeClass = dynamic_cast<TypeClassDefinition const*>(referencedDeclaration);
		if (
			!dynamic_cast<TypeClassDefinition const*>(referencedDeclaration)
		)
			m_errorReporter.fatalTypeError(0000_error, _identifier.location(), "Expected type class.");

		identifierAnnotation.type = m_typeSystem.freshTypeVariable(false, Sort{{TypeClass{typeClass}}});
		break;
	}
	}

	return true;
}

void TypeInference::endVisit(TupleExpression const& _tupleExpression)
{
	auto& expressionAnnotation = annotation(_tupleExpression);
	solAssert(!expressionAnnotation.type);

	auto componentTypes = _tupleExpression.components() | ranges::views::transform([&](auto _expr) -> Type {
		auto& componentAnnotation = annotation(*_expr);
		solAssert(componentAnnotation.type);
		return *componentAnnotation.type;
	}) | ranges::to<vector<Type>>;
	switch (m_expressionContext)
	{
	case ExpressionContext::Type:
	case ExpressionContext::Term:
		expressionAnnotation.type = TypeSystemHelpers{m_typeSystem}.tupleType(componentTypes);
		break;
	case ExpressionContext::Sort:
	{
		Type type = m_typeSystem.freshTypeVariable(false, {});
		for (auto componentType: componentTypes)
			unify(type, componentType, _tupleExpression.location());
		expressionAnnotation.type = type;
		break;
	}
	}
}

bool TypeInference::visit(IdentifierPath const&)
{
	solAssert(false);
}

bool TypeInference::visit(TypeClassInstantiation const& _typeClassInstantiation)
{
	auto const* typeClass = dynamic_cast<TypeClassDefinition const*>(_typeClassInstantiation.typeClass().annotation().referencedDeclaration);
	if (!typeClass)
		m_errorReporter.fatalTypeError(0000_error, _typeClassInstantiation.typeClass().location(), "Expected type class.");
	typeClass->accept(*this);

	map<string, Type> functionTypes;

	Type typeVar = m_typeSystem.freshTypeVariable(false, {});

	for (auto subNode: typeClass->subNodes())
	{
		auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(subNode.get());
		solAssert(functionDefinition);
		auto functionType = annotation(*functionDefinition).type;
		solAssert(functionType);
		functionTypes[functionDefinition->name()] = m_env->fresh(*functionType, true);
		auto typeVars = TypeSystemHelpers{m_typeSystem}.typeVars(functionTypes[functionDefinition->name()]);
		solAssert(typeVars.size() == 1);
		unify(typeVars.front(), typeVar);
	}
	for (auto subNode: _typeClassInstantiation.subNodes())
	{
		auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(subNode.get());
		solAssert(functionDefinition);
		Type* expectedFunctionType = util::valueOrNullptr(functionTypes, functionDefinition->name());
		if (!expectedFunctionType)
		{
			m_errorReporter.typeError(0000_error, functionDefinition->location(), "Function definition during instantiation that does not belong to the class.");
			continue;
		}
		subNode->accept(*this);
	}
	for (auto subNode: _typeClassInstantiation.subNodes())
	{
		auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(subNode.get());
		if (Type* expectedFunctionType = util::valueOrNullptr(functionTypes, functionDefinition->name()))
		{
			auto functionType = annotation(*functionDefinition).type;
			solAssert(functionType);
			// TODO: require exact match?
			unify(*functionType, *expectedFunctionType, functionDefinition->location());
			functionTypes.erase(functionDefinition->name());
		}
	}
	if (!functionTypes.empty())
		m_errorReporter.typeError(0000_error, _typeClassInstantiation.location(), "Type class instantiation does not implement all required functions.");

	return false;
}

bool TypeInference::visit(MemberAccess const& _memberAccess)
{
	if (m_expressionContext != ExpressionContext::Term)
	{
		m_errorReporter.typeError(0000_error, _memberAccess.location(), "Member access outside term context.");
		annotation(_memberAccess).type = m_typeSystem.freshTypeVariable(false, {});
		return false;

	}
	if (auto const* identifier = dynamic_cast<Identifier const*>(&_memberAccess.expression()))
	{
		auto const* declaration = identifier->annotation().referencedDeclaration;
		if (auto const* typeClass = dynamic_cast<TypeClassDefinition const*>(declaration))
		{
			for (auto subNode: typeClass->subNodes())
			{
				if (auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(subNode.get()))
				{
					if (functionDefinition->name() == _memberAccess.memberName())
					{
						auto& declarationAnnotation = annotation(*functionDefinition);
						if (!declarationAnnotation.type)
							functionDefinition->accept(*this);
						solAssert(declarationAnnotation.type);
						Type type = m_env->fresh(*declarationAnnotation.type, true);
						annotation(_memberAccess).type = type;
						auto typeVars = TypeSystemHelpers{m_typeSystem}.typeVars(type);
						if (typeVars.size() != 1)
							m_errorReporter.typeError(0000_error, _memberAccess.location(), "Type class reference does not uniquely depend on class type.");
						annotation(_memberAccess.expression()).type = typeVars.front();
						m_errorReporter.info(0000_error, _memberAccess.location(), m_env->typeToString(*declarationAnnotation.type));
						return false;
					}
				}
			}
			m_errorReporter.fatalTypeError(0000_error, _memberAccess.location(), "Unknown member of type-class.");
		}
		else
			m_errorReporter.fatalTypeError(0000_error, _memberAccess.location(), "Member access to non-type-class.");
	}
	else
		 m_errorReporter.fatalTypeError(0000_error, _memberAccess.location(), "Member access to non-identifier.");

	return false;
}

bool TypeInference::visit(TypeDefinition const& _typeDefinition)
{
	auto& typeDefinitionAnnotation = annotation(_typeDefinition);
	if (typeDefinitionAnnotation.type)
		 return false;

	if (_typeDefinition.typeExpression())
	{
		 ScopedSaveAndRestore expressionContext{m_expressionContext, ExpressionContext::Type};
		 _typeDefinition.typeExpression()->accept(*this);
	}

	vector<Type> arguments;
	if (_typeDefinition.arguments())
		 for (size_t i = 0; i < _typeDefinition.arguments()->parameters().size(); ++i)
			arguments.emplace_back(m_typeSystem.freshTypeVariable(true, {}));

	if (arguments.empty())
		 typeDefinitionAnnotation.type = m_typeSystem.type(TypeExpression::Constructor{&_typeDefinition}, arguments);
	else
		typeDefinitionAnnotation.type =
			TypeSystemHelpers{m_typeSystem}.functionType(
				TypeSystemHelpers{m_typeSystem}.tupleType(arguments),
				m_typeSystem.type(TypeExpression::Constructor{&_typeDefinition}, arguments)
			);
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

	std::vector<Type> argTypes;
	for(auto arg: _functionCall.arguments())
	{
		 auto& argAnnotation = annotation(*arg);
		 solAssert(argAnnotation.type);
		 argTypes.emplace_back(*argAnnotation.type);
	}

	switch(m_expressionContext)
	{
	case ExpressionContext::Term:
	case ExpressionContext::Type:
	{
		 Type argTuple = TypeSystemHelpers{m_typeSystem}.tupleType(argTypes);
		 Type genericFunctionType = TypeSystemHelpers{m_typeSystem}.functionType(argTuple, m_typeSystem.freshTypeVariable(false, {}));
		 unify(genericFunctionType, functionType, _functionCall.location());

		 functionCallAnnotation.type = m_env->resolve(std::get<1>(TypeSystemHelpers{m_typeSystem}.destFunctionType(m_env->resolve(genericFunctionType))));
		 break;
	}
	case ExpressionContext::Sort:
		 m_errorReporter.typeError(0000_error, _functionCall.location(), "Function call in sort context.");
		 functionCallAnnotation.type = m_typeSystem.freshTypeVariable(false, {});
		 break;
	}


}
