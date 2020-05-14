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

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <liblangutil/EVMVersion.h>

#include <boost/noncopyable.hpp>
#include <list>
#include <map>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::frontend
{

/**
 * Assigns types to declarations.
 */
class DeclarationTypeChecker: private ASTConstVisitor
{
public:
	DeclarationTypeChecker(
		langutil::ErrorReporter& _errorReporter,
		langutil::EVMVersion _evmVersion
	):
		m_errorReporter(_errorReporter),
		m_evmVersion(_evmVersion)
	{}

	bool check(ASTNode const& _contract);

private:

	bool visit(ElementaryTypeName const& _typeName) override;
	void endVisit(UserDefinedTypeName const& _typeName) override;
	bool visit(FunctionTypeName const& _typeName) override;
	void endVisit(Mapping const& _mapping) override;
	void endVisit(ArrayTypeName const& _typeName) override;
	void endVisit(VariableDeclaration const& _variable) override;
	bool visit(StructDefinition const& _struct) override;

	/// Adds a new error to the list of errors.
	void typeError(langutil::SourceLocation const& _location, std::string const& _description);

	/// Adds a new error to the list of errors and throws to abort reference resolving.
	void fatalTypeError(langutil::SourceLocation const& _location, std::string const& _description);

	/// Adds a new error to the list of errors and throws to abort reference resolving.
	void fatalDeclarationError(langutil::SourceLocation const& _location, std::string const& _description);

	langutil::ErrorReporter& m_errorReporter;
	langutil::EVMVersion m_evmVersion;
	bool m_insideFunctionType = false;
	bool m_recursiveStructSeen = false;
	std::set<StructDefinition const*> m_currentStructsSeen;
};

}
