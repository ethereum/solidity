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
#include <libsolidity/ast/experimental/TypeSystemHelper.h>
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
		{BuiltinType::Unit, "unit", 0},
		{BuiltinType::Pair, "pair", 2},
		{BuiltinType::Word, "word", 0},
		{BuiltinType::Integer, "integer", 0}
	})
		m_typeSystem.declareTypeConstructor(type, name, arity);

	auto declareBuiltinClass = [&](BuiltinClass _class, auto _memberCreator, Sort _sort = {}) {
		Type type = m_typeSystem.freshTypeVariable(false, std::move(_sort));
		auto error = m_typeSystem.declareTypeClass(
			TypeClass{_class},
			type,
			_memberCreator(type)
		);
		solAssert(!error, *error);
	};
	TypeSystemHelpers helper{m_typeSystem};
	using MemberList = std::map<std::string, Type>;

	declareBuiltinClass(BuiltinClass::Integer, [&](Type _typeVar) -> MemberList {
		return {
			{
				"fromInteger",
				helper.functionType(TypeConstant{{BuiltinType::Integer}, {}}, _typeVar)
			}
		};
	});
	declareBuiltinClass(BuiltinClass::Mul, [&](Type _typeVar) -> MemberList {
		return {
			{
				"mul",
				helper.functionType(helper.tupleType({_typeVar, _typeVar}), _typeVar)
			}
		};
	});
	annotation().operators[Token::Mul] = std::make_tuple(TypeClass{BuiltinClass::Mul}, "mul");
}

bool TypeRegistration::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool TypeRegistration::visit(TypeClassInstantiation const& _typeClassInstantiation)
{
	optional<TypeClass> typeClass = typeClassFromTypeClassName(_typeClassInstantiation.typeClass());
	if (!typeClass)
	{
		m_errorReporter.typeError(0000_error, _typeClassInstantiation.typeClass().location(), "Expected a type class.");
		return false;
	}

	optional<TypeConstructor> typeConstructor = typeConstructorFromTypeName(_typeClassInstantiation.typeConstructor());
	if (!typeConstructor)
	{
		m_errorReporter.typeError(0000_error, _typeClassInstantiation.typeConstructor().location(), "Invalid type name.");
		return false;
	}

	auto& instantiations = std::visit(util::GenericVisitor{
		[&](TypeClassDefinition const* classDefinition) -> auto&
		{
			return annotation(*classDefinition).instantiations;
		},
		[&](BuiltinClass _builtinClass) -> auto&
		{
			return annotation().builtinClassInstantiations[_builtinClass];
		}
	}, typeClass->declaration);


	if (
		auto [instantiation, newlyInserted] = instantiations.emplace(*typeConstructor, &_typeClassInstantiation);
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
		TypeConstructor{&_typeDefinition},
		_typeDefinition.name(),
		_typeDefinition.arguments() ? _typeDefinition.arguments()->parameters().size() : 0
	);
	return false;
}

TypeRegistration::Annotation& TypeRegistration::annotation(ASTNode const& _node)
{
	return m_analysis.annotation<TypeRegistration>(_node);
}


TypeRegistration::GlobalAnnotation& TypeRegistration::annotation()
{
	return m_analysis.annotation<TypeRegistration>();
}
