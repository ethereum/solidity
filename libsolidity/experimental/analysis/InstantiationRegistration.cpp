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


#include <libsolidity/experimental/analysis/InstantiationRegistration.h>

#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/analysis/TypeClassRegistration.h>
#include <libsolidity/experimental/analysis/TypeRegistration.h>
#include <libsolidity/experimental/ast/TypeSystemHelper.h>
#include <liblangutil/Exceptions.h>

using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

InstantiationRegistration::InstantiationRegistration(Analysis& _analysis):
	m_analysis(_analysis),
	m_errorReporter(_analysis.errorReporter()),
	m_typeSystem(_analysis.typeSystem())
{
}

bool InstantiationRegistration::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

void InstantiationRegistration::endVisit(TypeClassName const& _typeClassName)
{
	annotation(_typeClassName).typeClass = std::visit(util::GenericVisitor{
		[&](ASTPointer<IdentifierPath> _identifierPath) -> std::optional<TypeClass> {
			solAssert(_identifierPath);
			auto const* typeClassDefinition = dynamic_cast<TypeClassDefinition const*>(_identifierPath->annotation().referencedDeclaration);
			solAssert(typeClassDefinition);

			annotation(_typeClassName).typeClassDefinition = typeClassDefinition;

			solAssert(m_analysis.annotation<TypeClassRegistration>(*typeClassDefinition).typeClass.has_value());
			return *m_analysis.annotation<TypeClassRegistration>(*typeClassDefinition).typeClass;
		},
		[&](Token _token) -> std::optional<TypeClass> {
			auto const& classRegistrationAnnotation = m_analysis.annotation<TypeClassRegistration>();
			std::optional<BuiltinClass> builtinClass = builtinClassFromToken(_token);
			solAssert(builtinClass.has_value());
			solAssert(classRegistrationAnnotation.builtinClasses.count(*builtinClass) != 0);
			return classRegistrationAnnotation.builtinClasses.at(*builtinClass);
		}
	}, _typeClassName.name());
}

void InstantiationRegistration::endVisit(TypeClassInstantiation const& _typeClassInstantiation)
{
	std::optional<TypeConstructor> typeConstructor = m_analysis.annotation<TypeRegistration>(
		_typeClassInstantiation.typeConstructor()
	).typeConstructor;
	solAssert(typeConstructor);

	auto extractSort = [&](ASTPointer<VariableDeclaration> const& _declaration) {
		return extractSortFromTypeVariableDeclaration(*_declaration);
	};
	auto freshTypeVariable = [&](Sort const& _sort) {
		return m_typeSystem.freshTypeVariable(_sort);
	};

	std::vector<Sort> argumentSorts;
	std::vector<Type> argumentTypes;

	if (_typeClassInstantiation.argumentSorts())
	{
		argumentSorts = _typeClassInstantiation.argumentSorts()->parameters() |
			ranges::views::transform(extractSort) |
			ranges::to<std::vector<Sort>>;
		argumentTypes = argumentSorts |
			ranges::views::transform(freshTypeVariable) |
			ranges::to<std::vector<Type>>;
	}
	m_typeSystem.env().fixTypeVars(argumentTypes);

	Arity arity{std::move(argumentSorts), typeClass(_typeClassInstantiation.typeClass())};
	Type instanceType = Type{TypeConstant{*typeConstructor, std::move(argumentTypes)}};
	annotation(_typeClassInstantiation).instanceType = instanceType;

	std::optional<std::string> error = m_typeSystem.instantiateClass(instanceType, arity);
	if (error.has_value())
		m_errorReporter.typeError(5094_error, _typeClassInstantiation.location(), *error);
}

TypeClass InstantiationRegistration::resolveTypeClassName(Identifier const& _identifier) const
{
	// TODO: What about built-in classes, which have no declarations?
	Declaration const* referencedDeclaration = _identifier.annotation().referencedDeclaration;
	if (!referencedDeclaration)
		m_errorReporter.fatalTypeError(9789_error, _identifier.location(), "Undeclared identifier. Expected type class name.");

	auto const* typeClassDefinition = dynamic_cast<TypeClassDefinition const*>(referencedDeclaration);
	if (!typeClassDefinition)
		m_errorReporter.fatalTypeError(
			5650_error,
			_identifier.location(),
			SecondarySourceLocation{}.append("Referenced declaration:", referencedDeclaration->location()),
			"Expected type class."
		);

	auto const& typeClassAnnotation = m_analysis.annotation<TypeClassRegistration>(*typeClassDefinition);
	solAssert(typeClassAnnotation.typeClass.has_value());
	return *typeClassAnnotation.typeClass;
}

Sort InstantiationRegistration::extractSortFromTypeVariableDeclaration(VariableDeclaration const& _variableDeclaration) const
{
	if (!_variableDeclaration.typeExpression())
		return {};

	auto resolveExpressionAsIdentifier = [&](ASTPointer<Expression> const& _expression) {
		auto const* identifier = dynamic_cast<Identifier const*>(_expression.get());
		solAssert(identifier);
		return resolveTypeClassName(*identifier);
	};

	if (auto const* _identifier = dynamic_cast<Identifier const*>(_variableDeclaration.typeExpression().get()))
		return {{resolveTypeClassName(*_identifier)}};
	else if (auto const* _tupleExpression = dynamic_cast<TupleExpression const*>(_variableDeclaration.typeExpression().get()))
		return Sort{
			_tupleExpression->components() |
			ranges::views::transform(resolveExpressionAsIdentifier) |
			ranges::to<std::set<TypeClass>>
		};
	else
		solAssert(false);
}

TypeClass InstantiationRegistration::typeClass(ASTNode const& _node) const
{
	std::optional<TypeClass> result = annotation(_node).typeClass;
	solAssert(result.has_value());
	return *result;
}

InstantiationRegistration::Annotation const& InstantiationRegistration::annotation(ASTNode const& _node) const
{
	return m_analysis.annotation<InstantiationRegistration>(_node);
}

InstantiationRegistration::Annotation& InstantiationRegistration::annotation(ASTNode const& _node)
{
	return m_analysis.annotation<InstantiationRegistration>(_node);
}

InstantiationRegistration::GlobalAnnotation& InstantiationRegistration::annotation()
{
	return m_analysis.annotation<InstantiationRegistration>();
}
