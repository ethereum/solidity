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

class InstantiationRegistration: public ASTConstVisitor
{
public:
	struct Annotation
	{
		// For type class names.
		std::optional<TypeClass> typeClass;

		// For type class names.
		// TODO: Remove. Only needed temporarily to find and visit the definition out of order
		// when visiting an instantiation in TypeInference.
		TypeClassDefinition const* typeClassDefinition = nullptr;

		// For instantiations.
		std::optional<Type> instanceType;
	};
	struct GlobalAnnotation
	{
	};

	InstantiationRegistration(Analysis& _analysis);

	bool analyze(SourceUnit const& _sourceUnit);

private:
	void endVisit(TypeClassName const& _typeClassName) override;
	void endVisit(TypeClassInstantiation const& _typeClassInstantiation) override;

	TypeClass resolveTypeClassName(Identifier const& _identifier) const;
	Sort extractSortFromTypeVariableDeclaration(VariableDeclaration const& _variableDeclaration) const;

	TypeClass typeClass(ASTNode const& _node) const;

	Annotation const& annotation(ASTNode const& _node) const;
	Annotation& annotation(ASTNode const& _node);
	GlobalAnnotation& annotation();

	Analysis& m_analysis;
	langutil::ErrorReporter& m_errorReporter;
	TypeSystem& m_typeSystem;
};

}
