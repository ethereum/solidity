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
smtutil::Expression interfacePre(Predicate const& _pred, ContractDefinition const& _contract, EncodingContext& _context)
{
	auto& state = _context.state();
	vector<smtutil::Expression> stateExprs{state.thisAddress(0), state.crypto(0), state.state(0)};
	return _pred(stateExprs + initialStateVariables(_contract, _context));
}

smtutil::Expression interface(Predicate const& _pred, ContractDefinition const& _contract, EncodingContext& _context)
{
	auto& state = _context.state();
	vector<smtutil::Expression> stateExprs{state.thisAddress(0), state.crypto(0), state.state()};
	return _pred(stateExprs + currentStateVariables(_contract, _context));
}

smtutil::Expression nondetInterface(Predicate const& _pred, ContractDefinition const& _contract, EncodingContext& _context, unsigned _preIdx, unsigned _postIdx)
{
	return _pred(
		vector<smtutil::Expression>{_context.state().state(_preIdx)} +
		stateVariablesAtIndex(_preIdx, _contract, _context) +
		vector<smtutil::Expression>{_context.state().state(_postIdx)} +
		stateVariablesAtIndex(_postIdx, _contract, _context)
	);
}

smtutil::Expression implicitConstructor(Predicate const& _pred, ContractDefinition const&, EncodingContext& _context)
{
	auto& state = _context.state();
	vector<smtutil::Expression> stateExprs{state.errorFlag().currentValue(), state.thisAddress(0), state.crypto(0), state.tx(0), state.state(0)};
	return _pred(stateExprs);
}

smtutil::Expression constructor(Predicate const& _pred, ContractDefinition const& _contract, EncodingContext& _context)
{
	if (auto const* constructor = _contract.constructor())
		return _pred(currentFunctionVariables(*constructor, &_contract, _context));

	auto& state = _context.state();
	vector<smtutil::Expression> stateExprs{state.errorFlag().currentValue(), state.thisAddress(0), state.crypto(0), state.tx(0), state.state(0), state.state()};
	return _pred(stateExprs + currentStateVariables(_contract, _context));
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
	auto& state = _context.state();
	vector<smtutil::Expression> exprs{_context.state().errorFlag().currentValue(), state.thisAddress(0), state.crypto(0), state.tx(0), state.state(0)};
	exprs += _contract ? initialStateVariables(*_contract, _context) : vector<smtutil::Expression>{};
	exprs += applyMap(_function.parameters(), [&](auto _var) { return _context.variable(*_var)->valueAtIndex(0); });
	exprs += vector<smtutil::Expression>{state.state()};
	exprs += _contract ? currentStateVariables(*_contract, _context) : vector<smtutil::Expression>{};
	exprs += applyMap(_function.parameters(), [&](auto _var) { return _context.variable(*_var)->currentValue(); });
	exprs += applyMap(_function.returnParameters(), [&](auto _var) { return _context.variable(*_var)->currentValue(); });
	return exprs;
}

vector<smtutil::Expression> currentBlockVariables(FunctionDefinition const& _function, ContractDefinition const* _contract, EncodingContext& _context)
{
	return currentFunctionVariables(_function, _contract, _context) +
		applyMap(
			SMTEncoder::localVariablesIncludingModifiers(_function),
			[&](auto _var) { return _context.variable(*_var)->currentValue(); }
		);
}

}
