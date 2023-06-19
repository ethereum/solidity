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


#include <libsolidity/analysis/experimental/TypeInference.h>
#include <liblangutil/Exceptions.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

bool TypeInference::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool TypeInference::visit(FunctionDefinition const& _functionDefinition)
{
	ScopedSaveAndRestore env{m_env, {}};
	_functionDefinition.parameterList().accept(*this);
	if (_functionDefinition.returnParameterList())
		_functionDefinition.returnParameterList()->accept(*this);

	_functionDefinition.body().accept(*this);

	return false;
}

bool TypeInference::visit(ParameterList const&)
{
	return true;
}

bool TypeInference::visitNode(ASTNode const& _node)
{
	m_errorReporter.typeError(0000_error, _node.location(), "Unsupported AST node during type inference.");
	return false;
}

experimental::Type TypeInference::fromTypeName(TypeName const& _typeName)
{
	if (auto const* elementaryTypeName = dynamic_cast<ElementaryTypeName const*>(&_typeName))
	{
		switch(elementaryTypeName->typeName().token())
		{
		case Token::Word:
			return WordType{};
		default:
			m_errorReporter.typeError(0000_error, _typeName.location(), "Only elementary types are supported.");
			break;
		}
	}
	else
		m_errorReporter.typeError(0000_error, _typeName.location(), "Only elementary types are supported.");
	return m_env.freshFreeType();

}

bool TypeInference::visit(InlineAssembly const& _inlineAssembly)
{
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
			// Hack until we can disallow any shadowing: If we found an internal reference,
			// clear the external references, so that codegen does not use it.
			_inlineAssembly.annotation().externalReferences.erase(& _identifier);
			return false;
		}
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		if (ref == _inlineAssembly.annotation().externalReferences.end())
			return false;
		InlineAssemblyAnnotation::ExternalIdentifierInfo& identifierInfo = ref->second;
		Declaration const* declaration = identifierInfo.declaration;
		solAssert(!!declaration, "");

		m_env.assignType(m_typeSystem, declaration, WordType{});
		identifierInfo.valueSize = 1;
		return true;
	};
	solAssert(!_inlineAssembly.annotation().analysisInfo, "");
	_inlineAssembly.annotation().analysisInfo = make_shared<yul::AsmAnalysisInfo>();
	yul::AsmAnalyzer analyzer(
		*_inlineAssembly.annotation().analysisInfo,
		m_errorReporter,
		_inlineAssembly.dialect(),
		identifierAccess
	);
	if (!analyzer.analyze(_inlineAssembly.operations()))
		solAssert(m_errorReporter.hasErrors());
	return false;
}

bool TypeInference::visit(VariableDeclaration const& _variableDeclaration)
{
	Type type = _variableDeclaration.hasTypeName() ? fromTypeName(_variableDeclaration.typeName()) : m_typeSystem.freshTypeVariable();
	m_env.assignType(m_typeSystem, &_variableDeclaration, type);
	return false;
}

bool TypeInference::visit(Assignment const& _assignment)
{
	(void)_assignment;
	return true;
}

bool TypeInference::visit(Identifier const& _identifier)
{
	(void)_identifier;
	return true;
}
