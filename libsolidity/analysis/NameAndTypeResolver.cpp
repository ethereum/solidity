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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Parser part that determines the declarations corresponding to names and the types of expressions.
 */

#include <libsolidity/analysis/NameAndTypeResolver.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/analysis/TypeChecker.h>
#include <libsolidity/interface/ErrorReporter.h>
#include <libdevcore/StringUtils.h>

#include <boost/algorithm/string.hpp>

using namespace std;

namespace dev
{
namespace solidity
{

NameAndTypeResolver::NameAndTypeResolver(
	vector<Declaration const*> const& _globals,
	map<ASTNode const*, shared_ptr<DeclarationContainer>>& _scopes,
	ErrorReporter& _errorReporter
) :
	m_scopes(_scopes),
	m_errorReporter(_errorReporter)
{
	if (!m_scopes[nullptr])
		m_scopes[nullptr].reset(new DeclarationContainer());
	for (Declaration const* declaration: _globals)
	{
		solAssert(m_scopes[nullptr]->registerDeclaration(*declaration), "Unable to register global declaration.");
	}
}

bool NameAndTypeResolver::registerDeclarations(SourceUnit& _sourceUnit, ASTNode const* _currentScope)
{
	// The helper registers all declarations in m_scopes as a side-effect of its construction.
	try
	{
		DeclarationRegistrationHelper registrar(m_scopes, _sourceUnit, m_errorReporter, _currentScope);
	}
	catch (FatalError const&)
	{
		if (m_errorReporter.errors().empty())
			throw; // Something is weird here, rather throw again.
		return false;
	}
	return true;
}

bool NameAndTypeResolver::performImports(SourceUnit& _sourceUnit, map<string, SourceUnit const*> const& _sourceUnits)
{
	DeclarationContainer& target = *m_scopes.at(&_sourceUnit);
	bool error = false;
	for (auto const& node: _sourceUnit.nodes())
		if (auto imp = dynamic_cast<ImportDirective const*>(node.get()))
		{
			string const& path = imp->annotation().absolutePath;
			if (!_sourceUnits.count(path))
			{
				m_errorReporter.declarationError(
					imp->location(),
					"Import \"" + path + "\" (referenced as \"" + imp->path() + "\") not found."
				);
				error = true;
				continue;
			}
			auto scope = m_scopes.find(_sourceUnits.at(path));
			solAssert(scope != end(m_scopes), "");
			if (!imp->symbolAliases().empty())
				for (auto const& alias: imp->symbolAliases())
				{
					auto declarations = scope->second->resolveName(alias.first->name(), false);
					if (declarations.empty())
					{
						m_errorReporter.declarationError(
							imp->location(),
							"Declaration \"" +
							alias.first->name() +
							"\" not found in \"" +
							path +
							"\" (referenced as \"" +
							imp->path() +
							"\")."
						);
						error = true;
					}
					else
						for (Declaration const* declaration: declarations)
							if (!DeclarationRegistrationHelper::registerDeclaration(
								target, *declaration, alias.second.get(), &imp->location(), true, false, m_errorReporter
							))
								error = true;
				}
			else if (imp->name().empty())
				for (auto const& nameAndDeclaration: scope->second->declarations())
					for (auto const& declaration: nameAndDeclaration.second)
						if (!DeclarationRegistrationHelper::registerDeclaration(
							target, *declaration, &nameAndDeclaration.first, &imp->location(), true, false, m_errorReporter
						))
							error =  true;
		}
	return !error;
}

bool NameAndTypeResolver::resolveNamesAndTypes(ASTNode& _node, bool _resolveInsideCode)
{
	try
	{
		return resolveNamesAndTypesInternal(_node, _resolveInsideCode);
	}
	catch (FatalError const&)
	{
		if (m_errorReporter.errors().empty())
			throw; // Something is weird here, rather throw again.
		return false;
	}
}

bool NameAndTypeResolver::updateDeclaration(Declaration const& _declaration)
{
	try
	{
		m_scopes[nullptr]->registerDeclaration(_declaration, nullptr, false, true);
		solAssert(_declaration.scope() == nullptr, "Updated declaration outside global scope.");
	}
	catch (FatalError const&)
	{
		if (m_errorReporter.errors().empty())
			throw; // Something is weird here, rather throw again.
		return false;
	}
	return true;
}

void NameAndTypeResolver::activateVariable(string const& _name)
{
	solAssert(m_currentScope, "");
	// Scoped local variables are invisible before activation.
	// When a local variable is activated, its name is removed
	// from a scope's invisible variables.
	// This is used to avoid activation of variables of same name
	// in the same scope (an error is returned).
	if (m_currentScope->isInvisible(_name))
		m_currentScope->activateVariable(_name);
}

vector<Declaration const*> NameAndTypeResolver::resolveName(ASTString const& _name, ASTNode const* _scope) const
{
	auto iterator = m_scopes.find(_scope);
	if (iterator == end(m_scopes))
		return vector<Declaration const*>({});
	return iterator->second->resolveName(_name, false);
}

vector<Declaration const*> NameAndTypeResolver::nameFromCurrentScope(ASTString const& _name, bool _includeInvisibles) const
{
	return m_currentScope->resolveName(_name, true, _includeInvisibles);
}

Declaration const* NameAndTypeResolver::pathFromCurrentScope(vector<ASTString> const& _path) const
{
	solAssert(!_path.empty(), "");
	vector<Declaration const*> candidates = m_currentScope->resolveName(_path.front(), true);
	for (size_t i = 1; i < _path.size() && candidates.size() == 1; i++)
	{
		if (!m_scopes.count(candidates.front()))
			return nullptr;
		candidates = m_scopes.at(candidates.front())->resolveName(_path[i], false);
	}
	if (candidates.size() == 1)
		return candidates.front();
	else
		return nullptr;
}

vector<Declaration const*> NameAndTypeResolver::cleanedDeclarations(
		Identifier const& _identifier,
		vector<Declaration const*> const& _declarations
)
{
	solAssert(_declarations.size() > 1, "");
	vector<Declaration const*> uniqueFunctions;

	for (Declaration const* declaration: _declarations)
	{
		solAssert(declaration, "");
		// the declaration is functionDefinition, eventDefinition or a VariableDeclaration while declarations > 1
		solAssert(
			dynamic_cast<FunctionDefinition const*>(declaration) ||
			dynamic_cast<EventDefinition const*>(declaration) ||
			dynamic_cast<VariableDeclaration const*>(declaration) ||
			dynamic_cast<MagicVariableDeclaration const*>(declaration),
			"Found overloading involving something not a function, event or a (magic) variable."
		);

		FunctionTypePointer functionType { declaration->functionType(false) };
		if (!functionType)
			functionType = declaration->functionType(true);
		solAssert(functionType, "Failed to determine the function type of the overloaded.");

		for (auto parameter: functionType->parameterTypes() + functionType->returnParameterTypes())
			if (!parameter)
				m_errorReporter.fatalDeclarationError(_identifier.location(), "Function type can not be used in this context.");

		if (uniqueFunctions.end() == find_if(
			uniqueFunctions.begin(),
			uniqueFunctions.end(),
			[&](Declaration const* d)
			{
				shared_ptr<FunctionType const> newFunctionType { d->functionType(false) };
				if (!newFunctionType)
					newFunctionType = d->functionType(true);
				return newFunctionType && functionType->hasEqualArgumentTypes(*newFunctionType);
			}
		))
			uniqueFunctions.push_back(declaration);
	}
	return uniqueFunctions;
}

void NameAndTypeResolver::warnVariablesNamedLikeInstructions()
{
	for (auto const& instruction: c_instructions)
	{
		string const instructionName{boost::algorithm::to_lower_copy(instruction.first)};
		auto declarations = nameFromCurrentScope(instructionName, true);
		for (Declaration const* const declaration: declarations)
		{
			solAssert(!!declaration, "");
			if (dynamic_cast<MagicVariableDeclaration const* const>(declaration))
				// Don't warn the user for what the user did not.
				continue;
			m_errorReporter.warning(
				declaration->location(),
				"Variable is shadowed in inline assembly by an instruction of the same name"
			);
		}
	}
}

void NameAndTypeResolver::setScope(ASTNode const* _node)
{
	m_currentScope = m_scopes[_node].get();
}

bool NameAndTypeResolver::resolveNamesAndTypesInternal(ASTNode& _node, bool _resolveInsideCode)
{
	if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(&_node))
	{
		bool success = true;
		setScope(contract->scope());
		solAssert(!!m_currentScope, "");

		for (ASTPointer<InheritanceSpecifier> const& baseContract: contract->baseContracts())
			if (!resolveNamesAndTypes(*baseContract, true))
				success = false;

		setScope(contract);

		if (success)
		{
			linearizeBaseContracts(*contract);
			vector<ContractDefinition const*> properBases(
				++contract->annotation().linearizedBaseContracts.begin(),
				contract->annotation().linearizedBaseContracts.end()
			);

			for (ContractDefinition const* base: properBases)
				importInheritedScope(*base);
		}

		// these can contain code, only resolve parameters for now
		for (ASTPointer<ASTNode> const& node: contract->subNodes())
		{
			setScope(contract);
			if (!resolveNamesAndTypes(*node, false))
			{
				success = false;
				break;
			}
		}

		if (!success)
			return false;

		if (!_resolveInsideCode)
			return success;

		setScope(contract);

		// now resolve references inside the code
		for (ASTPointer<ASTNode> const& node: contract->subNodes())
		{
			setScope(contract);
			if (!resolveNamesAndTypes(*node, true))
				success = false;
		}
		return success;
	}
	else
	{
		if (m_scopes.count(&_node))
			setScope(&_node);
		return ReferencesResolver(m_errorReporter, *this, _resolveInsideCode).resolve(_node);
	}
}

void NameAndTypeResolver::importInheritedScope(ContractDefinition const& _base)
{
	auto iterator = m_scopes.find(&_base);
	solAssert(iterator != end(m_scopes), "");
	for (auto const& nameAndDeclaration: iterator->second->declarations())
		for (auto const& declaration: nameAndDeclaration.second)
			// Import if it was declared in the base, is not the constructor and is visible in derived classes
			if (declaration->scope() == &_base && declaration->isVisibleInDerivedContracts())
				if (!m_currentScope->registerDeclaration(*declaration))
				{
					SourceLocation firstDeclarationLocation;
					SourceLocation secondDeclarationLocation;
					Declaration const* conflictingDeclaration = m_currentScope->conflictingDeclaration(*declaration);
					solAssert(conflictingDeclaration, "");

					// Usual shadowing is not an error
					if (dynamic_cast<VariableDeclaration const*>(declaration) && dynamic_cast<VariableDeclaration const*>(conflictingDeclaration))
						continue;

					// Usual shadowing is not an error
					if (dynamic_cast<ModifierDefinition const*>(declaration) && dynamic_cast<ModifierDefinition const*>(conflictingDeclaration))
						continue;

					if (declaration->location().start < conflictingDeclaration->location().start)
					{
						firstDeclarationLocation = declaration->location();
						secondDeclarationLocation = conflictingDeclaration->location();
					}
					else
					{
						firstDeclarationLocation = conflictingDeclaration->location();
						secondDeclarationLocation = declaration->location();
					}

					m_errorReporter.declarationError(
						secondDeclarationLocation,
						SecondarySourceLocation().append("The previous declaration is here:", firstDeclarationLocation),
						"Identifier already declared."
					);
				}
}

void NameAndTypeResolver::linearizeBaseContracts(ContractDefinition& _contract)
{
	// order in the lists is from derived to base
	// list of lists to linearize, the last element is the list of direct bases
	list<list<ContractDefinition const*>> input(1, list<ContractDefinition const*>{});
	for (ASTPointer<InheritanceSpecifier> const& baseSpecifier: _contract.baseContracts())
	{
		UserDefinedTypeName const& baseName = baseSpecifier->name();
		auto base = dynamic_cast<ContractDefinition const*>(baseName.annotation().referencedDeclaration);
		if (!base)
			m_errorReporter.fatalTypeError(baseName.location(), "Contract expected.");
		// "push_front" has the effect that bases mentioned later can overwrite members of bases
		// mentioned earlier
		input.back().push_front(base);
		vector<ContractDefinition const*> const& basesBases = base->annotation().linearizedBaseContracts;
		if (basesBases.empty())
			m_errorReporter.fatalTypeError(baseName.location(), "Definition of base has to precede definition of derived contract");
		input.push_front(list<ContractDefinition const*>(basesBases.begin(), basesBases.end()));
	}
	input.back().push_front(&_contract);
	vector<ContractDefinition const*> result = cThreeMerge(input);
	if (result.empty())
		m_errorReporter.fatalTypeError(_contract.location(), "Linearization of inheritance graph impossible");
	_contract.annotation().linearizedBaseContracts = result;
	_contract.annotation().contractDependencies.insert(result.begin() + 1, result.end());
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

string NameAndTypeResolver::similarNameSuggestions(ASTString const& _name) const
{
	return quotedAlternativesList(m_currentScope->similarNames(_name));
}

DeclarationRegistrationHelper::DeclarationRegistrationHelper(
	map<ASTNode const*, shared_ptr<DeclarationContainer>>& _scopes,
	ASTNode& _astRoot,
	ErrorReporter& _errorReporter,
	ASTNode const* _currentScope
):
	m_scopes(_scopes),
	m_currentScope(_currentScope),
	m_errorReporter(_errorReporter)
{
	_astRoot.accept(*this);
	solAssert(m_currentScope == _currentScope, "Scopes not correctly closed.");
}

bool DeclarationRegistrationHelper::registerDeclaration(
	DeclarationContainer& _container,
	Declaration const& _declaration,
	string const* _name,
	SourceLocation const* _errorLocation,
	bool _warnOnShadow,
	bool _inactive,
	ErrorReporter& _errorReporter
)
{
	if (!_errorLocation)
		_errorLocation = &_declaration.location();

	string name = _name ? *_name : _declaration.name();
	Declaration const* shadowedDeclaration = nullptr;
	if (_warnOnShadow && !name.empty() && _container.enclosingContainer())
		for (auto const* decl: _container.enclosingContainer()->resolveName(name, true, true))
			shadowedDeclaration = decl;

	// We use "invisible" for both inactive variables in blocks and for members invisible in contracts.
	// They cannot both be true at the same time.
	solAssert(!(_inactive && !_declaration.isVisibleInContract()), "");
	if (!_container.registerDeclaration(_declaration, _name, !_declaration.isVisibleInContract() || _inactive))
	{
		SourceLocation firstDeclarationLocation;
		SourceLocation secondDeclarationLocation;
		Declaration const* conflictingDeclaration = _container.conflictingDeclaration(_declaration, _name);
		solAssert(conflictingDeclaration, "");
		bool const comparable =
			_errorLocation->sourceName &&
			conflictingDeclaration->location().sourceName &&
			*_errorLocation->sourceName == *conflictingDeclaration->location().sourceName;
		if (comparable && _errorLocation->start < conflictingDeclaration->location().start)
		{
			firstDeclarationLocation = *_errorLocation;
			secondDeclarationLocation = conflictingDeclaration->location();
		}
		else
		{
			firstDeclarationLocation = conflictingDeclaration->location();
			secondDeclarationLocation = *_errorLocation;
		}

		_errorReporter.declarationError(
			secondDeclarationLocation,
			SecondarySourceLocation().append("The previous declaration is here:", firstDeclarationLocation),
			"Identifier already declared."
		);
		return false;
	}
	else if (shadowedDeclaration)
	{
		if (dynamic_cast<MagicVariableDeclaration const*>(shadowedDeclaration))
			_errorReporter.warning(
				_declaration.location(),
				"This declaration shadows a builtin symbol."
			);
		else
		{
			auto shadowedLocation = shadowedDeclaration->location();
			_errorReporter.warning(
				_declaration.location(),
				"This declaration shadows an existing declaration.",
				SecondarySourceLocation().append("The shadowed declaration is here:", shadowedLocation)
			);
		}
	}
	return true;
}

bool DeclarationRegistrationHelper::visit(SourceUnit& _sourceUnit)
{
	if (!m_scopes[&_sourceUnit])
		// By importing, it is possible that the container already exists.
		m_scopes[&_sourceUnit].reset(new DeclarationContainer(m_currentScope, m_scopes[m_currentScope].get()));
	m_currentScope = &_sourceUnit;
	return true;
}

void DeclarationRegistrationHelper::endVisit(SourceUnit& _sourceUnit)
{
	_sourceUnit.annotation().exportedSymbols = m_scopes[&_sourceUnit]->declarations();
	closeCurrentScope();
}

bool DeclarationRegistrationHelper::visit(ImportDirective& _import)
{
	SourceUnit const* importee = _import.annotation().sourceUnit;
	solAssert(!!importee, "");
	if (!m_scopes[importee])
		m_scopes[importee].reset(new DeclarationContainer(nullptr, m_scopes[nullptr].get()));
	m_scopes[&_import] = m_scopes[importee];
	registerDeclaration(_import, false);
	return true;
}

bool DeclarationRegistrationHelper::visit(ContractDefinition& _contract)
{
	registerDeclaration(_contract, true);
	_contract.annotation().canonicalName = currentCanonicalName();
	return true;
}

void DeclarationRegistrationHelper::endVisit(ContractDefinition&)
{
	closeCurrentScope();
}

bool DeclarationRegistrationHelper::visit(StructDefinition& _struct)
{
	registerDeclaration(_struct, true);
	_struct.annotation().canonicalName = currentCanonicalName();
	return true;
}

void DeclarationRegistrationHelper::endVisit(StructDefinition&)
{
	closeCurrentScope();
}

bool DeclarationRegistrationHelper::visit(EnumDefinition& _enum)
{
	registerDeclaration(_enum, true);
	_enum.annotation().canonicalName = currentCanonicalName();
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

bool DeclarationRegistrationHelper::visit(Block& _block)
{
	_block.setScope(m_currentScope);
	enterNewSubScope(_block);
	return true;
}

void DeclarationRegistrationHelper::endVisit(Block&)
{
	closeCurrentScope();
}

bool DeclarationRegistrationHelper::visit(ForStatement& _for)
{
	_for.setScope(m_currentScope);
	enterNewSubScope(_for);
	return true;
}

void DeclarationRegistrationHelper::endVisit(ForStatement&)
{
	closeCurrentScope();
}

void DeclarationRegistrationHelper::endVisit(VariableDeclarationStatement& _variableDeclarationStatement)
{
	// Register the local variables with the function
	// This does not fit here perfectly, but it saves us another AST visit.
	solAssert(m_currentFunction, "Variable declaration without function.");
	for (ASTPointer<VariableDeclaration> const& var: _variableDeclarationStatement.declarations())
		if (var)
			m_currentFunction->addLocalVariable(*var);
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

void DeclarationRegistrationHelper::enterNewSubScope(ASTNode& _subScope)
{
	map<ASTNode const*, shared_ptr<DeclarationContainer>>::iterator iter;
	bool newlyAdded;
	shared_ptr<DeclarationContainer> container(new DeclarationContainer(m_currentScope, m_scopes[m_currentScope].get()));
	tie(iter, newlyAdded) = m_scopes.emplace(&_subScope, move(container));
	solAssert(newlyAdded, "Unable to add new scope.");
	m_currentScope = &_subScope;
}

void DeclarationRegistrationHelper::closeCurrentScope()
{
	solAssert(m_currentScope && m_scopes.count(m_currentScope), "Closed non-existing scope.");
	m_currentScope = m_scopes[m_currentScope]->enclosingNode();
}

void DeclarationRegistrationHelper::registerDeclaration(Declaration& _declaration, bool _opensScope)
{
	solAssert(m_currentScope && m_scopes.count(m_currentScope), "No current scope.");

	bool warnAboutShadowing = true;
	// Do not warn about shadowing for structs and enums because their members are
	// not accessible without prefixes. Also do not warn about event parameters
	// because they don't participate in any proper scope.
	if (
		dynamic_cast<StructDefinition const*>(m_currentScope) ||
		dynamic_cast<EnumDefinition const*>(m_currentScope) ||
		dynamic_cast<EventDefinition const*>(m_currentScope)
	)
		warnAboutShadowing = false;
	// Do not warn about the constructor shadowing the contract.
	if (auto fun = dynamic_cast<FunctionDefinition const*>(&_declaration))
		if (fun->isConstructor())
			warnAboutShadowing = false;

	// Register declaration as inactive if we are in block scope.
	bool inactive =
		(dynamic_cast<Block const*>(m_currentScope) || dynamic_cast<ForStatement const*>(m_currentScope));

	registerDeclaration(*m_scopes[m_currentScope], _declaration, nullptr, nullptr, warnAboutShadowing, inactive, m_errorReporter);

	_declaration.setScope(m_currentScope);
	if (_opensScope)
		enterNewSubScope(_declaration);
}

string DeclarationRegistrationHelper::currentCanonicalName() const
{
	string ret;
	for (
		ASTNode const* scope = m_currentScope;
		scope != nullptr;
		scope = m_scopes[scope]->enclosingNode()
	)
	{
		if (auto decl = dynamic_cast<Declaration const*>(scope))
		{
			if (!ret.empty())
				ret = "." + ret;
			ret = decl->name() + ret;
		}
	}
	return ret;
}

}
}
