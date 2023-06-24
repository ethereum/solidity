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


#include <libsolidity/analysis/experimental/TypeRegistration.h>
#include <libsolidity/analysis/experimental/Analysis.h>
#include <liblangutil/Exceptions.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

TypeRegistration::TypeRegistration(Analysis& _analysis):
m_analysis(_analysis),
m_errorReporter(_analysis.errorReporter()),
m_typeSystem(_analysis.typeSystem())
{
	for (auto [type, name, arity]: std::initializer_list<std::tuple<BuiltinType, const char*, uint64_t>> {
		{BuiltinType::Void, "void", 0},
		{BuiltinType::Function, "fun", 2},
		{BuiltinType::Unit, "unit", 0},
		{BuiltinType::Pair, "pair", 2},
		{BuiltinType::Word, "word", 0},
		{BuiltinType::Integer, "integer", 0}
	})
		m_typeSystem.declareTypeConstructor(type, name, arity);
}

bool TypeRegistration::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool TypeRegistration::visit(TypeClassDefinition const& _typeClassDefinition)
{
	if (!m_visitedClasses.insert(_typeClassDefinition.id()).second)
		return false;

	return false;
}
bool TypeRegistration::visit(TypeClassInstantiation const& _typeClassInstantiation)
{
	auto const* classDefintion = dynamic_cast<TypeClassDefinition const*>(_typeClassInstantiation.typeClass().annotation().referencedDeclaration);
	if (!classDefintion)
		m_errorReporter.fatalTypeError(0000_error, _typeClassInstantiation.typeClass().location(), "Expected a type class.");
	classDefintion->accept(*this);

	TypeClass typeClass{classDefintion};

	TypeName const& typeName = _typeClassInstantiation.typeConstructor();

	TypeExpression::Constructor typeConstructor = [&]() -> TypeExpression::Constructor {
		if (auto const* elementaryTypeName = dynamic_cast<ElementaryTypeName const*>(&typeName))
		{
			switch(elementaryTypeName->typeName().token())
			{
			case Token::Word:
				return BuiltinType::Word;
			case Token::Void:
				return BuiltinType::Void;
			case Token::Integer:
				return BuiltinType::Integer;
			case Token::Pair:
				return BuiltinType::Pair;
			case Token::Function:
				return BuiltinType::Function;
			case Token::Unit:
				return BuiltinType::Function;
			default:
				m_errorReporter.typeError(0000_error, typeName.location(), "Only elementary types are supported.");
				return BuiltinType::Void;
			}
		}
		else if (auto const* userDefinedType = dynamic_cast<UserDefinedTypeName const*>(&typeName))
		{
			if (auto const* referencedDeclaration = userDefinedType->pathNode().annotation().referencedDeclaration)
				return referencedDeclaration;
			else
			{
				m_errorReporter.typeError(0000_error, userDefinedType->pathNode().location(), "No declaration found for user-defined type name.");
				return BuiltinType::Void;
			}
		}
		else
		{
			m_errorReporter.typeError(0000_error, typeName.location(), "Only elementary types are supported.");
			return BuiltinType::Void;
		}
	}();

	Arity arity{
		{},
		typeClass
	};
	if (_typeClassInstantiation.argumentSorts().size() != m_typeSystem.constructorArguments(typeConstructor))
		m_errorReporter.fatalTypeError(0000_error, _typeClassInstantiation.location(), "Invalid number of arguments.");

	for (auto argumentSort : _typeClassInstantiation.argumentSorts())
	{
		if (auto const* referencedDeclaration = argumentSort->annotation().referencedDeclaration)
		{
			if (!dynamic_cast<TypeClassDefinition const*>(referencedDeclaration))
				m_errorReporter.fatalTypeError(0000_error, argumentSort->location(), "Argument sort has to be a type class.");
			// TODO: multi arities
			arity.argumentSorts.emplace_back(Sort{{TypeClass{referencedDeclaration}}});
		}
		else
		{
			// TODO: error Handling
			m_errorReporter.fatalTypeError(0000_error, argumentSort->location(), "Invalid sort.");
		}
	}
	m_typeSystem.instantiateClass(typeConstructor, arity);

	if (
		auto [instantiation, newlyInserted] = annotation(*classDefintion).instantiations.emplace(typeConstructor, &_typeClassInstantiation);
		!newlyInserted
	)
	{
		SecondarySourceLocation ssl;
		ssl.append("Previous instantiation.", instantiation->second->location());
		m_errorReporter.typeError(0000_error, _typeClassInstantiation.location(), ssl, "Duplicate type class instantiation.");
	}

	return false;
}

bool TypeRegistration::visit(TypeDefinition const& _typeDefinition)
{
	m_typeSystem.declareTypeConstructor(
		TypeExpression::Constructor{&_typeDefinition},
		_typeDefinition.name(),
		_typeDefinition.arguments() ? _typeDefinition.arguments()->parameters().size() : 0
	);
	return false;
}

TypeRegistration::Annotation& TypeRegistration::annotation(ASTNode const& _node)
{
	return m_analysis.annotation<TypeRegistration>(_node);
}
