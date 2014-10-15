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

namespace dev {
namespace solidity {


class NameAndTypeResolver::ScopeHelper {
public:
	ScopeHelper(NameAndTypeResolver& _resolver, Declaration& _declaration)
		: m_resolver(_resolver)
	{
		m_resolver.registerDeclaration(_declaration);
		m_resolver.enterNewSubScope(_declaration);
	}
	~ScopeHelper()
	{
		m_resolver.closeCurrentScope();
	}

private:
	NameAndTypeResolver& m_resolver;
};


NameAndTypeResolver::NameAndTypeResolver()
{
}

void NameAndTypeResolver::resolveNamesAndTypes(ContractDefinition& _contract)
{
	reset();

	handleContract(_contract);
}

void NameAndTypeResolver::handleContract(ContractDefinition& _contract)
{
	ScopeHelper scopeHelper(*this, _contract);

	// @todo structs (definition and usage)

	for (ptr<VariableDeclaration> const& variable : _contract.getStateVariables())
		registerVariableDeclarationAndResolveType(*variable);

	for (ptr<FunctionDefinition> const& function : _contract.getDefinedFunctions())
		handleFunction(*function);
}

void NameAndTypeResolver::reset()
{
	m_scopes.clear();
	m_globalScope = Scope();
	m_currentScope = &m_globalScope;
}

void NameAndTypeResolver::handleFunction(FunctionDefinition& _function)
{
	ScopeHelper scopeHelper(*this, _function);

	registerVariablesInFunction(_function);
	resolveReferencesInFunction(*_function.getReturnParameterList(), _function.getBody());
	_function.getBody().checkTypeRequirements();
}

void NameAndTypeResolver::registerVariablesInFunction(FunctionDefinition& _function)
{
	class VariableDeclarationFinder : public ASTVisitor {
	public:
		VariableDeclarationFinder(NameAndTypeResolver& _resolver) : m_resolver(_resolver) {}
		virtual bool visit(VariableDeclaration& _variable) override {
			m_resolver.registerVariableDeclarationAndResolveType(_variable);
			return false;
		}
	private:
		NameAndTypeResolver& m_resolver;
	};

	VariableDeclarationFinder declarationFinder(*this);
	_function.accept(declarationFinder);
}

void NameAndTypeResolver::resolveReferencesInFunction(ParameterList& _returnParameters,
													  Block& _functionBody)
{
	class ReferencesResolver : public ASTVisitor {
	public:
		ReferencesResolver(NameAndTypeResolver& _resolver,
						   ParameterList& _returnParameters)
			: m_resolver(_resolver), m_returnParameters(_returnParameters) {}
		virtual bool visit(Identifier& _identifier) override {
			Declaration* declaration = m_resolver.getNameFromCurrentScope(_identifier.getName());
			if (declaration == nullptr)
				BOOST_THROW_EXCEPTION(DeclarationError() << errinfo_comment("Undeclared identifier."));
			_identifier.setReferencedDeclaration(*declaration);
			return false;
		}
		virtual bool visit(Return& _return) override {
			_return.setFunctionReturnParameters(m_returnParameters);
			return true;
		}
	private:
		NameAndTypeResolver& m_resolver;
		ParameterList& m_returnParameters;
	};

	ReferencesResolver referencesResolver(*this, _returnParameters);
	_functionBody.accept(referencesResolver);
}

void NameAndTypeResolver::registerVariableDeclarationAndResolveType(VariableDeclaration& _variable)
{
	registerDeclaration(_variable);
	TypeName* typeName = _variable.getTypeName();
	if (typeName == nullptr) // unknown type, to be resolved by first assignment
		return;

	// walk the AST to resolve user defined type references
	// (walking is necessory because of mappings)
	// @todo this could probably also be done at an earlier stage where we anyway
	// walk the AST

	class UserDefinedTypeNameResolver : public ASTVisitor {
	public:
		UserDefinedTypeNameResolver(NameAndTypeResolver& _resolver)
			: m_resolver(_resolver) {}
		virtual bool visit(UserDefinedTypeName& _typeName) override {
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
		virtual bool visit(Mapping&) override {
			// @todo
			return true;
		}
	private:
		NameAndTypeResolver& m_resolver;
	};

	UserDefinedTypeNameResolver resolver(*this);
	_variable.accept(resolver);

	_variable.setType(typeName->toType());
}


void NameAndTypeResolver::registerDeclaration(Declaration& _declaration)
{
	if (!m_currentScope->registerDeclaration(_declaration))
		BOOST_THROW_EXCEPTION(DeclarationError() << errinfo_comment("Identifier already declared."));
}

Declaration* NameAndTypeResolver::getNameFromCurrentScope(ASTString const& _name, bool _recursive)
{
	return m_currentScope->resolveName(_name, _recursive);
}

void NameAndTypeResolver::enterNewSubScope(ASTNode& _node)
{
	decltype(m_scopes)::iterator iter;
	bool newlyAdded;
	std::tie(iter, newlyAdded) = m_scopes.emplace(&_node, Scope(m_currentScope));
	BOOST_ASSERT(newlyAdded);
	m_currentScope = &iter->second;
}

void NameAndTypeResolver::closeCurrentScope()
{
	m_currentScope = m_currentScope->getOuterScope();
}

} }
