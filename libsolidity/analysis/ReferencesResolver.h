/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Component that resolves type names to types and annotates the AST accordingly.
 */

#pragma once

#include <map>
#include <list>
#include <boost/noncopyable.hpp>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/ast/ASTAnnotations.h>

namespace dev
{
namespace solidity
{

class NameAndTypeResolver;

/**
 * Resolves references to declarations (of variables and types) and also establishes the link
 * between a return statement and the return parameter list.
 */
class ReferencesResolver: private ASTConstVisitor
{
public:
	ReferencesResolver(
		ErrorList& _errors,
		NameAndTypeResolver& _resolver,
		ContractDefinition const* _currentContract,
		ParameterList const* _returnParameters,
		bool _resolveInsideCode = false
	):
		m_errors(_errors),
		m_resolver(_resolver),
		m_currentContract(_currentContract),
		m_returnParameters(_returnParameters),
		m_resolveInsideCode(_resolveInsideCode)
	{}

	/// @returns true if no errors during resolving
	bool resolve(ASTNode& _root)
	{
		_root.accept(*this);
		return true;//m_errors.empty();
	}

private:
	virtual bool visit(Block const&) override { return m_resolveInsideCode; }
	virtual bool visit(Identifier const& _identifier) override;
	virtual bool visit(UserDefinedTypeName const& _typeName) override;
	virtual bool visit(Return const& _return) override;
	virtual void endVisit(VariableDeclaration const& _variable) override;

	TypePointer typeFor(TypeName const& _typeName);

	/// Adds a new error to the list of errors.
	void typeError(std::string const& _description, SourceLocation const& _location);

	/// Adds a new error to the list of errors and throws to abort type checking.
	void fatalTypeError(std::string const& _description);

	ErrorList& m_errors;
	NameAndTypeResolver& m_resolver;
	ContractDefinition const* m_currentContract;
	ParameterList const* m_returnParameters;
	bool m_resolveInsideCode;
};

}
}
