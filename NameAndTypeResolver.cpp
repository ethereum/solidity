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

NameAndTypeResolver::NameAndTypeResolver(vector<Declaration const*> const& _globals)
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
	std::vector<ContractDefinition const*> properBases(
		++_contract.getLinearizedBaseContracts().begin(),
		_contract.getLinearizedBaseContracts().end()
	);

	for (ContractDefinition const* base: properBases)
		importInheritedScope(*base);

	for (ASTPointer<StructDefinition> const& structDef: _contract.getDefinedStructs())
		ReferencesResolver resolver(*structDef, *this, &_contract, nullptr);
	for (ASTPointer<EnumDefinition> const& enumDef: _contract.getDefinedEnums())
		ReferencesResolver resolver(*enumDef, *this, &_contract, nullptr);
	for (ASTPointer<VariableDeclaration> const& variable: _contract.getStateVariables())
		ReferencesResolver resolver(*variable, *this, &_contract, nullptr);
	for (ASTPointer<EventDefinition> const& event: _contract.getEvents())
		ReferencesResolver resolver(*event, *this, &_contract, nullptr);

	// these can contain code, only resolve parameters for now
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

	m_currentScope = &m_scopes[&_contract];

	// now resolve references inside the code
	for (ASTPointer<ModifierDefinition> const& modifier: _contract.getFunctionModifiers())
	{
		m_currentScope = &m_scopes[modifier.get()];
		ReferencesResolver resolver(*modifier, *this, &_contract, nullptr, true);
	}
	for (ASTPointer<FunctionDefinition> const& function: _contract.getDefinedFunctions())
	{
		m_currentScope = &m_scopes[function.get()];
		ReferencesResolver referencesResolver(
			*function,
			*this,
			&_contract,
			function->getReturnParameterList().get(),
			true
		);
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
	m_scopes[nullptr].registerDeclaration(_declaration, false, true);
	solAssert(_declaration.getScope() == nullptr, "Updated declaration outside global scope.");
}

vector<Declaration const*> NameAndTypeResolver::resolveName(ASTString const& _name, Declaration const* _scope) const
{
	auto iterator = m_scopes.find(_scope);
	if (iterator == end(m_scopes))
		return vector<Declaration const*>({});
	return iterator->second.resolveName(_name, false);
}

vector<Declaration const*> NameAndTypeResolver::getNameFromCurrentScope(ASTString const& _name, bool _recursive)
{
	return m_currentScope->resolveName(_name, _recursive);
}

vector<Declaration const*> NameAndTypeResolver::cleanedDeclarations(
		Identifier const& _identifier,
		vector<Declaration const*> const& _declarations
)
{
	solAssert(_declarations.size() > 1, "");
	vector<Declaration const*> uniqueFunctions;

	for (auto it = _declarations.begin(); it != _declarations.end(); ++it)
	{
		solAssert(*it, "");
		// the declaration is functionDefinition while declarations > 1
		FunctionDefinition const& functionDefinition = dynamic_cast<FunctionDefinition const&>(**it);
		FunctionType functionType(functionDefinition);
		for (auto parameter: functionType.getParameterTypes() + functionType.getReturnParameterTypes())
			if (!parameter)
				BOOST_THROW_EXCEPTION(
					DeclarationError() <<
					errinfo_sourceLocation(_identifier.getLocation()) <<
					errinfo_comment("Function type can not be used in this context")
				);
		if (uniqueFunctions.end() == find_if(
			uniqueFunctions.begin(),
			uniqueFunctions.end(),
			[&](Declaration const* d)
			{
				FunctionType newFunctionType(dynamic_cast<FunctionDefinition const&>(*d));
				return functionType.hasEqualArgumentTypes(newFunctionType);
			}
		))
			uniqueFunctions.push_back(*it);
	}
	return uniqueFunctions;
}

void NameAndTypeResolver::importInheritedScope(ContractDefinition const& _base)
{
	auto iterator = m_scopes.find(&_base);
	solAssert(iterator != end(m_scopes), "");
	for (auto const& nameAndDeclaration: iterator->second.getDeclarations())
		for (auto const& declaration: nameAndDeclaration.second)
			// Import if it was declared in the base, is not the constructor and is visible in derived classes
			if (declaration->getScope() == &_base && declaration->isVisibleInDerivedContracts())
				m_currentScope->registerDeclaration(*declaration);
}

void NameAndTypeResolver::linearizeBaseContracts(ContractDefinition& _contract) const
{
	// order in the lists is from derived to base
	// list of lists to linearize, the last element is the list of direct bases
	list<list<ContractDefinition const*>> input(1, {});
	for (ASTPointer<InheritanceSpecifier> const& baseSpecifier: _contract.getBaseContracts())
	{
		ASTPointer<Identifier> baseName = baseSpecifier->getName();
		auto base = dynamic_cast<ContractDefinition const*>(&baseName->getReferencedDeclaration());
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

bool DeclarationRegistrationHelper::visit(EnumDefinition& _enum)
{
	registerDeclaration(_enum, true);
	return true;
}

void DeclarationRegistrationHelper::endVisit(EnumDefinition&)
{
	closeCurrentScope();
}

bool DeclarationRegistrationHelper::visit(EnumValue& _value)
{
	registerDeclaration(_value, false);
	return true;
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

void DeclarationRegistrationHelper::endVisit(VariableDeclarationStatement& _variableDeclarationStatement)
{
	// Register the local variables with the function
	// This does not fit here perfectly, but it saves us another AST visit.
	solAssert(m_currentFunction, "Variable declaration without function.");
	m_currentFunction->addLocalVariable(_variableDeclarationStatement.getDeclaration());
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
	if (!m_scopes[m_currentScope].registerDeclaration(_declaration, !_declaration.isVisibleInContract()))
	{
		SourceLocation firstDeclarationLocation;
		SourceLocation secondDeclarationLocation;
		Declaration const* conflictingDeclaration = m_scopes[m_currentScope].conflictingDeclaration(_declaration);
		solAssert(conflictingDeclaration, "");

		if (_declaration.getLocation().start < conflictingDeclaration->getLocation().start)
		{
			firstDeclarationLocation = _declaration.getLocation();
			secondDeclarationLocation = conflictingDeclaration->getLocation();
		}
		else
		{
			firstDeclarationLocation = conflictingDeclaration->getLocation();
			secondDeclarationLocation = _declaration.getLocation();
		}

		BOOST_THROW_EXCEPTION(
			DeclarationError() <<
			errinfo_sourceLocation(secondDeclarationLocation) <<
			errinfo_comment("Identifier already declared.") <<
			errinfo_secondarySourceLocation(
				SecondarySourceLocation().append("The previous declaration is here:", firstDeclarationLocation)
			)
		);
	}

	_declaration.setScope(m_currentScope);
	if (_opensScope)
		enterNewSubScope(_declaration);
}

ReferencesResolver::ReferencesResolver(
	ASTNode& _root,
	NameAndTypeResolver& _resolver,
	ContractDefinition const* _currentContract,
	ParameterList const* _returnParameters,
	bool _resolveInsideCode,
	bool _allowLazyTypes
):
	m_resolver(_resolver),
	m_currentContract(_currentContract),
	m_returnParameters(_returnParameters),
	m_resolveInsideCode(_resolveInsideCode),
	m_allowLazyTypes(_allowLazyTypes)
{
	_root.accept(*this);
}

void ReferencesResolver::endVisit(VariableDeclaration& _variable)
{
	// endVisit because the internal type needs resolving if it is a user defined type
	// or mapping
	if (_variable.getTypeName())
	{
		TypePointer type = _variable.getTypeName()->toType();
		using Location = VariableDeclaration::Location;
		Location loc = _variable.referenceLocation();
		// References are forced to calldata for external function parameters (not return)
		// and memory for parameters (also return) of publicly visible functions.
		// They default to memory for function parameters and storage for local variables.
		if (auto ref = dynamic_cast<ReferenceType const*>(type.get()))
		{
			if (_variable.isExternalCallableParameter())
			{
				// force location of external function parameters (not return) to calldata
				if (loc != Location::Default)
					BOOST_THROW_EXCEPTION(_variable.createTypeError(
						"Location has to be calldata for external functions "
						"(remove the \"memory\" or \"storage\" keyword)."
					));
				type = ref->copyForLocation(DataLocation::CallData, true);
			}
			else if (_variable.isCallableParameter() && _variable.getScope()->isPublic())
			{
				// force locations of public or external function (return) parameters to memory
				if (loc == VariableDeclaration::Location::Storage)
					BOOST_THROW_EXCEPTION(_variable.createTypeError(
						"Location has to be memory for publicly visible functions "
						"(remove the \"storage\" keyword)."
					));
				type = ref->copyForLocation(DataLocation::Memory, true);
			}
			else
			{
				if (loc == Location::Default)
					loc = _variable.isCallableParameter() ? Location::Memory : Location::Storage;
				bool isPointer = !_variable.isStateVariable();
				type = ref->copyForLocation(
					loc == Location::Memory ?
					DataLocation::Memory :
					DataLocation::Storage,
					isPointer
				);
			}
		}
		else if (loc != Location::Default && !ref)
			BOOST_THROW_EXCEPTION(_variable.createTypeError(
				"Storage location can only be given for array or struct types."
			));

		_variable.setType(type);

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
	auto declarations = m_resolver.getNameFromCurrentScope(_typeName.getName());
	if (declarations.empty())
		BOOST_THROW_EXCEPTION(
			DeclarationError() <<
			errinfo_sourceLocation(_typeName.getLocation()) <<
			errinfo_comment("Undeclared identifier.")
		);
	else if (declarations.size() > 1)
		BOOST_THROW_EXCEPTION(
			DeclarationError() <<
			errinfo_sourceLocation(_typeName.getLocation()) <<
			errinfo_comment("Duplicate identifier.")
		);
	else
		_typeName.setReferencedDeclaration(**declarations.begin());
	return false;
}

bool ReferencesResolver::visit(Identifier& _identifier)
{
	auto declarations = m_resolver.getNameFromCurrentScope(_identifier.getName());
	if (declarations.empty())
		BOOST_THROW_EXCEPTION(
			DeclarationError() <<
			errinfo_sourceLocation(_identifier.getLocation()) <<
			errinfo_comment("Undeclared identifier.")
		);
	else if (declarations.size() == 1)
		_identifier.setReferencedDeclaration(*declarations.front(), m_currentContract);
	else
		_identifier.setOverloadedDeclarations(m_resolver.cleanedDeclarations(_identifier, declarations));
	return false;
}

}
}
