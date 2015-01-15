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
	m_stateVariablesSize += _declaration.getType()->getStorageSize();
}

void CompilerContext::addVariable(VariableDeclaration const& _declaration)
{
	m_localVariables[&_declaration] = m_localVariablesSize;
	m_localVariablesSize += _declaration.getType()->getSizeOnStack();
}

void CompilerContext::addAndInitializeVariable(VariableDeclaration const& _declaration)
{
	addVariable(_declaration);

	int const size = _declaration.getType()->getSizeOnStack();
	for (int i = 0; i < size; ++i)
		*this << u256(0);
	m_asm.adjustDeposit(-size);
}

void CompilerContext::addFunction(FunctionDefinition const& _function)
{
	eth::AssemblyItem tag(m_asm.newTag());
	m_functionEntryLabels.insert(make_pair(&_function, tag));
	m_virtualFunctionEntryLabels.insert(make_pair(_function.getName(), tag));
}

bytes const& CompilerContext::getCompiledContract(const ContractDefinition& _contract) const
{
	auto ret = m_compiledContracts.find(&_contract);
	solAssert(ret != m_compiledContracts.end(), "Compiled contract not found.");
	return *ret->second;
}

bool CompilerContext::isLocalVariable(Declaration const* _declaration) const
{
	return m_localVariables.count(_declaration) > 0;
}

eth::AssemblyItem CompilerContext::getFunctionEntryLabel(FunctionDefinition const& _function) const
{
	auto res = m_functionEntryLabels.find(&_function);
	solAssert(res != m_functionEntryLabels.end(), "Function entry label not found.");
	return res->second.tag();
}

eth::AssemblyItem CompilerContext::getVirtualFunctionEntryLabel(FunctionDefinition const& _function) const
{
	auto res = m_virtualFunctionEntryLabels.find(_function.getName());
	solAssert(res != m_virtualFunctionEntryLabels.end(), "Function entry label not found.");
	return res->second.tag();
}

unsigned CompilerContext::getBaseStackOffsetOfVariable(Declaration const& _declaration) const
{
	auto res = m_localVariables.find(&_declaration);
	solAssert(res != m_localVariables.end(), "Variable not found on stack.");
	return m_localVariablesSize - res->second - 1;
}

unsigned CompilerContext::baseToCurrentStackOffset(unsigned _baseOffset) const
{
	return _baseOffset + m_asm.deposit();
}

unsigned CompilerContext::currentToBaseStackOffset(unsigned _offset) const
{
	return -baseToCurrentStackOffset(-_offset);
}

u256 CompilerContext::getStorageLocationOfVariable(const Declaration& _declaration) const
{
	auto it = m_stateVariables.find(&_declaration);
	solAssert(it != m_stateVariables.end(), "Variable not found in storage.");
	return it->second;
}

}
}
