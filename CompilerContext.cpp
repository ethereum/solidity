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

namespace dev {
namespace solidity {

void CompilerContext::addMagicGlobal(MagicVariableDeclaration const& _declaration)
{
	m_magicGlobals.insert(&_declaration);
}

void CompilerContext::addStateVariable(VariableDeclaration const& _declaration)
{
	m_stateVariables[&_declaration] = m_stateVariablesSize;
	m_stateVariablesSize += _declaration.getType()->getStorageSize();
}

void CompilerContext::initializeLocalVariables(unsigned _numVariables)
{
	if (_numVariables > 0)
	{
		*this << u256(0);
		for (unsigned i = 1; i < _numVariables; ++i)
			*this << eth::Instruction::DUP1;
		m_asm.adjustDeposit(-_numVariables);
	}
}

bool CompilerContext::isLocalVariable(Declaration const* _declaration) const
{
	return std::find(m_localVariables.begin(), m_localVariables.end(), _declaration) != m_localVariables.end();
}

eth::AssemblyItem CompilerContext::getFunctionEntryLabel(FunctionDefinition const& _function) const
{
	auto res = m_functionEntryLabels.find(&_function);
	if (asserts(res != m_functionEntryLabels.end()))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Function entry label not found."));
	return res->second.tag();
}

unsigned CompilerContext::getBaseStackOffsetOfVariable(Declaration const& _declaration) const
{
	auto res = find(begin(m_localVariables), end(m_localVariables), &_declaration);
	if (asserts(res != m_localVariables.end()))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Variable not found on stack."));
	return unsigned(end(m_localVariables) - res - 1);
}

unsigned CompilerContext::baseToCurrentStackOffset(unsigned _baseOffset) const
{
	return _baseOffset + m_asm.deposit();
}

u256 CompilerContext::getStorageLocationOfVariable(const Declaration& _declaration) const
{
	auto it = m_stateVariables.find(&_declaration);
	if (it == m_stateVariables.end())
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Variable not found in storage."));
	return it->second;
}



}
}
