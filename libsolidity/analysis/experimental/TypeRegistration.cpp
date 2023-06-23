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
		m_typeSystem.declareBuiltinType(type, name, arity);
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
	auto const* classDefintion = dynamic_cast<TypeClassDefinition const*>(_typeClassInstantiation.sort().annotation().referencedDeclaration);
	if (!classDefintion)
		m_errorReporter.fatalTypeError(0000_error, _typeClassInstantiation.sort().location(), "Expected a type class.");
	classDefintion->accept(*this);

//	TypeClass typeClass{classDefintion};

	auto fromTypeName = [&](TypeName const& _typeName) -> Type {
		if (auto const* elementaryTypeName = dynamic_cast<ElementaryTypeName const*>(&_typeName))
		{
			switch(elementaryTypeName->typeName().token())
			{
			case Token::Word:
				return m_typeSystem.builtinType(BuiltinType::Word, {});
			case Token::Void:
				return m_typeSystem.builtinType(BuiltinType::Void, {});
			case Token::Integer:
				return m_typeSystem.builtinType(BuiltinType::Integer, {});
			default:
				m_errorReporter.typeError(0000_error, _typeName.location(), "Only elementary types are supported.");
				break;
			}
		}
		else
			m_errorReporter.typeError(0000_error, _typeName.location(), "Unsupported type name.");
		return m_typeSystem.freshTypeVariable(false);
	};
	auto type = fromTypeName(_typeClassInstantiation.typeConstructor());
	_typeClassInstantiation.argumentSorts();

//	m_typeSystem.instantiateClass();

	return false;
}

TypeRegistration::Annotation& TypeRegistration::annotation(ASTNode const& _node)
{
	return m_analysis.annotation<TypeRegistration>(_node);
}
