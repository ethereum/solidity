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
#include <libsolidity/experimental/analysis/ASTTransform.h>
#include <libsolidity/experimental/analysis/Analysis.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

#include <range/v3/algorithm/fold_right.hpp>
#include <range/v3/view/move.hpp>
#include <range/v3/view/drop.hpp>

namespace legacy = solidity::frontend;
using namespace std;
using namespace solidity::langutil;
using namespace solidity::frontend::experimental;

ASTTransform::ASTTransform(Analysis& _analysis): m_analysis(_analysis), m_errorReporter(_analysis.errorReporter()), m_ast(make_unique<AST>())
{
	m_ast->nodeById.resize(_analysis.maxAstId() + 1, nullptr);
}

bool ASTTransform::visit(legacy::TypeDefinition const& _typeDefinition)
{
	SetNode setNode(*this, _typeDefinition);
	auto [it, newlyInserted] = m_ast->typeDefinitions.emplace(&_typeDefinition, term(_typeDefinition));
	solAssert(newlyInserted);
	return false;
}

bool ASTTransform::visit(legacy::TypeClassDefinition const& _typeClassDefinition)
{
	SetNode setNode(*this, _typeClassDefinition);
	auto [it, newlyInserted] = m_ast->typeClasses.emplace(&_typeClassDefinition, term(_typeClassDefinition));
	solAssert(newlyInserted);
	return false;
}

bool ASTTransform::visit(legacy::TypeClassInstantiation const& _typeClassInstantiation)
{
	SetNode setNode(*this, _typeClassInstantiation);
	auto [it, newlyInserted] = m_ast->typeClassInstantiations.emplace(&_typeClassInstantiation, term(_typeClassInstantiation));
	solAssert(newlyInserted);
	return false;
}

bool ASTTransform::visit(legacy::ContractDefinition const& _contractDefinition)
{
	SetNode setNode(*this, _contractDefinition);
	auto [it, newlyInserted] = m_ast->contracts.emplace(&_contractDefinition, term(_contractDefinition));
	solAssert(newlyInserted);
	return false;
}

bool ASTTransform::visit(legacy::FunctionDefinition const& _functionDefinition)
{
	SetNode setNode(*this, _functionDefinition);
	solAssert(m_ast->functions.emplace(&_functionDefinition, term(_functionDefinition)).second);
	return false;
}

bool ASTTransform::visitNode(ASTNode const& _node)
{
	m_errorReporter.typeError(0000_error, _node.location(), "Unexpected AST node during AST transform.");
	return false;
}

unique_ptr<Term> ASTTransform::term(legacy::TypeDefinition const& _typeDefinition)
{
	SetNode setNode(*this, _typeDefinition);
	unique_ptr<Term> name = reference(_typeDefinition);
	unique_ptr<Term> arguments = termOrConstant(_typeDefinition.arguments(), BuiltinConstant::Unit);
	if (_typeDefinition.typeExpression())
	{
		unique_ptr<Term> definiens = term(*_typeDefinition.typeExpression());
		return application(BuiltinConstant::TypeDefinition, std::move(name), std::move(arguments), std::move(definiens));
	}
	else
		return application(BuiltinConstant::TypeDeclaration, std::move(name), std::move(arguments));
}

unique_ptr<Term> ASTTransform::term(legacy::TypeClassDefinition const& _typeClassDefinition)
{
	SetNode setNode(*this, _typeClassDefinition);
	unique_ptr<Term> typeVariable = term(_typeClassDefinition.typeVariable());
	unique_ptr<Term> name = reference(_typeClassDefinition);
	unique_ptr<Term> functions = namedFunctionList(_typeClassDefinition.subNodes());
	return application(
		BuiltinConstant::TypeClassDefinition,
		std::move(typeVariable),
		std::move(name),
		std::move(functions)
	);
}

std::unique_ptr<Term> ASTTransform::term(legacy::TypeClassInstantiation const& _typeClassInstantiation)
{
	SetNode setNode(*this, _typeClassInstantiation);
	unique_ptr<Term> typeConstructor = term(_typeClassInstantiation.typeConstructor());
	unique_ptr<Term> argumentSorts = termOrConstant(_typeClassInstantiation.argumentSorts(), BuiltinConstant::Unit);
	unique_ptr<Term> typeClass = term(_typeClassInstantiation.typeClass());
	unique_ptr<Term> functions = namedFunctionList(_typeClassInstantiation.subNodes());
	return application(
		BuiltinConstant::TypeClassInstantiation,
		std::move(typeConstructor),
		std::move(argumentSorts),
		std::move(typeClass),
		std::move(functions)
	);
}

std::unique_ptr<Term> ASTTransform::term(legacy::FunctionDefinition const& _functionDefinition)
{
	SetNode setNode(*this, _functionDefinition);
	unique_ptr<Term> name = reference(_functionDefinition);
	unique_ptr<Term> arguments = term(_functionDefinition.parameterList());
	unique_ptr<Term> returnType = termOrConstant(_functionDefinition.experimentalReturnExpression(), BuiltinConstant::Unit);
	if (_functionDefinition.isImplemented())
	{
		unique_ptr<Term> body = term(_functionDefinition.body());
		return application(
			BuiltinConstant::FunctionDefinition,
			std::move(name),
			std::move(arguments),
			std::move(returnType),
			std::move(body)
		);
	}
	else
		return application(
			BuiltinConstant::FunctionDeclaration,
			std::move(name),
			std::move(arguments),
			std::move(returnType)
		);
}

std::unique_ptr<Term> ASTTransform::term(legacy::ContractDefinition const& _contractDefinition)
{
	SetNode setNode(*this, _contractDefinition);
	unique_ptr<Term> name = reference(_contractDefinition);
	return application(
		BuiltinConstant::ContractDefinition,
		std::move(name),
		namedFunctionList(_contractDefinition.subNodes())
	);
}

unique_ptr<Term> ASTTransform::term(legacy::VariableDeclarationStatement const& _declaration)
{
	SetNode setNode(*this, _declaration);
	solAssert(_declaration.declarations().size() == 1);
	unique_ptr<Term> value;
	if (_declaration.initialValue())
		value = term(*_declaration.initialValue());
	return term(*_declaration.declarations().front(), std::move(value));
}

unique_ptr<Term> ASTTransform::term(legacy::Assignment const& _assignment)
{
	SetNode setNode(*this, _assignment);
	if (_assignment.assignmentOperator() == Token::Assign)
		return application(BuiltinConstant::Assign, _assignment.leftHandSide(), _assignment.rightHandSide());
	else
		solAssert(false);
}

unique_ptr<Term> ASTTransform::term(legacy::Block const& _block)
{
	SetNode setNode(*this, _block);

	if (auto statements = ranges::fold_right_last(
		_block.statements() | ranges::view::transform([&](auto stmt) { return term(*stmt); }) | ranges::view::move,
		[&](auto stmt, auto acc) {
			return application(BuiltinConstant::ChainStatements, std::move(stmt), std::move(acc));
		}
	))
		return application(BuiltinConstant::Block, std::move(*statements));
	else
		return application(BuiltinConstant::Block, constant(BuiltinConstant::Unit));
}

unique_ptr<Term> ASTTransform::term(legacy::Statement const& _statement)
{
	SetNode setNode(*this, _statement);
	if (auto const* assembly = dynamic_cast<legacy::InlineAssembly const*>(&_statement))
		return application(BuiltinConstant::RegularStatement, *assembly);
	else if (auto const* declaration = dynamic_cast<legacy::VariableDeclarationStatement const*>(&_statement))
		return application(BuiltinConstant::RegularStatement, *declaration);
	else if (auto const* assign = dynamic_cast<legacy::Assignment const*>(&_statement))
		return application(BuiltinConstant::RegularStatement, *assign);
	else if (auto const* expressionStatement = dynamic_cast<legacy::ExpressionStatement const*>(&_statement))
		return application(BuiltinConstant::RegularStatement, expressionStatement->expression());
	else if (auto const* returnStatement = dynamic_cast<legacy::Return const*>(&_statement))
		return application(BuiltinConstant::ReturnStatement, termOrConstant(returnStatement->expression(), BuiltinConstant::Unit));
	else
	{
		m_analysis.errorReporter().fatalTypeError(0000_error, _statement.location(), "Unsupported statement.");
		solAssert(false);
	}
}

unique_ptr<Term> ASTTransform::term(legacy::TypeName const& _name)
{
	SetNode setNode(*this, _name);
	if (auto const* elementaryTypeName = dynamic_cast<legacy::ElementaryTypeName const*>(&_name))
	{
		switch (elementaryTypeName->typeName().token())
		{
		case Token::Void:
			return constant(BuiltinConstant::Void);
		case Token::Fun:
			return constant(BuiltinConstant::Fun);
		case Token::Unit:
			return constant(BuiltinConstant::Unit);
		case Token::Pair:
			return constant(BuiltinConstant::Pair);
		case Token::Word:
			return constant(BuiltinConstant::Word);
		case Token::Integer:
			return constant(BuiltinConstant::Integer);
		case Token::Bool:
			return constant(BuiltinConstant::Bool);
		default:
			m_analysis.errorReporter().typeError(0000_error, m_currentLocation, "Unsupported type.");
			return constant(BuiltinConstant::Undefined);
		}
	}
	else if (auto const* userDefinedTypeName = dynamic_cast<UserDefinedTypeName const*>(&_name))
	{
		auto const* declaration = userDefinedTypeName->pathNode().annotation().referencedDeclaration;
		solAssert(declaration);
		return reference(*declaration);
	}
	else
		solAssert(false);
}

unique_ptr<Term> ASTTransform::term(legacy::TypeClassName const& _typeClassName)
{
	SetNode setNode(*this, _typeClassName);
	return std::visit(util::GenericVisitor{
		[&](Token _token) -> unique_ptr<Term> { return builtinTypeClass(_token); },
		[&](ASTPointer<legacy::IdentifierPath> _identifierPath) -> unique_ptr<Term> {
			solAssert(_identifierPath->annotation().referencedDeclaration);
			return reference(*_identifierPath->annotation().referencedDeclaration);
		}
	}, _typeClassName.name());
}

unique_ptr<Term> ASTTransform::term(legacy::ParameterList const& _parameterList)
{
	SetNode setNode(*this, _parameterList);
	return tuple(_parameterList.parameters() | ranges::view::transform([&](auto parameter) {
		solAssert(!parameter->value());
		return term(*parameter);
	}) | ranges::view::move | ranges::to<list<unique_ptr<Term>>>);
}

unique_ptr<Term> ASTTransform::term(legacy::VariableDeclaration const& _variableDeclaration, std::unique_ptr<Term> _initialValue)
{
	SetNode setNode(*this, _variableDeclaration);
	solAssert(!_variableDeclaration.value());
	unique_ptr<Term> name = reference(_variableDeclaration);
	if (_variableDeclaration.typeExpression())
		name = constrain(std::move(name), term(*_variableDeclaration.typeExpression()));
	if (_initialValue)
		return application(BuiltinConstant::VariableDefinition, std::move(name), std::move(_initialValue));
	else
		return application(BuiltinConstant::VariableDeclaration, std::move(name));
}

unique_ptr<Term> ASTTransform::term(legacy::InlineAssembly const& _inlineAssembly)
{
	SetNode setNode(*this, _inlineAssembly);
	std::map<yul::Identifier const*, unique_ptr<Term>> references;
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

		SetNode setNode(*this, originLocationOf(_identifier));
		references.emplace(&_identifier, reference(*declaration));
		identifierInfo->valueSize = 1;
		return true;
	};
	solAssert(!_inlineAssembly.annotation().analysisInfo, "");
	_inlineAssembly.annotation().analysisInfo = make_shared<yul::AsmAnalysisInfo>();
	yul::AsmAnalyzer analyzer(
		*_inlineAssembly.annotation().analysisInfo,
		m_analysis.errorReporter(),
		_inlineAssembly.dialect(),
		identifierAccess
	);
	if (!analyzer.analyze(_inlineAssembly.operations()))
		solAssert(m_analysis.errorReporter().hasErrors());

	return makeTerm<InlineAssembly>(_inlineAssembly.operations(), std::move(references));
}


unique_ptr<Term> ASTTransform::term(legacy::Expression const& _expression)
{
	SetNode setNode(*this, _expression);
	if (auto const* id = dynamic_cast<legacy::Identifier const*>(&_expression))
	{
		solAssert(id->annotation().referencedDeclaration);
		return reference(*id->annotation().referencedDeclaration);
	}
	else if (auto const* call = dynamic_cast<legacy::FunctionCall const*>(&_expression))
	{
		list<unique_ptr<Term>> arguments;
		for (auto argument: call->arguments())
			arguments.emplace_back(term(*argument));
		return application(term(call->expression()), std::move(arguments));
	}
	else if (auto const* assign = dynamic_cast<legacy::Assignment const*>(&_expression))
		return term(*assign);
	else if (auto const* memberAccess = dynamic_cast<legacy::MemberAccess const*>(&_expression))
	{
		unique_ptr<Term> memberNameConstant;
		{
			SetNode setMemberLocation(*this, memberAccess->memberLocation());
			memberNameConstant = constant(memberAccess->memberName());
		}
		return application(BuiltinConstant::MemberAccess, term(memberAccess->expression()), std::move(memberNameConstant));
	}
	else if (auto const* operation = dynamic_cast<legacy::BinaryOperation const*>(&_expression))
	{
		unique_ptr<Term> left = term(operation->leftExpression());
		unique_ptr<Term> right = term(operation->rightExpression());
		return binaryOperation(operation->getOperator(), std::move(left), std::move(right));
	}
	else if (auto const* typeNameExpression = dynamic_cast<legacy::ElementaryTypeNameExpression const*>(&_expression))
		return term(typeNameExpression->type());
	else if (auto const* tupleExpression = dynamic_cast<legacy::TupleExpression const*>(&_expression))
		return tuple(tupleExpression->components() | ranges::view::transform([&](auto component) {
			return term(*component);
		}) | ranges::view::move | ranges::to<list<unique_ptr<Term>>>);
	else
	{
		m_analysis.errorReporter().fatalTypeError(0000_error, _expression.location(), "Unsupported expression.");
		return tuple({});
	}
}

unique_ptr<Term> ASTTransform::reference(legacy::Declaration const& _declaration)
{
	return makeTerm<Reference>(static_cast<size_t>(_declaration.id()), _declaration.name());
}

unique_ptr<Term> ASTTransform::tuple(list<unique_ptr<Term>> _components)
{
	if (auto term = ranges::fold_right_last(_components | ranges::view::move, [&](auto a, auto b) { return pair(std::move(a), std::move(b)); }))
		return std::move(*term);
	else
		return constant(BuiltinConstant::Unit);
}

unique_ptr<Term> ASTTransform::constrain(unique_ptr<Term> _value, unique_ptr<Term> _constraint)
{
	return application(BuiltinConstant::Constrain, std::move(_value), std::move(_constraint));
}

std::unique_ptr<Term> ASTTransform::namedFunctionList(std::vector<ASTPointer<ASTNode>> _nodes)
{
	list<unique_ptr<Term>> functionList;
	for (auto subNode: _nodes)
	{
		auto const *function = dynamic_cast<FunctionDefinition const*>(subNode.get());
		solAssert(function);
		unique_ptr<Term> functionName = constant(function->name());
		unique_ptr<Term> functionDefinition = term(*function);
		functionList.emplace_back(application(BuiltinConstant::NamedTerm, std::move(functionName), std::move(functionDefinition)));
	}
	return tuple(std::move(functionList));
}

unique_ptr<Term> ASTTransform::builtinBinaryOperator(Token _token)
{
	switch (_token)
	{
	case Token::Colon:
		return constant(BuiltinConstant::Constrain);
	case Token::RightArrow:
		return constant(BuiltinConstant::Fun);
	case Token::Mul:
		return constant(BuiltinConstant::Mul);
	case Token::Add:
		return constant(BuiltinConstant::Add);
	case Token::Equal:
		return constant(BuiltinConstant::Equal);
	default:
		m_analysis.errorReporter().typeError(0000_error, m_currentLocation, "Unsupported operator.");
		return constant(BuiltinConstant::Undefined);
	}
}

unique_ptr<Term> ASTTransform::builtinTypeClass(langutil::Token _token)
{
	switch (_token)
	{
	case Token::Mul:
		return constant(BuiltinConstant::Mul);
	case Token::Add:
		return constant(BuiltinConstant::Add);
	case Token::Integer:
		return constant(BuiltinConstant::Integer);
	case Token::Equal:
		return constant(BuiltinConstant::Equal);
	default:
		m_analysis.errorReporter().typeError(0000_error, m_currentLocation, "Invalid type class.");
		return constant(BuiltinConstant::Undefined);
	}
}

TermBase ASTTransform::makeTermBase()
{
	return TermBase{
		m_currentLocation,
		m_currentNode ? make_optional(m_currentNode->id()) : nullopt,
		std::monostate{}
	};
}
