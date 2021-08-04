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
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Component that resolves type names to types and annotates the AST accordingly.
 */

#include <libsolidity/analysis/ReferencesResolver.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/ast/AST.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

#include <libsolutil/StringUtils.h>
#include <libsolutil/CommonData.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;


bool ReferencesResolver::resolve(ASTNode const& _root)
{
	auto errorWatcher = m_errorReporter.errorWatcher();
	_root.accept(*this);
	return errorWatcher.ok();
}

bool ReferencesResolver::visit(Block const& _block)
{
	if (!m_resolveInsideCode)
		return false;
	m_resolver.setScope(&_block);
	return true;
}

void ReferencesResolver::endVisit(Block const& _block)
{
	if (!m_resolveInsideCode)
		return;

	m_resolver.setScope(_block.scope());
}

bool ReferencesResolver::visit(TryCatchClause const& _tryCatchClause)
{
	if (!m_resolveInsideCode)
		return false;
	m_resolver.setScope(&_tryCatchClause);
	return true;
}

void ReferencesResolver::endVisit(TryCatchClause const& _tryCatchClause)
{
	if (!m_resolveInsideCode)
		return;

	m_resolver.setScope(_tryCatchClause.scope());
}

bool ReferencesResolver::visit(ForStatement const& _for)
{
	if (!m_resolveInsideCode)
		return false;
	m_resolver.setScope(&_for);
	return true;
}

void ReferencesResolver::endVisit(ForStatement const& _for)
{
	if (!m_resolveInsideCode)
		return;
	m_resolver.setScope(_for.scope());
}

void ReferencesResolver::endVisit(VariableDeclarationStatement const& _varDeclStatement)
{
	if (!m_resolveInsideCode)
		return;
	for (auto const& var: _varDeclStatement.declarations())
		if (var)
			m_resolver.activateVariable(var->name());
}

bool ReferencesResolver::visit(VariableDeclaration const& _varDecl)
{
	if (_varDecl.documentation())
		resolveInheritDoc(*_varDecl.documentation(), _varDecl.annotation());

	return true;
}

bool ReferencesResolver::visit(Identifier const& _identifier)
{
	auto declarations = m_resolver.nameFromCurrentScope(_identifier.name());
	if (declarations.empty())
	{
		string suggestions = m_resolver.similarNameSuggestions(_identifier.name());
		string errorMessage = "Undeclared identifier.";
		if (!suggestions.empty())
		{
			if ("\"" + _identifier.name() + "\"" == suggestions)
				errorMessage += " " + std::move(suggestions) + " is not (or not yet) visible at this point.";
			else
				errorMessage += " Did you mean " + std::move(suggestions) + "?";
		}
		m_errorReporter.declarationError(7576_error, _identifier.location(), errorMessage);
	}
	else if (declarations.size() == 1)
		_identifier.annotation().referencedDeclaration = declarations.front();
	else
		_identifier.annotation().candidateDeclarations = declarations;
	return false;
}

bool ReferencesResolver::visit(FunctionDefinition const& _functionDefinition)
{
	m_returnParameters.push_back(_functionDefinition.returnParameterList().get());

	if (_functionDefinition.documentation())
		resolveInheritDoc(*_functionDefinition.documentation(), _functionDefinition.annotation());

	return true;
}

void ReferencesResolver::endVisit(FunctionDefinition const&)
{
	solAssert(!m_returnParameters.empty(), "");
	m_returnParameters.pop_back();
}

bool ReferencesResolver::visit(ModifierDefinition const& _modifierDefinition)
{
	m_returnParameters.push_back(nullptr);

	if (_modifierDefinition.documentation())
		resolveInheritDoc(*_modifierDefinition.documentation(), _modifierDefinition.annotation());

	return true;
}

void ReferencesResolver::endVisit(ModifierDefinition const&)
{
	solAssert(!m_returnParameters.empty(), "");
	m_returnParameters.pop_back();
}

void ReferencesResolver::endVisit(IdentifierPath const& _path)
{
	Declaration const* declaration = m_resolver.pathFromCurrentScope(_path.path());
	if (!declaration)
	{
		m_errorReporter.fatalDeclarationError(7920_error, _path.location(), "Identifier not found or not unique.");
		return;
	}

	_path.annotation().referencedDeclaration = declaration;
}

bool ReferencesResolver::visit(InlineAssembly const& _inlineAssembly)
{
	m_yulAnnotation = &_inlineAssembly.annotation();
	(*this)(_inlineAssembly.operations());
	m_yulAnnotation = nullptr;

	return false;
}

bool ReferencesResolver::visit(Return const& _return)
{
	solAssert(!m_returnParameters.empty(), "");
	_return.annotation().functionReturnParameters = m_returnParameters.back();
	return true;
}

void ReferencesResolver::operator()(yul::FunctionDefinition const& _function)
{
	validateYulIdentifierName(_function.name, _function.debugData->location);
	for (yul::TypedName const& varName: _function.parameters + _function.returnVariables)
		validateYulIdentifierName(varName.name, varName.debugData->location);

	bool wasInsideFunction = m_yulInsideFunction;
	m_yulInsideFunction = true;
	this->operator()(_function.body);
	m_yulInsideFunction = wasInsideFunction;
}

void ReferencesResolver::operator()(yul::Identifier const& _identifier)
{
	static set<string> suffixes{"slot", "offset", "length"};
	string suffix;
	for (string const& s: suffixes)
		if (boost::algorithm::ends_with(_identifier.name.str(), "." + s))
			suffix = s;

	// Could also use `pathFromCurrentScope`, split by '.'
	auto declarations = m_resolver.nameFromCurrentScope(_identifier.name.str());
	if (!suffix.empty())
	{
		// special mode to access storage variables
		if (!declarations.empty())
			// the special identifier exists itself, we should not allow that.
			return;
		string realName = _identifier.name.str().substr(0, _identifier.name.str().size() - suffix.size() - 1);
		solAssert(!realName.empty(), "Empty name.");
		declarations = m_resolver.nameFromCurrentScope(realName);
		if (!declarations.empty())
			// To support proper path resolution, we have to use pathFromCurrentScope.
			solAssert(!util::contains(realName, '.'), "");
	}
	if (declarations.size() > 1)
	{
		m_errorReporter.declarationError(
			4718_error,
			_identifier.debugData->location,
			"Multiple matching identifiers. Resolving overloaded identifiers is not supported."
		);
		return;
	}
	else if (declarations.size() == 0)
	{
		if (
			boost::algorithm::ends_with(_identifier.name.str(), "_slot") ||
			boost::algorithm::ends_with(_identifier.name.str(), "_offset")
		)
			m_errorReporter.declarationError(
				9467_error,
				_identifier.debugData->location,
				"Identifier not found. Use \".slot\" and \".offset\" to access storage variables."
			);
		return;
	}
	if (auto var = dynamic_cast<VariableDeclaration const*>(declarations.front()))
		if (var->isLocalVariable() && m_yulInsideFunction)
		{
			m_errorReporter.declarationError(
				6578_error,
				_identifier.debugData->location,
				"Cannot access local Solidity variables from inside an inline assembly function."
			);
			return;
		}

	m_yulAnnotation->externalReferences[&_identifier].suffix = move(suffix);
	m_yulAnnotation->externalReferences[&_identifier].declaration = declarations.front();
}

void ReferencesResolver::operator()(yul::VariableDeclaration const& _varDecl)
{
	for (auto const& identifier: _varDecl.variables)
		validateYulIdentifierName(identifier.name, identifier.debugData->location);

	if (_varDecl.value)
		visit(*_varDecl.value);
}

void ReferencesResolver::resolveInheritDoc(StructuredDocumentation const& _documentation, StructurallyDocumentedAnnotation& _annotation)
{
	switch (_annotation.docTags.count("inheritdoc"))
	{
	case 0:
		break;
	case 1:
	{
		string const& name = _annotation.docTags.find("inheritdoc")->second.content;
		if (name.empty())
		{
			m_errorReporter.docstringParsingError(
				1933_error,
				_documentation.location(),
				"Expected contract name following documentation tag @inheritdoc."
			);
			return;
		}

		vector<string> path;
		boost::split(path, name, boost::is_any_of("."));
		if (any_of(path.begin(), path.end(), [](auto& _str) { return _str.empty(); }))
		{
			m_errorReporter.docstringParsingError(
				5967_error,
				_documentation.location(),
				"Documentation tag @inheritdoc reference \"" +
				name +
				"\" is malformed."
			);
			return;
		}
		Declaration const* result = m_resolver.pathFromCurrentScope(path);

		if (result == nullptr)
		{
			m_errorReporter.docstringParsingError(
				9397_error,
				_documentation.location(),
				"Documentation tag @inheritdoc references inexistent contract \"" +
				name +
				"\"."
			);
			return;
		}
		else
		{
			_annotation.inheritdocReference = dynamic_cast<ContractDefinition const*>(result);

			if (!_annotation.inheritdocReference)
				m_errorReporter.docstringParsingError(
					1430_error,
					_documentation.location(),
					"Documentation tag @inheritdoc reference \"" +
					name +
					"\" is not a contract."
				);
		}
		break;
	}
	default:
		m_errorReporter.docstringParsingError(
			5142_error,
			_documentation.location(),
			"Documentation tag @inheritdoc can only be given once."
		);
		break;
	}
}

void ReferencesResolver::validateYulIdentifierName(yul::YulString _name, SourceLocation const& _location)
{
	if (util::contains(_name.str(), '.'))
		m_errorReporter.declarationError(
			3927_error,
			_location,
			"User-defined identifiers in inline assembly cannot contain '.'."
		);

	if (set<string>{"this", "super", "_"}.count(_name.str()))
		m_errorReporter.declarationError(
			4113_error,
			_location,
			"The identifier name \"" + _name.str() + "\" is reserved."
		);

	auto declarations = m_resolver.nameFromCurrentScope(_name.str());
	if (!declarations.empty())
	{
		SecondarySourceLocation ssl;
		for (auto const* decl: declarations)
			if (decl->location().hasText())
				ssl.append("The shadowed declaration is here:", decl->location());
		if (!ssl.infos.empty())
			m_errorReporter.declarationError(
				3859_error,
				_location,
				ssl,
				"This declaration shadows a declaration outside the inline assembly block."
			);
	}
}
