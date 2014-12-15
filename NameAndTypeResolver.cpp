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

using namespace std;

namespace dev
{
namespace solidity
{


NameAndTypeResolver::NameAndTypeResolver(std::vector<Declaration const*> const& _globals)
{
	for (Declaration const* declaration: _globals)
		m_scopes[nullptr].registerDeclaration(*declaration);
}

void NameAndTypeResolver::registerDeclarations(SourceUnit& _sourceUnit)
{
	// The helper registers all declarations in m_scopes as a side-effect of its construction.
	DeclarationRegistrationHelper registrar(m_scopes, _sourceUnit);
}

void NameAndTypeResolver::resolveNamesAndTypes(ContractDefinition& _contract)
{
	m_currentScope = &m_scopes[&_contract];
	for (ASTPointer<StructDefinition> const& structDef: _contract.getDefinedStructs())
		ReferencesResolver resolver(*structDef, *this, nullptr);
	for (ASTPointer<StructDefinition> const& structDef: _contract.getDefinedStructs())
		structDef->checkValidityOfMembers();
	for (ASTPointer<VariableDeclaration> const& variable: _contract.getStateVariables())
		ReferencesResolver resolver(*variable, *this, nullptr);
	for (ASTPointer<FunctionDefinition> const& function: _contract.getDefinedFunctions())
	{
		m_currentScope = &m_scopes[function.get()];
		ReferencesResolver referencesResolver(*function, *this,
											  function->getReturnParameterList().get());
	}
	// First, the parameter types of all functions need to be resolved before we can check
	// the types, since it is possible to call functions that are only defined later
	// in the source.
	_contract.checkTypeRequirements();
	m_currentScope = &m_scopes[nullptr];
}

void NameAndTypeResolver::updateDeclaration(Declaration const& _declaration)
{
	m_scopes[nullptr].registerDeclaration(_declaration, true);
	if (asserts(_declaration.getScope() == nullptr))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Updated declaration outside global scope."));
}

Declaration const* NameAndTypeResolver::resolveName(ASTString const& _name, Declaration const* _scope) const
{
	auto iterator = m_scopes.find(_scope);
	if (iterator == end(m_scopes))
		return nullptr;
	return iterator->second.resolveName(_name, false);
}

Declaration const* NameAndTypeResolver::getNameFromCurrentScope(ASTString const& _name, bool _recursive)
{
	return m_currentScope->resolveName(_name, _recursive);
}

DeclarationRegistrationHelper::DeclarationRegistrationHelper(map<ASTNode const*, DeclarationContainer>& _scopes,
															 ASTNode& _astRoot):
	m_scopes(_scopes), m_currentScope(nullptr)
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
	m_currentFunction = &_function;
	return true;
}

void DeclarationRegistrationHelper::endVisit(FunctionDefinition&)
{
	m_currentFunction = nullptr;
	closeCurrentScope();
}

void DeclarationRegistrationHelper::endVisit(VariableDefinition& _variableDefinition)
{
	// Register the local variables with the function
	// This does not fit here perfectly, but it saves us another AST visit.
	if (asserts(m_currentFunction))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Variable definition without function."));
	m_currentFunction->addLocalVariable(_variableDefinition.getDeclaration());
}

bool DeclarationRegistrationHelper::visit(VariableDeclaration& _declaration)
{
	registerDeclaration(_declaration, false);
	return true;
}

void DeclarationRegistrationHelper::enterNewSubScope(Declaration const& _declaration)
{
	map<ASTNode const*, DeclarationContainer>::iterator iter;
	bool newlyAdded;
	tie(iter, newlyAdded) = m_scopes.emplace(&_declaration, DeclarationContainer(m_currentScope, &m_scopes[m_currentScope]));
	if (asserts(newlyAdded))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unable to add new scope."));
	m_currentScope = &_declaration;
}

void DeclarationRegistrationHelper::closeCurrentScope()
{
	if (asserts(m_currentScope))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Closed non-existing scope."));
	m_currentScope = m_scopes[m_currentScope].getEnclosingDeclaration();
}

void DeclarationRegistrationHelper::registerDeclaration(Declaration& _declaration, bool _opensScope)
{
	if (!m_scopes[m_currentScope].registerDeclaration(_declaration))
		BOOST_THROW_EXCEPTION(DeclarationError() << errinfo_sourceLocation(_declaration.getLocation())
												 << errinfo_comment("Identifier already declared."));
	//@todo the exception should also contain the location of the first declaration
	_declaration.setScope(m_currentScope);
	if (_opensScope)
		enterNewSubScope(_declaration);
}

ReferencesResolver::ReferencesResolver(ASTNode& _root, NameAndTypeResolver& _resolver,
									   ParameterList* _returnParameters, bool _allowLazyTypes):
	m_resolver(_resolver), m_returnParameters(_returnParameters), m_allowLazyTypes(_allowLazyTypes)
{
	_root.accept(*this);
}

void ReferencesResolver::endVisit(VariableDeclaration& _variable)
{
	// endVisit because the internal type needs resolving if it is a user defined type
	// or mapping
	if (_variable.getTypeName())
	{
		_variable.setType(_variable.getTypeName()->toType());
		if (!_variable.getType())
			BOOST_THROW_EXCEPTION(_variable.getTypeName()->createTypeError("Invalid type name"));
	}
	else if (!m_allowLazyTypes)
		BOOST_THROW_EXCEPTION(_variable.createTypeError("Explicit type needed."));
	// otherwise we have a "var"-declaration whose type is resolved by the first assignment
}

bool ReferencesResolver::visit(Return& _return)
{
	if (asserts(m_returnParameters))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Return parameters not set."));
	_return.setFunctionReturnParameters(*m_returnParameters);
	return true;
}

bool ReferencesResolver::visit(Mapping&)
{
	return true;
}

bool ReferencesResolver::visit(UserDefinedTypeName& _typeName)
{
	Declaration const* declaration = m_resolver.getNameFromCurrentScope(_typeName.getName());
	if (!declaration)
		BOOST_THROW_EXCEPTION(DeclarationError() << errinfo_sourceLocation(_typeName.getLocation())
												 << errinfo_comment("Undeclared identifier."));
	_typeName.setReferencedDeclaration(*declaration);
	return false;
}

bool ReferencesResolver::visit(Identifier& _identifier)
{
	Declaration const* declaration = m_resolver.getNameFromCurrentScope(_identifier.getName());
	if (!declaration)
		BOOST_THROW_EXCEPTION(DeclarationError() << errinfo_sourceLocation(_identifier.getLocation())
												 << errinfo_comment("Undeclared identifier."));
	_identifier.setReferencedDeclaration(*declaration);
	return false;
}


}
}
