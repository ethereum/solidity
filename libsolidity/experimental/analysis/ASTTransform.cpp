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
}

bool ASTTransform::visit(legacy::ContractDefinition const& _contractDefinition)
{
	SetNode setNode(*this, _contractDefinition);
	auto [it, newlyInserted] = m_ast->contracts.emplace(&_contractDefinition, AST::ContractInfo{});
	solAssert(newlyInserted);
	AST::ContractInfo& contractInfo = it->second;
	for (auto const& node: _contractDefinition.subNodes())
		if (auto const* function = dynamic_cast<legacy::FunctionDefinition const*>(node.get()))
			solAssert(contractInfo.functions.emplace(string{}, functionDefinition(*function)).second);
		else
			m_errorReporter.typeError(0000_error, node->location(), "Unsupported contract element.");

	return false;
}

bool ASTTransform::visit(legacy::FunctionDefinition const& _functionDefinition)
{
	SetNode setNode(*this, _functionDefinition);
	solAssert(m_ast->functions.emplace(&_functionDefinition, functionDefinition(_functionDefinition)).second);
	return false;
}

bool ASTTransform::visit(legacy::TypeClassDefinition const& _typeClassDefinition)
{
	SetNode setNode(*this, _typeClassDefinition);
	auto [it, newlyInserted] = m_ast->typeClasses.emplace(&_typeClassDefinition, AST::TypeClassInformation{});
	solAssert(newlyInserted);
	auto& info = it->second;
	info.typeVariable = term(_typeClassDefinition.typeVariable());
	info.declaration = reference(_typeClassDefinition);
	declare(_typeClassDefinition, *info.declaration);
	map<std::string, AST::FunctionInfo>& functions = info.functions;
	for (auto subNode: _typeClassDefinition.subNodes())
	{
		auto const *function = dynamic_cast<FunctionDefinition const*>(subNode.get());
		solAssert(function);
		solAssert(functions.emplace(function->name(), functionDefinition(*function)).second);
	}
	return false;
}

bool ASTTransform::visit(legacy::TypeClassInstantiation const& _typeClassInstantiation)
{
	SetNode setNode(*this, _typeClassInstantiation);
	auto [it, newlyInserted] = m_ast->typeClassInstantiations.emplace(&_typeClassInstantiation, AST::TypeClassInstantiationInformation{});
	solAssert(newlyInserted);
	auto& info = it->second;
	info.typeClass = std::visit(util::GenericVisitor{
		[&](Token _token) -> unique_ptr<Term> { return builtinTypeClass(_token); },
		[&](ASTPointer<legacy::IdentifierPath> _identifierPath) -> unique_ptr<Term> {
			solAssert(_identifierPath->annotation().referencedDeclaration);
			return reference(*_identifierPath->annotation().referencedDeclaration);
		}
	}, _typeClassInstantiation.typeClass().name());
	info.typeConstructor = term(_typeClassInstantiation.typeConstructor());
	info.argumentSorts = termOrConstant(_typeClassInstantiation.argumentSorts(), BuiltinConstant::Unit);
	map<std::string, AST::FunctionInfo>& functions = info.functions;
	for (auto subNode: _typeClassInstantiation.subNodes())
	{
		auto const *function = dynamic_cast<FunctionDefinition const*>(subNode.get());
		solAssert(function);
		solAssert(functions.emplace(function->name(), functionDefinition(*function)).second);
	}
	return false;
}

bool ASTTransform::visit(legacy::TypeDefinition const& _typeDefinition)
{
	SetNode setNode(*this, _typeDefinition);
	auto [it, newlyInserted] = m_ast->typeDefinitions.emplace(&_typeDefinition, AST::TypeInformation{});
	solAssert(newlyInserted);
	auto& info = it->second;
	info.declaration = makeTerm<VariableDeclaration>(reference(_typeDefinition), nullptr);
	declare(_typeDefinition, *info.declaration);
	if (_typeDefinition.arguments())
		info.arguments = tuple(_typeDefinition.arguments()->parameters() | ranges::view::transform([&](auto argument){
			solAssert(!argument->typeExpression()); // TODO: error handling
			return term(*argument);
		}) | ranges::view::move | ranges::to<list<unique_ptr<Term>>>);
	if (_typeDefinition.typeExpression())
		info.value = term(*_typeDefinition.typeExpression());
	return false;
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

unique_ptr<Term> ASTTransform::term(TypeName const& _name)
{
	SetNode setNode(*this, _name);
	if (auto const* elementaryTypeName = dynamic_cast<ElementaryTypeName const*>(&_name))
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

unique_ptr<Term> ASTTransform::term(legacy::Statement const& _statement)
{
	SetNode setNode(*this, _statement);
	if (auto const* assembly = dynamic_cast<legacy::InlineAssembly const*>(&_statement))
		return term(*assembly);
	else if (auto const* declaration = dynamic_cast<legacy::VariableDeclarationStatement const*>(&_statement))
		return term(*declaration);
	else if (auto const* assign = dynamic_cast<legacy::Assignment const*>(&_statement))
		return term(*assign);
	else if (auto const* expressionStatement = dynamic_cast<legacy::ExpressionStatement const*>(&_statement))
		return term(expressionStatement->expression());
	else if (auto const* returnStatement = dynamic_cast<legacy::Return const*>(&_statement))
		return application(BuiltinConstant::Return, termOrConstant(returnStatement->expression(), BuiltinConstant::Unit));
	else
	{
		m_analysis.errorReporter().fatalTypeError(0000_error, _statement.location(), "Unsupported statement.");
		solAssert(false);
	}
}

unique_ptr<Term> ASTTransform::term(legacy::Block const& _block)
{
	SetNode setNode(*this, _block);
	if (_block.statements().empty())
		return application(BuiltinConstant::Block, constant(BuiltinConstant::Unit));
	auto makeStatement = [&](auto _stmt) {
		return application(BuiltinConstant::Statement, *_stmt);
	};

	return application(
		BuiltinConstant::Block,
		ranges::fold_right(
			_block.statements() | ranges::view::drop(1),
			makeStatement(_block.statements().front()),
			[&](auto stmt, auto acc) {
				return application(BuiltinConstant::ChainStatements, std::move(acc), makeStatement(stmt));
			}
		)
	);
}

AST::FunctionInfo ASTTransform::functionDefinition(legacy::FunctionDefinition const& _functionDefinition)
{
	SetNode setNode(*this, _functionDefinition);
	std::unique_ptr<Term> body = nullptr;
	unique_ptr<Term> argumentExpression = term(_functionDefinition.parameterList());
	if (_functionDefinition.isImplemented())
		body = term(_functionDefinition.body());
	unique_ptr<Term> returnType = termOrConstant(_functionDefinition.experimentalReturnExpression(), BuiltinConstant::Unit);
	unique_ptr<Term> name = reference(_functionDefinition);
	unique_ptr<Term> function = makeTerm<VariableDeclaration>(std::move(name), std::move(body));
	declare(_functionDefinition, *function);
	return AST::FunctionInfo{
		std::move(function),
		std::move(argumentExpression),
		std::move(returnType)
	};
}

unique_ptr<Term> ASTTransform::term(legacy::ParameterList const& _parameterList)
{
	SetNode setNode(*this, _parameterList);
	return tuple(_parameterList.parameters() | ranges::view::transform([&](auto parameter) {
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
	unique_ptr<Term> declaration = makeTerm<VariableDeclaration>(std::move(name), std::move(_initialValue));
	declare(_variableDeclaration, *declaration);
	return declaration;
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

unique_ptr<Term> ASTTransform::binaryOperation(
	Token _operator,
	unique_ptr<Term> _leftHandSide,
	unique_ptr<Term> _rightHandSide
)
{
	return application(builtinBinaryOperator(_operator), std::move(_leftHandSide), std::move(_rightHandSide));
}

unique_ptr<Term> ASTTransform::reference(legacy::Declaration const& _declaration)
{
	auto [it, newlyInserted] = m_declarationIndices.emplace(&_declaration, m_ast->declarations.size());
	if (newlyInserted)
		m_ast->declarations.emplace_back(AST::DeclarationInfo{nullptr, {}});
	return makeTerm<Reference>(it->second);
}

size_t ASTTransform::declare(legacy::Declaration const& _declaration, Term& _term)
{
	auto [it, newlyInserted] = m_declarationIndices.emplace(&_declaration, m_ast->declarations.size());
	if (newlyInserted)
		m_ast->declarations.emplace_back(AST::DeclarationInfo{&_term, _declaration.name()});
	else
	{
		auto& info = m_ast->declarations.at(it->second);
		solAssert(!info.target);
		info.target = &_term;
		info.name = _declaration.name();
	}
	termBase(_term).declaration = it->second;
	return it->second;
}

TermBase ASTTransform::makeTermBase()
{
	return TermBase{
		m_currentLocation,
		m_currentNode ? make_optional(m_currentNode->id()) : nullopt,
		std::monostate{},
		nullopt
	};
}

unique_ptr<Term> ASTTransform::constrain(unique_ptr<Term> _value, unique_ptr<Term> _constraint)
{
	return application(BuiltinConstant::Constrain, std::move(_value), std::move(_constraint));
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

unique_ptr<Term> ASTTransform::pair(unique_ptr<Term> _first, unique_ptr<Term> _second)
{
	return application(
		application(
			BuiltinConstant::Pair,
			std::move(_first)
		),
		std::move(_second)
	);
}

unique_ptr<Term> ASTTransform::tuple(list<unique_ptr<Term>> _components)
{
	if (auto term = ranges::fold_right_last(_components | ranges::view::move, [&](auto a, auto b) { return pair(std::move(a), std::move(b)); }))
		return std::move(*term);
	else
		return constant(BuiltinConstant::Unit);
}

unique_ptr<Term> ASTTransform::application(unique_ptr<Term> _function, std::list<unique_ptr<Term>> _arguments)
{
	return makeTerm<Application>(std::move(_function), tuple(std::move(_arguments)));
}
