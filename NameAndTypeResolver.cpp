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
	m_currentScope = &m_scopes[nullptr];

	for (ASTPointer<InheritanceSpecifier> const& baseContract: _contract.getBaseContracts())
		ReferencesResolver resolver(*baseContract, *this, &_contract, nullptr);

	m_currentScope = &m_scopes[&_contract];

	linearizeBaseContracts(_contract);
	for (ContractDefinition const* base: _contract.getLinearizedBaseContracts())
		importInheritedScope(*base);

	for (ASTPointer<StructDefinition> const& structDef: _contract.getDefinedStructs())
		ReferencesResolver resolver(*structDef, *this, &_contract, nullptr);
	for (ASTPointer<VariableDeclaration> const& variable: _contract.getStateVariables())
		ReferencesResolver resolver(*variable, *this, &_contract, nullptr);
	for (ASTPointer<EventDefinition> const& event: _contract.getEvents())
		ReferencesResolver resolver(*event, *this, &_contract, nullptr);
	for (ASTPointer<ModifierDefinition> const& modifier: _contract.getFunctionModifiers())
	{
		m_currentScope = &m_scopes[modifier.get()];
		ReferencesResolver resolver(*modifier, *this, &_contract, nullptr);
	}
	for (ASTPointer<FunctionDefinition> const& function: _contract.getDefinedFunctions())
	{
		m_currentScope = &m_scopes[function.get()];
		ReferencesResolver referencesResolver(*function, *this, &_contract,
											  function->getReturnParameterList().get());
	}
}

void NameAndTypeResolver::checkTypeRequirements(ContractDefinition& _contract)
{
	for (ASTPointer<StructDefinition> const& structDef: _contract.getDefinedStructs())
		structDef->checkValidityOfMembers();
	_contract.checkTypeRequirements();
}

void NameAndTypeResolver::updateDeclaration(Declaration const& _declaration)
{
	m_scopes[nullptr].registerDeclaration(_declaration, true);
	solAssert(_declaration.getScope() == nullptr, "Updated declaration outside global scope.");
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

void NameAndTypeResolver::importInheritedScope(ContractDefinition const& _base)
{
	auto iterator = m_scopes.find(&_base);
	solAssert(iterator != end(m_scopes), "");
	for (auto const& nameAndDeclaration: iterator->second.getDeclarations())
	{
		Declaration const* declaration = nameAndDeclaration.second;
		// Import if it was declared in the base and is not the constructor
		if (declaration->getScope() == &_base && declaration->getName() != _base.getName())
			m_currentScope->registerDeclaration(*declaration);
	}
}

void NameAndTypeResolver::linearizeBaseContracts(ContractDefinition& _contract) const
{
	// order in the lists is from derived to base
	// list of lists to linearize, the last element is the list of direct bases
	list<list<ContractDefinition const*>> input(1, {});
	for (ASTPointer<InheritanceSpecifier> const& baseSpecifier: _contract.getBaseContracts())
	{
		ASTPointer<Identifier> baseName = baseSpecifier->getName();
		ContractDefinition const* base = dynamic_cast<ContractDefinition const*>(
														baseName->getReferencedDeclaration());
		if (!base)
			BOOST_THROW_EXCEPTION(baseName->createTypeError("Contract expected."));
		// "push_front" has the effect that bases mentioned later can overwrite members of bases
		// mentioned earlier
		input.back().push_front(base);
		vector<ContractDefinition const*> const& basesBases = base->getLinearizedBaseContracts();
		if (basesBases.empty())
			BOOST_THROW_EXCEPTION(baseName->createTypeError("Definition of base has to precede definition of derived contract"));
		input.push_front(list<ContractDefinition const*>(basesBases.begin(), basesBases.end()));
	}
	input.back().push_front(&_contract);
	vector<ContractDefinition const*> result = cThreeMerge(input);
	if (result.empty())
		BOOST_THROW_EXCEPTION(_contract.createTypeError("Linearization of inheritance graph impossible"));
	_contract.setLinearizedBaseContracts(result);
}

template <class _T>
vector<_T const*> NameAndTypeResolver::cThreeMerge(list<list<_T const*>>& _toMerge)
{
	// returns true iff _candidate appears only as last element of the lists
	auto appearsOnlyAtHead = [&](_T const* _candidate) -> bool
	{
		for (list<_T const*> const& bases: _toMerge)
		{
			solAssert(!bases.empty(), "");
			if (find(++bases.begin(), bases.end(), _candidate) != bases.end())
				return false;
		}
		return true;
	};
	// returns the next candidate to append to the linearized list or nullptr on failure
	auto nextCandidate = [&]() -> _T const*
	{
		for (list<_T const*> const& bases: _toMerge)
		{
			solAssert(!bases.empty(), "");
			if (appearsOnlyAtHead(bases.front()))
				return bases.front();
		}
		return nullptr;
	};
	// removes the given contract from all lists
	auto removeCandidate = [&](_T const* _candidate)
	{
		for (auto it = _toMerge.begin(); it != _toMerge.end();)
		{
			it->remove(_candidate);
			if (it->empty())
				it = _toMerge.erase(it);
			else
				++it;
		}
	};

	_toMerge.remove_if([](list<_T const*> const& _bases) { return _bases.empty(); });
	vector<_T const*> result;
	while (!_toMerge.empty())
	{
		_T const* candidate = nextCandidate();
		if (!candidate)
			return vector<_T const*>();
		result.push_back(candidate);
		removeCandidate(candidate);
	}
	return result;
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

bool DeclarationRegistrationHelper::visit(ModifierDefinition& _modifier)
{
	registerDeclaration(_modifier, true);
	m_currentFunction = &_modifier;
	return true;
}

void DeclarationRegistrationHelper::endVisit(ModifierDefinition&)
{
	m_currentFunction = nullptr;
	closeCurrentScope();
}

void DeclarationRegistrationHelper::endVisit(VariableDefinition& _variableDefinition)
{
	// Register the local variables with the function
	// This does not fit here perfectly, but it saves us another AST visit.
	solAssert(m_currentFunction, "Variable definition without function.");
	m_currentFunction->addLocalVariable(_variableDefinition.getDeclaration());
}

bool DeclarationRegistrationHelper::visit(VariableDeclaration& _declaration)
{
	registerDeclaration(_declaration, false);
	return true;
}

bool DeclarationRegistrationHelper::visit(EventDefinition& _event)
{
	registerDeclaration(_event, true);
	return true;
}

void DeclarationRegistrationHelper::endVisit(EventDefinition&)
{
	closeCurrentScope();
}

void DeclarationRegistrationHelper::enterNewSubScope(Declaration const& _declaration)
{
	map<ASTNode const*, DeclarationContainer>::iterator iter;
	bool newlyAdded;
	tie(iter, newlyAdded) = m_scopes.emplace(&_declaration, DeclarationContainer(m_currentScope, &m_scopes[m_currentScope]));
	solAssert(newlyAdded, "Unable to add new scope.");
	m_currentScope = &_declaration;
}

void DeclarationRegistrationHelper::closeCurrentScope()
{
	solAssert(m_currentScope, "Closed non-existing scope.");
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
									   ContractDefinition const* _currentContract,
									   ParameterList const* _returnParameters, bool _allowLazyTypes):
	m_resolver(_resolver), m_currentContract(_currentContract),
	m_returnParameters(_returnParameters), m_allowLazyTypes(_allowLazyTypes)
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
	_return.setFunctionReturnParameters(m_returnParameters);
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
	_identifier.setReferencedDeclaration(*declaration, m_currentContract);
	return false;
}


}
}
