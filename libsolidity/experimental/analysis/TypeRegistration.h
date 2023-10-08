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
#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/experimental/ast/TypeSystem.h>

#include <liblangutil/ErrorReporter.h>

namespace solidity::frontend::experimental
{

class Analysis;

class TypeRegistration: public ASTConstVisitor
{
public:
	using TypeClassInstantiations = std::map<TypeConstructor, TypeClassInstantiation const*>;
	struct Annotation
	{
		// For type class definitions.
		TypeClassInstantiations instantiations;
		// For builtins, type definitions, type class definitions, type names and type name expressions.
		std::optional<TypeConstructor> typeConstructor;
	};
	struct GlobalAnnotation
	{
		std::map<PrimitiveClass, TypeClassInstantiations> primitiveClassInstantiations;
		std::map<std::string, TypeDefinition const*> builtinTypeDefinitions;
	};
	TypeRegistration(Analysis& _analysis);

	bool analyze(SourceUnit const& _sourceUnit);
private:
	bool visit(TypeClassDefinition const& _typeClassDefinition) override;
	bool visit(TypeClassInstantiation const& _typeClassInstantiation) override;
	bool visit(TypeDefinition const& _typeDefinition) override;
	void endVisit(TypeDefinition const& _typeDefinition) override;
	bool visit(UserDefinedTypeName const& _typeName) override;
	void endVisit(ElementaryTypeNameExpression const& _typeName) override;
	bool visit(Builtin const& _builtin) override;
	Annotation& annotation(ASTNode const& _node);
	GlobalAnnotation& annotation();

	Analysis& m_analysis;
	langutil::ErrorReporter& m_errorReporter;
	TypeSystem& m_typeSystem;
	std::set<int64_t> m_visitedClasses;
};

}
