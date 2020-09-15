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

#pragma once

#include <libsolidity/formal/Predicate.h>

#include <libsmtutil/Sorts.h>

namespace solidity::frontend::smt
{

/**
 * This file represents the specification for CHC predicate sorts.
 * Types of predicates:
 *
 * 1. Interface
 * The idle state of a contract. Signature:
 * interface(stateVariables).
 *
 * 2. Nondet interface
 * The nondeterminism behavior of a contract. Signature:
 * nondet_interface(stateVariables, stateVariables').
 *
 * 3. Implicit constructor
 * The implicit constructor of a contract, that is, without input parameters. Signature:
 * implicit_constructor().
 *
 * 4. Constructor entry/summary
 * The summary of an implicit constructor. Signature:
 * constructor_summary(error, stateVariables').
 *
 * 5. Function entry/summary
 * The entry point of a function definition. Signature:
 * function_entry(error, stateVariables, inputVariables, stateVariables', inputVariables', outputVariables').
 *
 * 6. Function body
 * Use for any predicate within a function. Signature:
 * function_body(error, stateVariables, inputVariables, stateVariables', inputVariables', outputVariables', localVariables).
 */

/// @returns the interface predicate sort for _contract.
smtutil::SortPointer interfaceSort(ContractDefinition const& _contract);

/// @returns the nondeterminisc interface predicate sort for _contract.
smtutil::SortPointer nondetInterfaceSort(ContractDefinition const& _contract);

/// @returns the implicit constructor predicate sort.
smtutil::SortPointer implicitConstructorSort();

/// @returns the constructor entry/summary predicate sort for _contract.
smtutil::SortPointer constructorSort(ContractDefinition const& _contract);

/// @returns the function entry/summary predicate sort for _function contained in _contract.
smtutil::SortPointer functionSort(FunctionDefinition const& _function, ContractDefinition const* _contract);

/// @returns the function body predicate sort for _function contained in _contract.
smtutil::SortPointer functionBodySort(FunctionDefinition const& _function, ContractDefinition const* _contract);

/// @returns the sort of a predicate without parameters.
smtutil::SortPointer arity0FunctionSort();

/// Helpers

std::vector<smtutil::SortPointer> stateSorts(ContractDefinition const& _contract) ;

}
