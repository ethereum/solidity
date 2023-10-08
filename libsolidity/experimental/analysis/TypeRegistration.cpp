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


#include <libsolidity/experimental/analysis/TypeRegistration.h>
#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/ast/TypeSystemHelper.h>
#include <liblangutil/Exceptions.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

TypeRegistration::TypeRegistration(Analysis& _analysis):
	m_analysis(_analysis),
	m_errorReporter(_analysis.errorReporter()),
	m_typeSystem(_analysis.typeSystem())
{
}

bool TypeRegistration::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool TypeRegistration::visit(TypeClassDefinition const& _typeClassDefinition)
{
	if (annotation(_typeClassDefinition).typeConstructor)
		return false;
	annotation(_typeClassDefinition).typeConstructor = m_typeSystem.declareTypeConstructor(
		_typeClassDefinition.name(),
		"t_" + *_typeClassDefinition.annotation().canonicalName + "_" + util::toString(_typeClassDefinition.id()),
		0,
		&_typeClassDefinition
	);
	return true;
}

bool TypeRegistration::visit(Builtin const& _builtin)
{
	if (
		_builtin.typeClassFunctionParameter().has_value() ||
		annotation(_builtin).typeConstructor
	)
		return false;

	auto primitiveType = [&](std::string _name) -> std::optional<PrimitiveType> {
		if (_name == "void") return PrimitiveType::Void;
		if (_name == "fun") return PrimitiveType::Function;
		if (_name == "unit") return PrimitiveType::Unit;
		if (_name == "pair") return PrimitiveType::Pair;
		if (_name == "word") return PrimitiveType::Word;
		if (_name == "integer") return PrimitiveType::Integer;
		if (_name == "bool") return PrimitiveType::Bool;
		return std::nullopt;
	}(_builtin.nameParameter());

	if (!primitiveType.has_value())
		m_errorReporter.fatalTypeError(
			7758_error,
			_builtin.location(),
			"Expected the name of a built-in primitive type."
		);

	annotation(_builtin).typeConstructor = m_typeSystem.constructor(primitiveType.value());
	return true;
}

void TypeRegistration::endVisit(ElementaryTypeNameExpression const & _typeNameExpression)
{
	if (annotation(_typeNameExpression).typeConstructor)
		return;

	// TODO: this is not visited in the ElementaryTypeNameExpression visit - is that intentional?
	_typeNameExpression.type().accept(*this);
	if (auto constructor = annotation(_typeNameExpression.type()).typeConstructor)
		annotation(_typeNameExpression).typeConstructor = constructor;
	else
		solAssert(m_errorReporter.hasErrors());
}

bool TypeRegistration::visit(UserDefinedTypeName const& _userDefinedTypeName)
{
	if (annotation(_userDefinedTypeName).typeConstructor)
		return false;
	auto const* declaration = _userDefinedTypeName.pathNode().annotation().referencedDeclaration;
	if (!declaration)
	{
		// TODO: fatal/non-fatal
		m_errorReporter.fatalTypeError(5096_error, _userDefinedTypeName.pathNode().location(), "Expected declaration.");
		return false;
	}
	declaration->accept(*this);
	if (!(annotation(_userDefinedTypeName).typeConstructor = annotation(*declaration).typeConstructor))
	{
		// TODO: fatal/non-fatal
		m_errorReporter.fatalTypeError(9831_error, _userDefinedTypeName.pathNode().location(), "Expected type declaration.");
		return false;
	}
	return true;
}

bool TypeRegistration::visit(TypeClassInstantiation const& _typeClassInstantiation)
{
	if (annotation(_typeClassInstantiation).typeConstructor)
		return false;
	_typeClassInstantiation.typeConstructor().accept(*this);
	auto typeConstructor = annotation(_typeClassInstantiation.typeConstructor()).typeConstructor;
	if (!typeConstructor)
	{
		m_errorReporter.typeError(5577_error, _typeClassInstantiation.typeConstructor().location(), "Invalid type name.");
		return false;
	}
	auto* instantiations = [&](ASTPointer<IdentifierPath> _path) -> TypeClassInstantiations* {
		if (TypeClassDefinition const* classDefinition = dynamic_cast<TypeClassDefinition const*>(_path->annotation().referencedDeclaration))
			return &annotation(*classDefinition).instantiations;
		m_errorReporter.typeError(3570_error, _typeClassInstantiation.typeClass().location(), "Expected a type class.");
		return nullptr;
	}(_typeClassInstantiation.typeClass().name());

	if (!instantiations)
		return false;

	if (
		auto [instantiation, newlyInserted] = instantiations->emplace(*typeConstructor, &_typeClassInstantiation);
		!newlyInserted
	)
	{
		SecondarySourceLocation ssl;
		ssl.append("Previous instantiation.", instantiation->second->location());
		m_errorReporter.typeError(6620_error, _typeClassInstantiation.location(), ssl, "Duplicate type class instantiation.");
	}

	return true;
}

bool TypeRegistration::visit(TypeDefinition const& _typeDefinition)
{
	return !annotation(_typeDefinition).typeConstructor.has_value();
}

void TypeRegistration::endVisit(TypeDefinition const& _typeDefinition)
{
	if (annotation(_typeDefinition).typeConstructor.has_value())
		return;

	if (auto const* builtin = dynamic_cast<Builtin const*>(_typeDefinition.typeExpression()))
	{
		auto [previousDefinitionIt, inserted] = annotation().builtinTypeDefinitions.try_emplace(
			builtin->nameParameter(),
			&_typeDefinition
		);

		if (inserted)
			annotation(_typeDefinition).typeConstructor = annotation(*builtin).typeConstructor;
		else
		{
			auto const& [builtinName, previousDefinition] = *previousDefinitionIt;
			m_errorReporter.typeError(
				9609_error,
				_typeDefinition.location(),
				SecondarySourceLocation{}.append("Previous definition:", previousDefinition->location()),
				"Duplicate builtin type definition."
			);
		}
	}
	else
		annotation(_typeDefinition).typeConstructor = m_typeSystem.declareTypeConstructor(
			_typeDefinition.name(),
			"t_" + *_typeDefinition.annotation().canonicalName + "_" + util::toString(_typeDefinition.id()),
			_typeDefinition.arguments() ? _typeDefinition.arguments()->parameters().size() : 0,
			&_typeDefinition
		);
}

TypeRegistration::Annotation& TypeRegistration::annotation(ASTNode const& _node)
{
	return m_analysis.annotation<TypeRegistration>(_node);
}

TypeRegistration::GlobalAnnotation& TypeRegistration::annotation()
{
	return m_analysis.annotation<TypeRegistration>();
}
