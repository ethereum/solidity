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
 * @date 2014
 * Parser part that determines the declarations corresponding to names and the types of expressions.
 */

#include <libsolidity/analysis/NameAndTypeResolver.h>

#include <libsolidity/analysis/TypeChecker.h>
#include <libsolidity/ast/AST.h>
#include <liblangutil/ErrorReporter.h>
#include <libsolutil/StringUtils.h>
#include <boost/algorithm/string.hpp>
#include <unordered_set>

using namespace std;
using namespace solidity::langutil;

namespace solidity::frontend
{

NameAndTypeResolver::NameAndTypeResolver(
	GlobalContext& _globalContext,
	langutil::EVMVersion _evmVersion,
	ErrorReporter& _errorReporter
):
	m_evmVersion(_evmVersion),
	m_errorReporter(_errorReporter),
	m_globalContext(_globalContext)
{
	m_scopes[nullptr] = make_shared<DeclarationContainer>();
	for (Declaration const* declaration: _globalContext.declarations())
	{
		solAssert(m_scopes[nullptr]->registerDeclaration(*declaration, false, false), "Unable to register global declaration.");
	}
}

bool NameAndTypeResolver::registerDeclarations(SourceUnit& _sourceUnit, ASTNode const* _currentScope)
{
	// The helper registers all declarations in m_scopes as a side-effect of its construction.
	try
	{
		DeclarationRegistrationHelper registrar(m_scopes, _sourceUnit, m_errorReporter, m_globalContext, _currentScope);
	}
	catch (langutil::FatalError const&)
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
			string const& path = *imp->annotation().absolutePath;
			// The import resolution in CompilerStack enforces this.
			solAssert(_sourceUnits.count(path), "");
			auto scope = m_scopes.find(_sourceUnits.at(path));
			solAssert(scope != end(m_scopes), "");
			if (!imp->symbolAliases().empty())
				for (auto const& alias: imp->symbolAliases())
				{
					auto declarations = scope->second->resolveName(alias.symbol->name(), false);
					if (declarations.empty())
					{
						m_errorReporter.declarationError(
							2904_error,
							imp->location(),
							"Declaration \"" +
							alias.symbol->name() +
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
								target,
								*declaration,
								alias.alias ? alias.alias.get() : &alias.symbol->name(),
								&alias.location,
								false,
								m_errorReporter
							))
								error = true;
				}
			else if (imp->name().empty())
				for (auto const& nameAndDeclaration: scope->second->declarations())
					for (auto const& declaration: nameAndDeclaration.second)
						if (!DeclarationRegistrationHelper::registerDeclaration(
							target, *declaration, &nameAndDeclaration.first, &imp->location(), false, m_errorReporter
						))
							error =  true;
		}
	_sourceUnit.annotation().exportedSymbols = m_scopes[&_sourceUnit]->declarations();
	return !error;
}

bool NameAndTypeResolver::resolveNamesAndTypes(SourceUnit& _source)
{
	try
	{
		for (shared_ptr<ASTNode> const& node: _source.nodes())
		{
			setScope(&_source);
			if (!resolveNamesAndTypesInternal(*node, true))
				return false;
		}
	}
	catch (langutil::FatalError const&)
	{
		if (m_errorReporter.errors().empty())
			throw; // Something is weird here, rather throw again.
		return false;
	}
	return true;
}

bool NameAndTypeResolver::updateDeclaration(Declaration const& _declaration)
{
	try
	{
		m_scopes[nullptr]->registerDeclaration(_declaration, false, true);
		solAssert(_declaration.scope() == nullptr, "Updated declaration outside global scope.");
	}
	catch (langutil::FatalError const&)
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
	vector<Declaration const*> candidates = m_currentScope->resolveName(
		_path.front(),
		/* _recursive */ true,
		/* _alsoInvisible */ false,
		/* _onlyVisibleAsUnqualifiedNames */ true
	);

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

void NameAndTypeResolver::warnHomonymDeclarations() const
{
	DeclarationContainer::Homonyms homonyms;
	m_scopes.at(nullptr)->populateHomonyms(back_inserter(homonyms));

	for (auto [innerLocation, outerDeclarations]: homonyms)
	{
		solAssert(innerLocation && !outerDeclarations.empty(), "");

		bool magicShadowed = false;
		SecondarySourceLocation homonymousLocations;
		SecondarySourceLocation shadowedLocations;
		for (Declaration const* outerDeclaration: outerDeclarations)
		{
			solAssert(outerDeclaration, "");
			if (dynamic_cast<MagicVariableDeclaration const*>(outerDeclaration))
				magicShadowed = true;
			else if (!outerDeclaration->isVisibleInContract())
				homonymousLocations.append("The other declaration is here:", outerDeclaration->location());
			else
				shadowedLocations.append("The shadowed declaration is here:", outerDeclaration->location());
		}

		if (magicShadowed)
			m_errorReporter.warning(
				2319_error,
				*innerLocation,
				"This declaration shadows a builtin symbol."
			);
		if (!homonymousLocations.infos.empty())
			m_errorReporter.warning(
				8760_error,
				*innerLocation,
				"This declaration has the same name as another declaration.",
				homonymousLocations
			);
		if (!shadowedLocations.infos.empty())
			m_errorReporter.warning(
				2519_error,
				*innerLocation,
				"This declaration shadows an existing declaration.",
				shadowedLocations
			);
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
		solAssert(_resolveInsideCode, "");

		m_globalContext.setCurrentContract(*contract);
		if (!contract->isLibrary())
			updateDeclaration(*m_globalContext.currentSuper());
		updateDeclaration(*m_globalContext.currentThis());

		for (ASTPointer<InheritanceSpecifier> const& baseContract: contract->baseContracts())
			if (!resolveNamesAndTypesInternal(*baseContract, true))
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
			if (!resolveNamesAndTypesInternal(*node, false))
				success = false;
		}

		if (!success)
			return false;

		setScope(contract);

		// now resolve references inside the code
		for (ASTPointer<ASTNode> const& node: contract->subNodes())
		{
			setScope(contract);
			if (!resolveNamesAndTypesInternal(*node, true))
				success = false;
		}

		// make "this" and "super" invisible.
		m_scopes[nullptr]->registerDeclaration(*m_globalContext.currentThis(), true, true);
		m_scopes[nullptr]->registerDeclaration(*m_globalContext.currentSuper(), true, true);
		m_globalContext.resetCurrentContract();

		return success;
	}
	else
	{
		if (m_scopes.count(&_node))
			setScope(&_node);
		return ReferencesResolver(m_errorReporter, *this, m_evmVersion, _resolveInsideCode).resolve(_node);
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
				if (!m_currentScope->registerDeclaration(*declaration, false, false))
				{
					SourceLocation firstDeclarationLocation;
					SourceLocation secondDeclarationLocation;
					Declaration const* conflictingDeclaration = m_currentScope->conflictingDeclaration(*declaration);
					solAssert(conflictingDeclaration, "");

					// Usual shadowing is not an error
					if (
						dynamic_cast<ModifierDefinition const*>(declaration) &&
						dynamic_cast<ModifierDefinition const*>(conflictingDeclaration)
					)
						continue;

					// Public state variable can override functions
					if (auto varDecl = dynamic_cast<VariableDeclaration const*>(conflictingDeclaration))
						if (
							dynamic_cast<FunctionDefinition const*>(declaration) &&
							varDecl->isStateVariable() &&
							varDecl->isPublic()
						)
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
						9097_error,
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
		IdentifierPath const& baseName = baseSpecifier->name();
		auto base = dynamic_cast<ContractDefinition const*>(baseName.annotation().referencedDeclaration);
		if (!base)
			m_errorReporter.fatalTypeError(8758_error, baseName.location(), "Contract expected.");
		// "push_front" has the effect that bases mentioned later can overwrite members of bases
		// mentioned earlier
		input.back().push_front(base);
		vector<ContractDefinition const*> const& basesBases = base->annotation().linearizedBaseContracts;
		if (basesBases.empty())
			m_errorReporter.fatalTypeError(2449_error, baseName.location(), "Definition of base has to precede definition of derived contract");
		input.push_front(list<ContractDefinition const*>(basesBases.begin(), basesBases.end()));
	}
	input.back().push_front(&_contract);
	vector<ContractDefinition const*> result = cThreeMerge(input);
	if (result.empty())
		m_errorReporter.fatalTypeError(5005_error, _contract.location(), "Linearization of inheritance graph impossible");
	_contract.annotation().linearizedBaseContracts = result;
}

template <class T>
vector<T const*> NameAndTypeResolver::cThreeMerge(list<list<T const*>>& _toMerge)
{
	// returns true iff _candidate appears only as last element of the lists
	auto appearsOnlyAtHead = [&](T const* _candidate) -> bool
	{
		for (list<T const*> const& bases: _toMerge)
		{
			solAssert(!bases.empty(), "");
			if (find(++bases.begin(), bases.end(), _candidate) != bases.end())
				return false;
		}
		return true;
	};
	// returns the next candidate to append to the linearized list or nullptr on failure
	auto nextCandidate = [&]() -> T const*
	{
		for (list<T const*> const& bases: _toMerge)
		{
			solAssert(!bases.empty(), "");
			if (appearsOnlyAtHead(bases.front()))
				return bases.front();
		}
		return nullptr;
	};
	// removes the given contract from all lists
	auto removeCandidate = [&](T const* _candidate)
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

	_toMerge.remove_if([](list<T const*> const& _bases) { return _bases.empty(); });
	vector<T const*> result;
	while (!_toMerge.empty())
	{
		T const* candidate = nextCandidate();
		if (!candidate)
			return vector<T const*>();
		result.push_back(candidate);
		removeCandidate(candidate);
	}
	return result;
}

string NameAndTypeResolver::similarNameSuggestions(ASTString const& _name) const
{
	return util::quotedAlternativesList(m_currentScope->similarNames(_name));
}

DeclarationRegistrationHelper::DeclarationRegistrationHelper(
	map<ASTNode const*, shared_ptr<DeclarationContainer>>& _scopes,
	ASTNode& _astRoot,
	ErrorReporter& _errorReporter,
	GlobalContext& _globalContext,
	ASTNode const* _currentScope
):
	m_scopes(_scopes),
	m_currentScope(_currentScope),
	m_errorReporter(_errorReporter),
	m_globalContext(_globalContext)
{
	_astRoot.accept(*this);
	solAssert(m_currentScope == _currentScope, "Scopes not correctly closed.");
}

bool DeclarationRegistrationHelper::registerDeclaration(
	DeclarationContainer& _container,
	Declaration const& _declaration,
	string const* _name,
	SourceLocation const* _errorLocation,
	bool _inactive,
	ErrorReporter& _errorReporter
)
{
	if (!_errorLocation)
		_errorLocation = &_declaration.location();

	string name = _name ? *_name : _declaration.name();

	// We use "invisible" for both inactive variables in blocks and for members invisible in contracts.
	// They cannot both be true at the same time.
	solAssert(!(_inactive && !_declaration.isVisibleInContract()), "");

	static set<string> illegalNames{"_", "super", "this"};

	if (illegalNames.count(name))
	{
		auto isPublicFunctionOrEvent = [](Declaration const* _d) -> bool
		{
			if (auto functionDefinition = dynamic_cast<FunctionDefinition const*>(_d))
			{
				if (!functionDefinition->isFree() && functionDefinition->isPublic())
					return true;
			}
			else if (dynamic_cast<EventDefinition const*>(_d))
				return true;

			return false;
		};

		// We allow an exception for public functions or events.
		if (!isPublicFunctionOrEvent(&_declaration))
			_errorReporter.declarationError(
				3726_error,
				*_errorLocation,
				"The name \"" + name + "\" is reserved."
			);
	}

	if (!_container.registerDeclaration(_declaration, _name, _errorLocation, !_declaration.isVisibleInContract() || _inactive, false))
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
			2333_error,
			secondDeclarationLocation,
			SecondarySourceLocation().append("The previous declaration is here:", firstDeclarationLocation),
			"Identifier already declared."
		);
		return false;
	}

	return true;
}

bool DeclarationRegistrationHelper::visit(SourceUnit& _sourceUnit)
{
	if (!m_scopes[&_sourceUnit])
		// By importing, it is possible that the container already exists.
		m_scopes[&_sourceUnit] = make_shared<DeclarationContainer>(m_currentScope, m_scopes[m_currentScope].get());
	return ASTVisitor::visit(_sourceUnit);
}

void DeclarationRegistrationHelper::endVisit(SourceUnit& _sourceUnit)
{
	ASTVisitor::endVisit(_sourceUnit);
}

bool DeclarationRegistrationHelper::visit(ImportDirective& _import)
{
	SourceUnit const* importee = _import.annotation().sourceUnit;
	solAssert(!!importee, "");
	if (!m_scopes[importee])
		m_scopes[importee] = make_shared<DeclarationContainer>(nullptr, m_scopes[nullptr].get());
	m_scopes[&_import] = m_scopes[importee];
	return ASTVisitor::visit(_import);
}

bool DeclarationRegistrationHelper::visit(ContractDefinition& _contract)
{
	m_globalContext.setCurrentContract(_contract);
	m_scopes[nullptr]->registerDeclaration(*m_globalContext.currentThis(), false, true);
	m_scopes[nullptr]->registerDeclaration(*m_globalContext.currentSuper(), false, true);
	m_currentContract = &_contract;

	return ASTVisitor::visit(_contract);
}

void DeclarationRegistrationHelper::endVisit(ContractDefinition& _contract)
{
	// make "this" and "super" invisible.
	m_scopes[nullptr]->registerDeclaration(*m_globalContext.currentThis(), true, true);
	m_scopes[nullptr]->registerDeclaration(*m_globalContext.currentSuper(), true, true);
	m_globalContext.resetCurrentContract();
	m_currentContract = nullptr;
	ASTVisitor::endVisit(_contract);
}

void DeclarationRegistrationHelper::endVisit(VariableDeclarationStatement& _variableDeclarationStatement)
{
	// Register the local variables with the function
	// This does not fit here perfectly, but it saves us another AST visit.
	solAssert(m_currentFunction, "Variable declaration without function.");
	for (ASTPointer<VariableDeclaration> const& var: _variableDeclarationStatement.declarations())
		if (var)
			m_currentFunction->addLocalVariable(*var);
	ASTVisitor::endVisit(_variableDeclarationStatement);
}

bool DeclarationRegistrationHelper::visitNode(ASTNode& _node)
{
	if (auto const* scopable = dynamic_cast<Scopable const*>(&_node))
		solAssert(scopable->annotation().scope == m_currentScope, "");

	if (auto* declaration = dynamic_cast<Declaration*>(&_node))
		registerDeclaration(*declaration);

	if (auto* annotation = dynamic_cast<TypeDeclarationAnnotation*>(&_node.annotation()))
	{
		string canonicalName = dynamic_cast<Declaration const&>(_node).name();
		solAssert(!canonicalName.empty(), "");

		for (
			ASTNode const* scope = m_currentScope;
			scope != nullptr;
			scope = m_scopes[scope]->enclosingNode()
		)
			if (auto decl = dynamic_cast<Declaration const*>(scope))
			{
				solAssert(!decl->name().empty(), "");
				canonicalName = decl->name() + "." + canonicalName;
			}

		annotation->canonicalName = canonicalName;
	}

	if (dynamic_cast<ScopeOpener const*>(&_node))
		enterNewSubScope(_node);

	if (auto* variableScope = dynamic_cast<VariableScope*>(&_node))
		m_currentFunction = variableScope;

	return true;
}

void DeclarationRegistrationHelper::endVisitNode(ASTNode& _node)
{
	if (dynamic_cast<ScopeOpener const*>(&_node))
		closeCurrentScope();
	if (dynamic_cast<VariableScope*>(&_node))
		m_currentFunction = nullptr;
}

void DeclarationRegistrationHelper::enterNewSubScope(ASTNode& _subScope)
{
	if (m_scopes.count(&_subScope))
		// Source units are the only AST nodes for which containers can be created from multiple places due to imports.
		solAssert(dynamic_cast<SourceUnit const*>(&_subScope), "Unexpected scope type.");
	else
	{
		bool newlyAdded = m_scopes.emplace(
			&_subScope,
			make_shared<DeclarationContainer>(m_currentScope, m_scopes[m_currentScope].get())
		).second;
		solAssert(newlyAdded, "Unable to add new scope.");
	}
	m_currentScope = &_subScope;
}

void DeclarationRegistrationHelper::closeCurrentScope()
{
	solAssert(m_currentScope && m_scopes.count(m_currentScope), "Closed non-existing scope.");
	m_currentScope = m_scopes[m_currentScope]->enclosingNode();
}

void DeclarationRegistrationHelper::registerDeclaration(Declaration& _declaration)
{
	solAssert(m_currentScope && m_scopes.count(m_currentScope), "No current scope.");
	solAssert(m_currentScope == _declaration.scope(), "Unexpected current scope.");

	// Register declaration as inactive if we are in block scope.
	bool inactive =
		(dynamic_cast<Block const*>(m_currentScope) || dynamic_cast<ForStatement const*>(m_currentScope));

	registerDeclaration(*m_scopes[m_currentScope], _declaration, nullptr, nullptr, inactive, m_errorReporter);

	solAssert(_declaration.annotation().scope == m_currentScope, "");
	solAssert(_declaration.annotation().contract == m_currentContract, "");
}

}
