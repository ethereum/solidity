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

#include <libsolidity/formal/PredicateInstance.h>

#include <libsolidity/formal/EncodingContext.h>
#include <libsolidity/formal/SMTEncoder.h>

using namespace std;
using namespace solidity::util;
using namespace solidity::smtutil;

namespace solidity::frontend::smt
{

smtutil::Expression interface(Predicate const& _pred, ContractDefinition const& _contract, EncodingContext& _context)
{
	return _pred(currentStateVariables(_contract, _context));
}

smtutil::Expression constructor(Predicate const& _pred, ContractDefinition const& _contract, EncodingContext& _context)
{
	if (auto const* constructor = _contract.constructor())
		return _pred(currentFunctionVariables(*constructor, &_contract, _context));

	return _pred(
		vector<smtutil::Expression>{_context.state().errorFlag().currentValue()} +
		currentStateVariables(_contract, _context)
	);
}

/// Currently it does not have arguments but it will have tx data in the future.
smtutil::Expression implicitConstructor(Predicate const& _pred, ContractDefinition const&, EncodingContext&)
{
	return _pred({});
}

smtutil::Expression function(
	Predicate const& _pred,
	FunctionDefinition const& _function,
	ContractDefinition const* _contract,
	EncodingContext& _context
)
{
	return _pred(currentFunctionVariables(_function, _contract, _context));
}

smtutil::Expression functionBlock(
	Predicate const& _pred,
	FunctionDefinition const& _function,
	ContractDefinition const* _contract,
	EncodingContext& _context
)
{
	return _pred(currentBlockVariables(_function, _contract, _context));
}

/// Helpers

vector<smtutil::Expression> initialStateVariables(ContractDefinition const& _contract, EncodingContext& _context)
{
	return stateVariablesAtIndex(0, _contract, _context);
}

vector<smtutil::Expression> stateVariablesAtIndex(unsigned _index, ContractDefinition const& _contract, EncodingContext& _context)
{
	return applyMap(
		SMTEncoder::stateVariablesIncludingInheritedAndPrivate(_contract),
		[&](auto _var) { return _context.variable(*_var)->valueAtIndex(_index); }
	);
}

vector<smtutil::Expression> currentStateVariables(ContractDefinition const& _contract, EncodingContext& _context)
{
	return applyMap(
		SMTEncoder::stateVariablesIncludingInheritedAndPrivate(_contract),
		[&](auto _var) { return _context.variable(*_var)->currentValue(); }
	);
}

vector<smtutil::Expression> currentFunctionVariables(
	FunctionDefinition const& _function,
	ContractDefinition const* _contract,
	EncodingContext& _context
)
{
	vector<smtutil::Expression> exprs{_context.state().errorFlag().currentValue()};
	exprs += _contract ? initialStateVariables(*_contract, _context) : vector<smtutil::Expression>{};
	exprs += applyMap(_function.parameters(), [&](auto _var) { return _context.variable(*_var)->valueAtIndex(0); });
	exprs += _contract ? currentStateVariables(*_contract, _context) : vector<smtutil::Expression>{};
	exprs += applyMap(_function.parameters(), [&](auto _var) { return _context.variable(*_var)->currentValue(); });
	exprs += applyMap(_function.returnParameters(), [&](auto _var) { return _context.variable(*_var)->currentValue(); });
	return exprs;
}

vector<smtutil::Expression> currentBlockVariables(FunctionDefinition const& _function, ContractDefinition const* _contract, EncodingContext& _context)
{
	return currentFunctionVariables(_function, _contract, _context) +
		applyMap(
			_function.localVariables(),
			[&](auto _var) { return _context.variable(*_var)->currentValue(); }
		);
}

}
