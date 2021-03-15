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
 * Class that contains contextual information during IR generation.
 */

#include <libsolidity/codegen/ir/IRGenerationContext.h>

#include <libsolidity/codegen/YulUtilFunctions.h>
#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>

#include <libsolutil/Whiskers.h>
#include <libsolutil/StringUtils.h>

#include <range/v3/view/map.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;

string IRGenerationContext::enqueueFunctionForCodeGeneration(FunctionDefinition const& _function)
{
	string name = IRNames::function(_function);

	if (!m_functions.contains(name))
		m_functionGenerationQueue.insert(&_function);

	return name;
}

FunctionDefinition const* IRGenerationContext::dequeueFunctionForCodeGeneration()
{
	solAssert(!m_functionGenerationQueue.empty(), "");

	FunctionDefinition const* result = *m_functionGenerationQueue.begin();
	m_functionGenerationQueue.erase(m_functionGenerationQueue.begin());
	return result;
}

ContractDefinition const& IRGenerationContext::mostDerivedContract() const
{
	solAssert(m_mostDerivedContract, "Most derived contract requested but not set.");
	return *m_mostDerivedContract;
}

IRVariable const& IRGenerationContext::addLocalVariable(VariableDeclaration const& _varDecl)
{
	auto const& [it, didInsert] = m_localVariables.emplace(
		std::make_pair(&_varDecl, IRVariable{_varDecl})
	);
	solAssert(didInsert, "Local variable added multiple times.");
	return it->second;
}

IRVariable const& IRGenerationContext::localVariable(VariableDeclaration const& _varDecl)
{
	solAssert(
		m_localVariables.count(&_varDecl),
		"Unknown variable: " + _varDecl.name()
	);
	return m_localVariables.at(&_varDecl);
}

void IRGenerationContext::resetLocalVariables()
{
	m_localVariables.clear();
}

void IRGenerationContext::registerImmutableVariable(VariableDeclaration const& _variable)
{
	solAssert(_variable.immutable(), "Attempted to register a non-immutable variable as immutable.");
	solUnimplementedAssert(
		_variable.annotation().type->isValueType(),
		"Only immutable variables of value type are supported."
	);
	solAssert(m_reservedMemory.has_value(), "Reserved memory has already been reset.");
	m_immutableVariables[&_variable] = CompilerUtils::generalPurposeMemoryStart + *m_reservedMemory;
	solAssert(_variable.annotation().type->memoryHeadSize() == 32, "Memory writes might overlap.");
	*m_reservedMemory += _variable.annotation().type->memoryHeadSize();
}

size_t IRGenerationContext::immutableMemoryOffset(VariableDeclaration const& _variable) const
{
	solAssert(
		m_immutableVariables.count(&_variable),
		"Unknown immutable variable: " + _variable.name()
	);
	return m_immutableVariables.at(&_variable);
}

size_t IRGenerationContext::reservedMemory()
{
	solAssert(m_reservedMemory.has_value(), "Reserved memory was used before.");
	size_t reservedMemory = *m_reservedMemory;
	m_reservedMemory = std::nullopt;
	return reservedMemory;
}

void IRGenerationContext::addStateVariable(
	VariableDeclaration const& _declaration,
	u256 _storageOffset,
	unsigned _byteOffset
)
{
	m_stateVariables[&_declaration] = make_pair(move(_storageOffset), _byteOffset);
}

string IRGenerationContext::newYulVariable()
{
	return "_" + to_string(++m_varCounter);
}

void IRGenerationContext::initializeInternalDispatch(InternalDispatchMap _internalDispatch)
{
	solAssert(internalDispatchClean(), "");

	for (DispatchSet const& functions: _internalDispatch | ranges::views::values)
		for (auto function: functions)
			enqueueFunctionForCodeGeneration(*function);

	m_internalDispatchMap = move(_internalDispatch);
}

InternalDispatchMap IRGenerationContext::consumeInternalDispatchMap()
{
	InternalDispatchMap internalDispatch = move(m_internalDispatchMap);
	m_internalDispatchMap.clear();
	return internalDispatch;
}

void IRGenerationContext::addToInternalDispatch(FunctionDefinition const& _function)
{
	FunctionType const* functionType = TypeProvider::function(_function, FunctionType::Kind::Internal);
	solAssert(functionType, "");

	YulArity arity = YulArity::fromType(*functionType);

	if (m_internalDispatchMap.count(arity) != 0 && m_internalDispatchMap[arity].count(&_function) != 0)
		// Note that m_internalDispatchMap[arity] is a set with a custom comparator, which looks at function IDs not definitions
		solAssert(*m_internalDispatchMap[arity].find(&_function) == &_function, "Different definitions with the same function ID");

	m_internalDispatchMap[arity].insert(&_function);
	enqueueFunctionForCodeGeneration(_function);
}


void IRGenerationContext::internalFunctionCalledThroughDispatch(YulArity const& _arity)
{
	m_internalDispatchMap.try_emplace(_arity);
}

YulUtilFunctions IRGenerationContext::utils()
{
	return YulUtilFunctions(m_evmVersion, m_revertStrings, m_functions);
}

ABIFunctions IRGenerationContext::abiFunctions()
{
	return ABIFunctions(m_evmVersion, m_revertStrings, m_functions);
}
