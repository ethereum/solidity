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
 * Routines used by both the compiler and the expression compiler.
 */

#pragma once

#include <libsolidity/CompilerContext.h>
#include <libsolidity/ASTForward.h>

namespace dev {
namespace solidity {

class Type; // forward

/// The size in bytes of the function (hash) identifier
static const unsigned int g_functionIdentifierOffset = 4;

class CompilerUtils
{
public:
	CompilerUtils(CompilerContext& _context): m_context(_context) {}

	/// Loads data from memory to the stack.
	/// @param _offset offset in memory (or calldata)
	/// @param _bytes number of bytes to load
	/// @param _leftAligned if true, store left aligned on stack (otherwise right aligned)
	/// @param _fromCalldata if true, load from calldata, not from memory
	void loadFromMemory(unsigned _offset, unsigned _bytes = 32, bool _leftAligned = false, bool _fromCalldata = false);
	/// Stores data from stack in memory.
	/// @param _offset offset in memory
	/// @param _bytes number of bytes to store
	/// @param _leftAligned if true, data is left aligned on stack (otherwise right aligned)
	void storeInMemory(unsigned _offset, unsigned _bytes = 32, bool _leftAligned = false);

	/// Moves the value that is at the top of the stack to a stack variable.
	void moveToStackVariable(VariableDeclaration const& _variable);
	/// Copies a variable of type @a _type from a stack depth of @a _stackDepth to the top of the stack.
	void copyToStackTop(unsigned _stackDepth, Type const& _type);
	/// Removes the current value from the top of the stack.
	void popStackElement(Type const& _type);

	template <class T>
	static unsigned getSizeOnStack(std::vector<T> const& _variables);
	static unsigned getSizeOnStack(std::vector<std::shared_ptr<Type const>> const& _variableTypes);

private:
	CompilerContext& m_context;
};

template <class T>
unsigned CompilerUtils::getSizeOnStack(std::vector<T> const& _variables)
{
	unsigned size = 0;
	for (T const& variable: _variables)
		size += variable->getType()->getSizeOnStack();
	return size;
}

}
}
