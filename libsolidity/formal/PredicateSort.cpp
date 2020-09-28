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

using namespace std;
using namespace solidity::util;
using namespace solidity::smtutil;

namespace solidity::frontend::smt
{

SortPointer interfaceSort(ContractDefinition const& _contract)
{
	return make_shared<FunctionSort>(
		stateSorts(_contract),
		SortProvider::boolSort
	);
}

SortPointer nondetInterfaceSort(ContractDefinition const& _contract)
{
	auto varSorts = stateSorts(_contract);
	return make_shared<FunctionSort>(
		varSorts + varSorts,
		SortProvider::boolSort
	);
}

SortPointer implicitConstructorSort()
{
	return arity0FunctionSort();
}

SortPointer constructorSort(ContractDefinition const& _contract)
{
	if (auto const* constructor = _contract.constructor())
		return functionSort(*constructor, &_contract);

	return make_shared<FunctionSort>(
		vector<SortPointer>{SortProvider::uintSort} + stateSorts(_contract),
		SortProvider::boolSort
	);
}

SortPointer functionSort(FunctionDefinition const& _function, ContractDefinition const* _contract)
{
	auto smtSort = [](auto _var) { return smt::smtSortAbstractFunction(*_var->type()); };
	auto varSorts = _contract ? stateSorts(*_contract) : vector<SortPointer>{};
	auto inputSorts = applyMap(_function.parameters(), smtSort);
	auto outputSorts = applyMap(_function.returnParameters(), smtSort);
	return make_shared<FunctionSort>(
		vector<SortPointer>{SortProvider::uintSort} +
			varSorts +
			inputSorts +
			varSorts +
			inputSorts +
			outputSorts,
		SortProvider::boolSort
	);
}

SortPointer functionBodySort(FunctionDefinition const& _function, ContractDefinition const* _contract)
{
	auto fSort = dynamic_pointer_cast<FunctionSort>(functionSort(_function, _contract));
	solAssert(fSort, "");

	auto smtSort = [](auto _var) { return smt::smtSortAbstractFunction(*_var->type()); };
	return make_shared<FunctionSort>(
		fSort->domain + applyMap(_function.localVariables(), smtSort),
		SortProvider::boolSort
	);
}

SortPointer arity0FunctionSort()
{
	return make_shared<FunctionSort>(
		vector<SortPointer>(),
		SortProvider::boolSort
	);
}

/// Helpers

vector<SortPointer> stateSorts(ContractDefinition const& _contract)
{
	return applyMap(
		SMTEncoder::stateVariablesIncludingInheritedAndPrivate(_contract),
		[](auto _var) { return smt::smtSortAbstractFunction(*_var->type()); }
	);
}

}
