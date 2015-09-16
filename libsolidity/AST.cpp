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
 * Solidity abstract syntax tree.
 */

#include <algorithm>
#include <functional>
#include <libsolidity/Utils.h>
#include <libsolidity/AST.h>
#include <libsolidity/ASTVisitor.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/AST_accept.h>

#include <libdevcore/SHA3.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

ASTNode::ASTNode(SourceLocation const& _location):
	m_location(_location)
{
}

ASTNode::~ASTNode()
{
}

TypeError ASTNode::createTypeError(string const& _description) const
{
	return TypeError() << errinfo_sourceLocation(location()) << errinfo_comment(_description);
}

map<FixedHash<4>, FunctionTypePointer> ContractDefinition::interfaceFunctions() const
{
	auto exportedFunctionList = interfaceFunctionList();

	map<FixedHash<4>, FunctionTypePointer> exportedFunctions;
	for (auto const& it: exportedFunctionList)
		exportedFunctions.insert(it);

	solAssert(
		exportedFunctionList.size() == exportedFunctions.size(),
		"Hash collision at Function Definition Hash calculation"
	);

	return exportedFunctions;
}

FunctionDefinition const* ContractDefinition::constructor() const
{
	for (ASTPointer<FunctionDefinition> const& f: m_definedFunctions)
		if (f->isConstructor())
			return f.get();
	return nullptr;
}

FunctionDefinition const* ContractDefinition::fallbackFunction() const
{
	for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
		for (ASTPointer<FunctionDefinition> const& f: contract->definedFunctions())
			if (f->name().empty())
				return f.get();
	return nullptr;
}

vector<ASTPointer<EventDefinition>> const& ContractDefinition::interfaceEvents() const
{
	if (!m_interfaceEvents)
	{
		set<string> eventsSeen;
		m_interfaceEvents.reset(new vector<ASTPointer<EventDefinition>>());
		for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
			for (ASTPointer<EventDefinition> const& e: contract->events())
				if (eventsSeen.count(e->name()) == 0)
				{
					eventsSeen.insert(e->name());
					m_interfaceEvents->push_back(e);
				}
	}
	return *m_interfaceEvents;
}

vector<pair<FixedHash<4>, FunctionTypePointer>> const& ContractDefinition::interfaceFunctionList() const
{
	if (!m_interfaceFunctionList)
	{
		set<string> functionsSeen;
		set<string> signaturesSeen;
		m_interfaceFunctionList.reset(new vector<pair<FixedHash<4>, FunctionTypePointer>>());
		for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
		{
			for (ASTPointer<FunctionDefinition> const& f: contract->definedFunctions())
			{
				if (!f->isPartOfExternalInterface())
					continue;
				string functionSignature = f->externalSignature();
				if (signaturesSeen.count(functionSignature) == 0)
				{
					functionsSeen.insert(f->name());
					signaturesSeen.insert(functionSignature);
					FixedHash<4> hash(dev::sha3(functionSignature));
					m_interfaceFunctionList->push_back(make_pair(hash, make_shared<FunctionType>(*f, false)));
				}
			}

			for (ASTPointer<VariableDeclaration> const& v: contract->stateVariables())
				if (functionsSeen.count(v->name()) == 0 && v->isPartOfExternalInterface())
				{
					FunctionType ftype(*v);
					solAssert(!!v->annotation().type.get(), "");
					functionsSeen.insert(v->name());
					FixedHash<4> hash(dev::sha3(ftype.externalSignature(v->name())));
					m_interfaceFunctionList->push_back(make_pair(hash, make_shared<FunctionType>(*v)));
				}
		}
	}
	return *m_interfaceFunctionList;
}

string const& ContractDefinition::devDocumentation() const
{
	return m_devDocumentation;
}

string const& ContractDefinition::userDocumentation() const
{
	return m_userDocumentation;
}

void ContractDefinition::setDevDocumentation(string const& _devDocumentation)
{
	m_devDocumentation = _devDocumentation;
}

void ContractDefinition::setUserDocumentation(string const& _userDocumentation)
{
	m_userDocumentation = _userDocumentation;
}


vector<Declaration const*> const& ContractDefinition::inheritableMembers() const
{
	if (!m_inheritableMembers)
	{
		set<string> memberSeen;
		m_inheritableMembers.reset(new vector<Declaration const*>());
		auto addInheritableMember = [&](Declaration const* _decl)
		{
			if (memberSeen.count(_decl->name()) == 0 && _decl->isVisibleInDerivedContracts())
			{
				memberSeen.insert(_decl->name());
				m_inheritableMembers->push_back(_decl);
			}
		};

		for (ASTPointer<FunctionDefinition> const& f: definedFunctions())
			addInheritableMember(f.get());

		for (ASTPointer<VariableDeclaration> const& v: stateVariables())
			addInheritableMember(v.get());

		for (ASTPointer<StructDefinition> const& s: definedStructs())
			addInheritableMember(s.get());
	}
	return *m_inheritableMembers;
}

TypePointer ContractDefinition::type(ContractDefinition const* m_currentContract) const
{
	return make_shared<TypeType>(make_shared<ContractType>(*this), m_currentContract);
}

TypePointer StructDefinition::type(ContractDefinition const*) const
{
	return make_shared<TypeType>(make_shared<StructType>(*this));
}

TypePointer EnumValue::type(ContractDefinition const*) const
{
	auto parentDef = dynamic_cast<EnumDefinition const*>(scope());
	solAssert(parentDef, "Enclosing Scope of EnumValue was not set");
	return make_shared<EnumType>(*parentDef);
}

TypePointer EnumDefinition::type(ContractDefinition const*) const
{
	return make_shared<TypeType>(make_shared<EnumType>(*this));
}

TypePointer FunctionDefinition::type(ContractDefinition const*) const
{
	return make_shared<FunctionType>(*this);
}

string FunctionDefinition::externalSignature() const
{
	return FunctionType(*this).externalSignature(name());
}

TypePointer ModifierDefinition::type(ContractDefinition const*) const
{
	return make_shared<ModifierType>(*this);
}

TypePointer EventDefinition::type(ContractDefinition const*) const
{
	return make_shared<FunctionType>(*this);
}

bool VariableDeclaration::isLValue() const
{
	// External function parameters and constant declared variables are Read-Only
	return !isExternalCallableParameter() && !m_isConstant;
}

bool VariableDeclaration::isCallableParameter() const
{
	auto const* callable = dynamic_cast<CallableDeclaration const*>(scope());
	if (!callable)
		return false;
	for (auto const& variable: callable->parameters())
		if (variable.get() == this)
			return true;
	if (callable->returnParameterList())
		for (auto const& variable: callable->returnParameterList()->parameters())
			if (variable.get() == this)
				return true;
	return false;
}

bool VariableDeclaration::isExternalCallableParameter() const
{
	auto const* callable = dynamic_cast<CallableDeclaration const*>(scope());
	if (!callable || callable->visibility() != Declaration::Visibility::External)
		return false;
	for (auto const& variable: callable->parameters())
		if (variable.get() == this)
			return true;
	return false;
}

bool VariableDeclaration::canHaveAutoType() const
{
	auto const* callable = dynamic_cast<CallableDeclaration const*>(scope());
	return (!!callable && !isCallableParameter());
}

TypePointer VariableDeclaration::type(ContractDefinition const*) const
{
	return annotation().type;
}
