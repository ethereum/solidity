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
#include <libsolidity/interface/Exceptions.h>

using namespace std;

namespace dev
{
namespace solidity
{

NameAndTypeResolver::NameAndTypeResolver(
	vector<Declaration const*> const& _globals,
	ErrorList& _errors
) :
	m_errors(_errors)
{
	if (!m_scopes[nullptr])
		m_scopes[nullptr].reset(new DeclarationContainer());
	for (Declaration const* declaration: _globals)
		m_scopes[nullptr]->registerDeclaration(*declaration);
}

bool NameAndTypeResolver::registerDeclarations(SourceUnit& _sourceUnit)
{
	if (!m_scopes[&_sourceUnit])
		// By importing, it is possible that the container already exists.
		m_scopes[&_sourceUnit].reset(new DeclarationContainer(nullptr, m_scopes[nullptr].get()));
	m_currentScope = m_scopes[&_sourceUnit].get();

	// The helper registers all declarations in m_scopes as a side-effect of its construction.
	try
	{
		DeclarationRegistrationHelper registrar(m_scopes, _sourceUnit, m_errors);
		_sourceUnit.annotation().exportedSymbols = m_scopes[&_sourceUnit]->declarations();
	}
	catch (FatalError const&)
	{
		if (m_errors.empty())
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
				reportDeclarationError(
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
						reportDeclarationError(
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
						{
							ASTString const* name = alias.second ? alias.second.get() : &declaration->name();
							if (!target.registerDeclaration(*declaration, name))
							{
								reportDeclarationError(
									imp->location(),
									"Identifier \"" + *name + "\" already declared."
								);
								error = true;
							}
						}
				}
			else if (imp->name().empty())
				for (auto const& nameAndDeclaration: scope->second->declarations())
					for (auto const& declaration: nameAndDeclaration.second)
						if (!target.registerDeclaration(*declaration, &nameAndDeclaration.first))
						{
							reportDeclarationError(
								imp->location(),
								"Identifier \"" + nameAndDeclaration.first + "\" already declared."
							);
							error = true;
						}
		}
	return !error;
}

bool NameAndTypeResolver::resolveNamesAndTypes(ContractDefinition& _contract)
{
	try
	{
		m_currentScope = m_scopes[_contract.scope()].get();
		solAssert(!!m_currentScope, "");

		ReferencesResolver resolver(m_errors, *this, nullptr);
		bool success = true;
		for (ASTPointer<InheritanceSpecifier> const& baseContract: _contract.baseContracts())
			if (!resolver.resolve(*baseContract))
				success = false;

		m_currentScope = m_scopes[&_contract].get();

		if (success)
		{
			linearizeBaseContracts(_contract);
			vector<ContractDefinition const*> properBases(
				++_contract.annotation().linearizedBaseContracts.begin(),
				_contract.annotation().linearizedBaseContracts.end()
			);

			for (ContractDefinition const* base: properBases)
				importInheritedScope(*base);
		}

		// these can contain code, only resolve parameters for now
		for (ASTPointer<ASTNode> const& node: _contract.subNodes())
		{
			m_currentScope = m_scopes[m_scopes.count(node.get()) ? node.get() : &_contract].get();
			if (!resolver.resolve(*node))
				success = false;
		}

		if (!success)
			return false;

		m_currentScope = m_scopes[&_contract].get();

		// now resolve references inside the code
		for (ModifierDefinition const* modifier: _contract.functionModifiers())
		{
			m_currentScope = m_scopes[modifier].get();
			ReferencesResolver resolver(m_errors, *this, nullptr, true);
			if (!resolver.resolve(*modifier))
				success = false;
		}

		for (FunctionDefinition const* function: _contract.definedFunctions())
		{
			m_currentScope = m_scopes[function].get();
			if (!ReferencesResolver(
				m_errors,
				*this,
				function->returnParameterList().get(),
				true
			).resolve(*function))
				success = false;
		}
		if (!success)
			return false;
	}
	catch (FatalError const&)
	{
		if (m_errors.empty())
			throw; // Something is weird here, rather throw again.
		return false;
	}
	return true;
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
		if (m_errors.empty())
			throw; // Something is weird here, rather throw again.
		return false;
	}
	return true;
}

vector<Declaration const*> NameAndTypeResolver::resolveName(ASTString const& _name, ASTNode const* _scope) const
{
	auto iterator = m_scopes.find(_scope);
	if (iterator == end(m_scopes))
		return vector<Declaration const*>({});
	return iterator->second->resolveName(_name, false);
}

vector<Declaration const*> NameAndTypeResolver::nameFromCurrentScope(ASTString const& _name, bool _recursive) const
{
	return m_currentScope->resolveName(_name, _recursive);
}

Declaration const* NameAndTypeResolver::pathFromCurrentScope(vector<ASTString> const& _path, bool _recursive) const
{
	solAssert(!_path.empty(), "");
	vector<Declaration const*> candidates = m_currentScope->resolveName(_path.front(), _recursive);
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

	for (auto it = _declarations.begin(); it != _declarations.end(); ++it)
	{
		solAssert(*it, "");
		// the declaration is functionDefinition or a VariableDeclaration while declarations > 1
		solAssert(dynamic_cast<FunctionDefinition const*>(*it) || dynamic_cast<VariableDeclaration const*>(*it),
			"Found overloading involving something not a function or a variable");

		unique_ptr<FunctionType const> functionType {};

		if (FunctionDefinition const* functionDefinition = dynamic_cast<FunctionDefinition const*>(*it))
		{
			functionType = unique_ptr<FunctionType const>(new FunctionType(*functionDefinition));
			for (auto parameter: functionType->parameterTypes() + functionType->returnParameterTypes())
				if (!parameter)
					reportFatalDeclarationError(_identifier.location(), "Function type can not be used in this context");
		}
		else
		{
			VariableDeclaration const* variableDeclaration = dynamic_cast<VariableDeclaration const*>(*it);
			functionType = unique_ptr<FunctionType const>(new FunctionType(*variableDeclaration));
		}
		solAssert(functionType, "failed to determine the function type of the overloaded");

		if (uniqueFunctions.end() == find_if(
			uniqueFunctions.begin(),
			uniqueFunctions.end(),
			[&](Declaration const* d)
			{
				if (FunctionDefinition const* functionDefinition = dynamic_cast<FunctionDefinition const*>(d))
				{
					FunctionType const newFunctionType(*functionDefinition);
					return functionType->hasEqualArgumentTypes(newFunctionType);
				}
				else if (VariableDeclaration const* variableDeclaration = dynamic_cast<VariableDeclaration const*>(d))
				{
					FunctionType const newFunctionType(*variableDeclaration);
					return functionType->hasEqualArgumentTypes(newFunctionType);
				}
				return false; // to make compiler happy
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

					reportDeclarationError(
						secondDeclarationLocation,
						"Identifier already declared.",
						firstDeclarationLocation,
						"The previous declaration is here:"
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
			reportFatalTypeError(baseName.createTypeError("Contract expected."));
		// "push_front" has the effect that bases mentioned later can overwrite members of bases
		// mentioned earlier
		input.back().push_front(base);
		vector<ContractDefinition const*> const& basesBases = base->annotation().linearizedBaseContracts;
		if (basesBases.empty())
			reportFatalTypeError(baseName.createTypeError("Definition of base has to precede definition of derived contract"));
		input.push_front(list<ContractDefinition const*>(basesBases.begin(), basesBases.end()));
	}
	input.back().push_front(&_contract);
	vector<ContractDefinition const*> result = cThreeMerge(input);
	if (result.empty())
		reportFatalTypeError(_contract.createTypeError("Linearization of inheritance graph impossible"));
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

void NameAndTypeResolver::reportDeclarationError(
	SourceLocation _sourceLoction,
	string const& _description,
	SourceLocation _secondarySourceLocation,
	string const& _secondaryDescription
)
{
	auto err = make_shared<Error>(Error::Type::DeclarationError); // todo remove Error?
	*err <<
		errinfo_sourceLocation(_sourceLoction) <<
		errinfo_comment(_description) <<
		errinfo_secondarySourceLocation(
			SecondarySourceLocation().append(_secondaryDescription, _secondarySourceLocation)
		);

	m_errors.push_back(err);
}

void NameAndTypeResolver::reportDeclarationError(SourceLocation _sourceLocation, string const& _description)
{
	auto err = make_shared<Error>(Error::Type::DeclarationError); // todo remove Error?
	*err <<	errinfo_sourceLocation(_sourceLocation) << errinfo_comment(_description);

	m_errors.push_back(err);
}

void NameAndTypeResolver::reportFatalDeclarationError(
	SourceLocation _sourceLocation,
	string const& _description
)
{
	reportDeclarationError(_sourceLocation, _description);
	BOOST_THROW_EXCEPTION(FatalError());
}

void NameAndTypeResolver::reportTypeError(Error const& _e)
{
	m_errors.push_back(make_shared<Error>(_e));
}

void NameAndTypeResolver::reportFatalTypeError(Error const& _e)
{
	reportTypeError(_e);
	BOOST_THROW_EXCEPTION(FatalError());
}

DeclarationRegistrationHelper::DeclarationRegistrationHelper(
	map<ASTNode const*, shared_ptr<DeclarationContainer>>& _scopes,
	ASTNode& _astRoot,
	ErrorList& _errors
):
	m_scopes(_scopes),
	m_currentScope(&_astRoot),
	m_errors(_errors)
{
	solAssert(!!m_scopes.at(m_currentScope), "");
	_astRoot.accept(*this);
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

void DeclarationRegistrationHelper::enterNewSubScope(Declaration const& _declaration)
{
	map<ASTNode const*, shared_ptr<DeclarationContainer>>::iterator iter;
	bool newlyAdded;
	shared_ptr<DeclarationContainer> container(new DeclarationContainer(m_currentScope, m_scopes[m_currentScope].get()));
	tie(iter, newlyAdded) = m_scopes.emplace(&_declaration, move(container));
	solAssert(newlyAdded, "Unable to add new scope.");
	m_currentScope = &_declaration;
}

void DeclarationRegistrationHelper::closeCurrentScope()
{
	solAssert(m_currentScope, "Closed non-existing scope.");
	m_currentScope = m_scopes[m_currentScope]->enclosingNode();
}

void DeclarationRegistrationHelper::registerDeclaration(Declaration& _declaration, bool _opensScope)
{
	if (!m_scopes[m_currentScope]->registerDeclaration(_declaration, nullptr, !_declaration.isVisibleInContract()))
	{
		SourceLocation firstDeclarationLocation;
		SourceLocation secondDeclarationLocation;
		Declaration const* conflictingDeclaration = m_scopes[m_currentScope]->conflictingDeclaration(_declaration);
		solAssert(conflictingDeclaration, "");

		if (_declaration.location().start < conflictingDeclaration->location().start)
		{
			firstDeclarationLocation = _declaration.location();
			secondDeclarationLocation = conflictingDeclaration->location();
		}
		else
		{
			firstDeclarationLocation = conflictingDeclaration->location();
			secondDeclarationLocation = _declaration.location();
		}

		declarationError(
			secondDeclarationLocation,
			"Identifier already declared.",
			firstDeclarationLocation,
			"The previous declaration is here:"
		);
	}

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

void DeclarationRegistrationHelper::declarationError(
	SourceLocation _sourceLocation,
	string const& _description,
	SourceLocation _secondarySourceLocation,
	string const& _secondaryDescription
)
{
	auto err = make_shared<Error>(Error::Type::DeclarationError);
	*err <<
		errinfo_sourceLocation(_sourceLocation) <<
		errinfo_comment(_description) <<
		errinfo_secondarySourceLocation(
			SecondarySourceLocation().append(_secondaryDescription, _secondarySourceLocation)
		);

	m_errors.push_back(err);
}

void DeclarationRegistrationHelper::declarationError(SourceLocation _sourceLocation, string const& _description)
{
	auto err = make_shared<Error>(Error::Type::DeclarationError);
	*err <<	errinfo_sourceLocation(_sourceLocation) << errinfo_comment(_description);

	m_errors.push_back(err);
}

void DeclarationRegistrationHelper::fatalDeclarationError(
	SourceLocation _sourceLocation,
	string const& _description
)
{
	declarationError(_sourceLocation, _description);
	BOOST_THROW_EXCEPTION(FatalError());
}

}
}
