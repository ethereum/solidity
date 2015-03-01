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
 * Utilities for the solidity compiler.
 */

#include <utility>
#include <numeric>
#include <libsolidity/AST.h>
#include <libsolidity/Compiler.h>

using namespace std;

namespace dev
{
namespace solidity
{

void CompilerContext::addMagicGlobal(MagicVariableDeclaration const& _declaration)
{
	m_magicGlobals.insert(&_declaration);
}

void CompilerContext::addStateVariable(VariableDeclaration const& _declaration)
{
	m_stateVariables[&_declaration] = m_stateVariablesSize;
	bigint newSize = bigint(m_stateVariablesSize) + _declaration.getType()->getStorageSize();
	if (newSize >= bigint(1) << 256)
		BOOST_THROW_EXCEPTION(TypeError()
			<< errinfo_comment("State variable does not fit in storage.")
			<< errinfo_sourceLocation(_declaration.getLocation()));
	m_stateVariablesSize = u256(newSize);
}

void CompilerContext::startFunction(Declaration const& _function)
{
	m_functionsWithCode.insert(&_function);
	*this << getFunctionEntryLabel(_function);
}

void CompilerContext::addVariable(VariableDeclaration const& _declaration,
								  unsigned _offsetToCurrent)
{
	solAssert(m_asm.deposit() >= 0 && unsigned(m_asm.deposit()) >= _offsetToCurrent, "");
	m_localVariables[&_declaration] = unsigned(m_asm.deposit()) - _offsetToCurrent;
}

void CompilerContext::removeVariable(VariableDeclaration const& _declaration)
{
	solAssert(!!m_localVariables.count(&_declaration), "");
	m_localVariables.erase(&_declaration);
}

void CompilerContext::addAndInitializeVariable(VariableDeclaration const& _declaration)
{
	LocationSetter locationSetter(*this, &_declaration);
	addVariable(_declaration);
	int const size = _declaration.getType()->getSizeOnStack();
	for (int i = 0; i < size; ++i)
		*this << u256(0);
}

bytes const& CompilerContext::getCompiledContract(const ContractDefinition& _contract) const
{
	auto ret = m_compiledContracts.find(&_contract);
	solAssert(ret != m_compiledContracts.end(), "Compiled contract not found.");
	return *ret->second;
}

bool CompilerContext::isLocalVariable(Declaration const* _declaration) const
{
	return !!m_localVariables.count(_declaration);
}

eth::AssemblyItem CompilerContext::getFunctionEntryLabel(Declaration const& _declaration)
{
	auto res = m_functionEntryLabels.find(&_declaration);
	if (res == m_functionEntryLabels.end())
	{
		eth::AssemblyItem tag(m_asm.newTag());
		m_functionEntryLabels.insert(make_pair(&_declaration, tag));
		return tag.tag();
	}
	else
		return res->second.tag();
}

eth::AssemblyItem CompilerContext::getVirtualFunctionEntryLabel(FunctionDefinition const& _function)
{
	solAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	for (ContractDefinition const* contract: m_inheritanceHierarchy)
		for (ASTPointer<FunctionDefinition> const& function: contract->getDefinedFunctions())
		{
			if (!function->isConstructor() &&
				dynamic_cast<FunctionType const&>(*function->getType(contract)).getCanonicalSignature() ==
				dynamic_cast<FunctionType const&>(*_function.getType(contract)).getCanonicalSignature())
				return getFunctionEntryLabel(*function);
		}
	solAssert(false, "Virtual function " + _function.getName() + " not found.");
	return m_asm.newTag(); // not reached
}

eth::AssemblyItem CompilerContext::getSuperFunctionEntryLabel(string const& _name, ContractDefinition const& _base)
{
	auto it = getSuperContract(_base);
	for (; it != m_inheritanceHierarchy.end(); ++it)
		for (ASTPointer<FunctionDefinition> const& function: (*it)->getDefinedFunctions())
			if (!function->isConstructor() && function->getName() == _name)     // TODO: add a test case for this!
				return getFunctionEntryLabel(*function);
	solAssert(false, "Super function " + _name + " not found.");
	return m_asm.newTag(); // not reached
}

FunctionDefinition const* CompilerContext::getNextConstructor(ContractDefinition const& _contract) const
{
	vector<ContractDefinition const*>::const_iterator it = getSuperContract(_contract);
	for (; it != m_inheritanceHierarchy.end(); ++it)
		if ((*it)->getConstructor())
			return (*it)->getConstructor();

	return nullptr;
}

set<Declaration const*> CompilerContext::getFunctionsWithoutCode()
{
	set<Declaration const*> functions;
	for (auto const& it: m_functionEntryLabels)
		if (m_functionsWithCode.count(it.first) == 0)
			functions.insert(it.first);
	return move(functions);
}

ModifierDefinition const& CompilerContext::getFunctionModifier(string const& _name) const
{
	solAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	for (ContractDefinition const* contract: m_inheritanceHierarchy)
		for (ASTPointer<ModifierDefinition> const& modifier: contract->getFunctionModifiers())
			if (modifier->getName() == _name)
				return *modifier.get();
	BOOST_THROW_EXCEPTION(InternalCompilerError()
						  << errinfo_comment("Function modifier " + _name + " not found."));
}

unsigned CompilerContext::getBaseStackOffsetOfVariable(Declaration const& _declaration) const
{
	auto res = m_localVariables.find(&_declaration);
	solAssert(res != m_localVariables.end(), "Variable not found on stack.");
	return res->second;
}

unsigned CompilerContext::baseToCurrentStackOffset(unsigned _baseOffset) const
{
	return m_asm.deposit() - _baseOffset - 1;
}

unsigned CompilerContext::currentToBaseStackOffset(unsigned _offset) const
{
	return m_asm.deposit() - _offset - 1;
}

u256 CompilerContext::getStorageLocationOfVariable(const Declaration& _declaration) const
{
	auto it = m_stateVariables.find(&_declaration);
	solAssert(it != m_stateVariables.end(), "Variable not found in storage.");
	return it->second;
}

void CompilerContext::resetVisitedNodes(ASTNode const* _node)
{
	stack<ASTNode const*> newStack;
	newStack.push(_node);
	std::swap(m_visitedNodes, newStack);
}

CompilerContext& CompilerContext::operator<<(eth::AssemblyItem const& _item)
{
	solAssert(!m_visitedNodes.empty(), "No node on the visited stack");
	m_asm.append(_item, m_visitedNodes.top()->getLocation());
	return *this;
}

CompilerContext& CompilerContext::operator<<(eth::Instruction _instruction)
{
	solAssert(!m_visitedNodes.empty(), "No node on the visited stack");
	m_asm.append(_instruction, m_visitedNodes.top()->getLocation());
	return *this;
}

CompilerContext& CompilerContext::operator<<(u256 const& _value)
{
	solAssert(!m_visitedNodes.empty(), "No node on the visited stack");
	m_asm.append(_value, m_visitedNodes.top()->getLocation());
	return *this;
}

CompilerContext& CompilerContext::operator<<(bytes const& _data)
{
	solAssert(!m_visitedNodes.empty(), "No node on the visited stack");
	m_asm.append(_data, m_visitedNodes.top()->getLocation());
	return *this;
}

vector<ContractDefinition const*>::const_iterator CompilerContext::getSuperContract(ContractDefinition const& _contract) const
{
	solAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	auto it = find(m_inheritanceHierarchy.begin(), m_inheritanceHierarchy.end(), &_contract);
	solAssert(it != m_inheritanceHierarchy.end(), "Base not found in inheritance hierarchy.");
	return ++it;
}

}
}
