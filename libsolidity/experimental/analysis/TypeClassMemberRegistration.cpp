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


#include <libsolidity/experimental/analysis/TypeClassMemberRegistration.h>

#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/analysis/TypeClassRegistration.h>
#include <libsolidity/experimental/analysis/TypeRegistration.h>
#include <libsolidity/experimental/ast/TypeSystemHelper.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

#include <fmt/format.h>

using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

TypeClassMemberRegistration::TypeClassMemberRegistration(Analysis& _analysis):
	m_analysis(_analysis),
	m_errorReporter(_analysis.errorReporter()),
	m_typeSystem(_analysis.typeSystem())
{
}

bool TypeClassMemberRegistration::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

void TypeClassMemberRegistration::endVisit(Builtin const& _builtin)
{
	// Builtin may be used to register user defined types, which is handled earlier at TypeRegistration step
	if (!_builtin.functionParameter().has_value())
		return;

	// TODO: consider using a map of string-Token and ranges::find instead of ifs
	auto operatorToken = [](std::string const& _name) -> std::optional<Token> {
		if (_name == "+") return Token::Add;
		if (_name == "*") return Token::Mul;
		if (_name == "==") return Token::Equal;
		if (_name == "<") return Token::LessThan;
		if (_name == "<=") return Token::LessThanOrEqual;
		if (_name == ">") return Token::GreaterThan;
		if (_name == ">=") return Token::GreaterThanOrEqual;
		return std::nullopt;
	}(_builtin.nameParameter());

	// TODO: Handle conversion "fromInteger" (and other possible functions?)
	if (!operatorToken.has_value())
		m_errorReporter.fatalTypeError(
			77_error,
			_builtin.nameParameterLocation(),
			"Only operators +, *, ==, <, <=, >, >= are allowed as members of a type class."
		);

	ASTPointer<Expression> functionParameter = *_builtin.functionParameter();
	// TODO: Remove
	// solAssert(typeClassFunctionName->path().size() == 2);
	// std::string const& typeClassName = typeClassFunctionName->path()[0];
	// std::string const& functionName = typeClassFunctionName->path()[1];
	//
	// std::optional<TypeClass> typeClass = m_typeSystem.typeClass(typeClassName);
	//
	// if (!typeClass.has_value())
	// 	m_errorReporter.fatalTypeError(
	// 		78_error,
	// 		typeClassFunctionName->pathLocations()[0],
	// 		"Cannot find type class named " + typeClassName + '.'
	// 	);

	auto registrationResult = annotation().operators.emplace(operatorToken.value(),	functionParameter);
	if (!registrationResult.second)
		m_errorReporter.fatalTypeError(
			79_error,
			_builtin.location(),
			// TODO: registrationResult.first->second->location(),
			fmt::format(
				"Previous expression already associated with operator {}.",
				_builtin.nameParameter()
			)
		);
}

void TypeClassMemberRegistration::endVisit(TypeClassDefinition const& _typeClassDefinition)
{
	solAssert(m_analysis.annotation<TypeClassRegistration>(_typeClassDefinition).typeClass.has_value());
	TypeClass typeClass = m_analysis.annotation<TypeClassRegistration>(_typeClassDefinition).typeClass.value();

	_typeClassDefinition.typeVariable().accept(*this);

	std::map<std::string, Type> functionTypes;
	for (auto subNode: _typeClassDefinition.subNodes())
	{
		auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(subNode.get());
		solAssert(functionDefinition);

		std::optional<Type> functionType = TypeSystemHelpers{m_typeSystem}.functionType(
			m_typeSystem.freshTypeVariable({}),
			m_typeSystem.freshTypeVariable({})
		);

		if (!functionTypes.emplace(functionDefinition->name(), functionType.value()).second)
			m_errorReporter.fatalTypeError(
				3195_error,
				// TODO: Secondary location with previous definition
				functionDefinition->location(),
				"Function in type class declared multiple times."
			);
	}

	annotation().typeClassFunctions[typeClass] = std::move(functionTypes);
}

TypeClassMemberRegistration::Annotation& TypeClassMemberRegistration::annotation(ASTNode const& _node)
{
	return m_analysis.annotation<TypeClassMemberRegistration>(_node);
}

TypeClassMemberRegistration::Annotation const& TypeClassMemberRegistration::annotation(ASTNode const& _node) const
{
	return m_analysis.annotation<TypeClassMemberRegistration>(_node);
}

TypeClassMemberRegistration::GlobalAnnotation& TypeClassMemberRegistration::annotation()
{
	return m_analysis.annotation<TypeClassMemberRegistration>();
}
