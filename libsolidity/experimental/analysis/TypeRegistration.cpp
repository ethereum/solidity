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

bool TypeRegistration::visit(ElementaryTypeName const& _typeName)
{
	if (annotation(_typeName).typeConstructor)
		return false;
	annotation(_typeName).typeConstructor = [&]() -> std::optional<TypeConstructor> {
		switch (_typeName.typeName().token())
		{
		case Token::Void:
			return m_typeSystem.constructor(PrimitiveType::Void);
		case Token::Fun:
			return m_typeSystem.constructor(PrimitiveType::Function);
		case Token::Unit:
			return m_typeSystem.constructor(PrimitiveType::Unit);
		case Token::Pair:
			return m_typeSystem.constructor(PrimitiveType::Pair);
		case Token::Word:
			return m_typeSystem.constructor(PrimitiveType::Word);
		case Token::IntegerType:
			return m_typeSystem.constructor(PrimitiveType::Integer);
		case Token::Bool:
			return m_typeSystem.constructor(PrimitiveType::Bool);
		default:
			m_errorReporter.fatalTypeError(7758_error, _typeName.location(), "Expected primitive type.");
			return std::nullopt;
		}
	}();
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
	auto* instantiations = std::visit(util::GenericVisitor{
		[&](ASTPointer<IdentifierPath> _path) -> TypeClassInstantiations*
		{
			if (TypeClassDefinition const* classDefinition = dynamic_cast<TypeClassDefinition const*>(_path->annotation().referencedDeclaration))
				return &annotation(*classDefinition).instantiations;
			m_errorReporter.typeError(3570_error, _typeClassInstantiation.typeClass().location(), "Expected a type class.");
			return nullptr;
		},
		[&](Token _token) -> TypeClassInstantiations*
		{
			if (auto typeClass = builtinClassFromToken(_token))
				return &annotation().builtinClassInstantiations[*typeClass];
			m_errorReporter.typeError(5262_error, _typeClassInstantiation.typeClass().location(), "Expected a type class.");
			return nullptr;
		}
	}, _typeClassInstantiation.typeClass().name());

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
	if (annotation(_typeDefinition).typeConstructor)
		return false;
	annotation(_typeDefinition).typeConstructor = m_typeSystem.declareTypeConstructor(
		_typeDefinition.name(),
		"t_" + *_typeDefinition.annotation().canonicalName + "_" + util::toString(_typeDefinition.id()),
		_typeDefinition.arguments() ? _typeDefinition.arguments()->parameters().size() : 0,
		&_typeDefinition
	);
	return true;
}

TypeRegistration::Annotation& TypeRegistration::annotation(ASTNode const& _node)
{
	return m_analysis.annotation<TypeRegistration>(_node);
}

TypeRegistration::GlobalAnnotation& TypeRegistration::annotation()
{
	return m_analysis.annotation<TypeRegistration>();
}
