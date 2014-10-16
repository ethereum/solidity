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
 * Parser part that determines the declarations corresponding to names and the types of expressions.
 */

#include <libsolidity/NameAndTypeResolver.h>

#include <libsolidity/AST.h>
#include <libsolidity/Exceptions.h>
#include <boost/assert.hpp>

namespace dev
{
namespace solidity
{


NameAndTypeResolver::NameAndTypeResolver()
{
}

void NameAndTypeResolver::resolveNamesAndTypes(ContractDefinition& _contract)
{
	reset();
	DeclarationRegistrationHelper registrar(m_scopes, _contract);
	m_currentScope = &m_scopes[&_contract];
	//@todo structs
	for (ptr<VariableDeclaration> const & variable : _contract.getStateVariables())
		ReferencesResolver resolver(*variable, *this, nullptr);
	for (ptr<FunctionDefinition> const & function : _contract.getDefinedFunctions())
	{
		m_currentScope = &m_scopes[function.get()];
		ReferencesResolver referencesResolver(*function, *this,
											  function->getReturnParameterList().get());
	}
	// First, all function parameter types need to be resolved before we can check
	// the types, since it is possible to call functions that are only defined later
	// in the source.
	for (ptr<FunctionDefinition> const & function : _contract.getDefinedFunctions())
	{
		m_currentScope = &m_scopes[function.get()];
		function->getBody().checkTypeRequirements();
	}
}

void NameAndTypeResolver::reset()
{
	m_scopes.clear();
	m_currentScope = nullptr;
}

Declaration* NameAndTypeResolver::getNameFromCurrentScope(ASTString const& _name, bool _recursive)
{
	return m_currentScope->resolveName(_name, _recursive);
}


DeclarationRegistrationHelper::DeclarationRegistrationHelper(std::map<ASTNode*, Scope>& _scopes, ASTNode& _astRoot)
	: m_scopes(_scopes), m_currentScope(&m_scopes[nullptr])
{
	_astRoot.accept(*this);
}

bool DeclarationRegistrationHelper::visit(ContractDefinition& _contract)
{
	registerDeclaration(_contract, true);
	return true;
}

void DeclarationRegistrationHelper::endVisit(ContractDefinition&)
{
	closeCurrentScope();
}

bool DeclarationRegistrationHelper::visit(StructDefinition& _struct)
{
	registerDeclaration(_struct, true);
	return true;
}

void DeclarationRegistrationHelper::endVisit(StructDefinition&)
{
	closeCurrentScope();
}

bool DeclarationRegistrationHelper::visit(FunctionDefinition& _function)
{
	registerDeclaration(_function, true);
	return true;
}

void DeclarationRegistrationHelper::endVisit(FunctionDefinition&)
{
	closeCurrentScope();
}

bool DeclarationRegistrationHelper::visit(VariableDeclaration& _declaration)
{
	registerDeclaration(_declaration, false);
	return true;
}

void DeclarationRegistrationHelper::endVisit(VariableDeclaration&)
{
}

void DeclarationRegistrationHelper::enterNewSubScope(ASTNode& _node)
{
	std::map<ASTNode*, Scope>::iterator iter;
	bool newlyAdded;
	std::tie(iter, newlyAdded) = m_scopes.emplace(&_node, Scope(m_currentScope));
	BOOST_ASSERT(newlyAdded);
	m_currentScope = &iter->second;
}

void DeclarationRegistrationHelper::closeCurrentScope()
{
	BOOST_ASSERT(m_currentScope != nullptr);
	m_currentScope = m_currentScope->getOuterScope();
}

void DeclarationRegistrationHelper::registerDeclaration(Declaration& _declaration, bool _opensScope)
{
	BOOST_ASSERT(m_currentScope != nullptr);
	if (!m_currentScope->registerDeclaration(_declaration))
		BOOST_THROW_EXCEPTION(DeclarationError() << errinfo_comment("Identifier already declared."));
	if (_opensScope)
		enterNewSubScope(_declaration);
}

ReferencesResolver::ReferencesResolver(ASTNode& _root, NameAndTypeResolver& _resolver,
									   ParameterList* _returnParameters)
	: m_resolver(_resolver), m_returnParameters(_returnParameters)
{
	_root.accept(*this);
}

void ReferencesResolver::endVisit(VariableDeclaration& _variable)
{
	// endVisit because the internal type needs resolving if it is a user defined type
	// or mapping
	if (_variable.getTypeName() != nullptr)
		_variable.setType(_variable.getTypeName()->toType());
	// otherwise we have a "var"-declaration whose type is resolved by the first assignment
}

bool ReferencesResolver::visit(Return& _return)
{
	BOOST_ASSERT(m_returnParameters != nullptr);
	_return.setFunctionReturnParameters(*m_returnParameters);
	return true;
}

bool ReferencesResolver::visit(Mapping&)
{
	// @todo
	return true;
}

bool ReferencesResolver::visit(UserDefinedTypeName& _typeName)
{
	Declaration* declaration = m_resolver.getNameFromCurrentScope(_typeName.getName());
	if (declaration == nullptr)
		BOOST_THROW_EXCEPTION(DeclarationError() << errinfo_comment("Undeclared identifier."));
	StructDefinition* referencedStruct = dynamic_cast<StructDefinition*>(declaration);
	//@todo later, contracts are also valid types
	if (referencedStruct == nullptr)
		BOOST_THROW_EXCEPTION(TypeError() << errinfo_comment("Identifier does not name a type name."));
	_typeName.setReferencedStruct(*referencedStruct);
	return false;
}

bool ReferencesResolver::visit(Identifier& _identifier)
{
	Declaration* declaration = m_resolver.getNameFromCurrentScope(_identifier.getName());
	if (declaration == nullptr)
		BOOST_THROW_EXCEPTION(DeclarationError() << errinfo_comment("Undeclared identifier."));
	_identifier.setReferencedDeclaration(*declaration);
	return false;
}


}
}
