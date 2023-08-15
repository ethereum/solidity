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

#include <libsolidity/formal/PredicateSort.h>

#include <libsolidity/formal/SMTEncoder.h>
#include <libsolidity/formal/SymbolicTypes.h>

using namespace solidity::util;
using namespace solidity::smtutil;

namespace solidity::frontend::smt
{

SortPointer interfaceSort(ContractDefinition const& _contract, SymbolicState& _state)
{
	return std::make_shared<FunctionSort>(
		std::vector<SortPointer>{_state.thisAddressSort(), _state.abiSort(), _state.cryptoSort(), _state.stateSort()} + stateSorts(_contract),
		SortProvider::boolSort
	);
}

SortPointer nondetInterfaceSort(ContractDefinition const& _contract, SymbolicState& _state)
{
	auto varSorts = stateSorts(_contract);
	std::vector<SortPointer> stateSort{_state.stateSort()};
	return std::make_shared<FunctionSort>(
		std::vector<SortPointer>{_state.errorFlagSort(), _state.thisAddressSort(), _state.abiSort(), _state.cryptoSort()} +
			stateSort +
			varSorts +
			stateSort +
			varSorts,
		SortProvider::boolSort
	);
}

SortPointer constructorSort(ContractDefinition const& _contract, SymbolicState& _state)
{
	if (auto const* constructor = _contract.constructor())
		return functionSort(*constructor, &_contract, _state);

	auto varSorts = stateSorts(_contract);
	std::vector<SortPointer> stateSort{_state.stateSort()};
	return std::make_shared<FunctionSort>(
		std::vector<SortPointer>{_state.errorFlagSort(), _state.thisAddressSort(), _state.abiSort(), _state.cryptoSort(), _state.txSort(), _state.stateSort(), _state.stateSort()} + varSorts + varSorts,
		SortProvider::boolSort
	);
}

SortPointer functionSort(FunctionDefinition const& _function, ContractDefinition const* _contract, SymbolicState& _state)
{
	auto smtSort = [](auto _var) { return smt::smtSortAbstractFunction(*_var->type()); };
	auto varSorts = _contract ? stateSorts(*_contract) : std::vector<SortPointer>{};
	auto inputSorts = applyMap(_function.parameters(), smtSort);
	auto outputSorts = applyMap(_function.returnParameters(), smtSort);
	return std::make_shared<FunctionSort>(
		std::vector<SortPointer>{_state.errorFlagSort(), _state.thisAddressSort(), _state.abiSort(), _state.cryptoSort(), _state.txSort(), _state.stateSort()} +
			varSorts +
			inputSorts +
			std::vector<SortPointer>{_state.stateSort()} +
			varSorts +
			inputSorts +
			outputSorts,
		SortProvider::boolSort
	);
}

SortPointer functionBodySort(FunctionDefinition const& _function, ContractDefinition const* _contract, SymbolicState& _state)
{
	auto fSort = std::dynamic_pointer_cast<FunctionSort>(functionSort(_function, _contract, _state));
	solAssert(fSort, "");

	auto smtSort = [](auto _var) { return smt::smtSortAbstractFunction(*_var->type()); };
	return std::make_shared<FunctionSort>(
		fSort->domain + applyMap(SMTEncoder::localVariablesIncludingModifiers(_function, _contract), smtSort),
		SortProvider::boolSort
	);
}

SortPointer arity0FunctionSort()
{
	return std::make_shared<FunctionSort>(
		std::vector<SortPointer>(),
		SortProvider::boolSort
	);
}

/// Helpers

std::vector<SortPointer> stateSorts(ContractDefinition const& _contract)
{
	return applyMap(
		SMTEncoder::stateVariablesIncludingInheritedAndPrivate(_contract),
		[](auto _var) { return smt::smtSortAbstractFunction(*_var->type()); }
	);
}

}
