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
 * Optimisation stage that replaces expressions of type ``sload(x)`` by the value
 * currently stored in storage, if known.
 */

#include <libyul/optimiser/LoadResolver.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/SideEffects.h>
#include <libyul/AST.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void LoadResolver::run(OptimiserStepContext& _context, Block& _ast)
{
	bool containsMSize = MSizeFinder::containsMSize(_context.dialect, _ast);
	LoadResolver{
		_context.dialect,
		SideEffectsPropagator::sideEffects(_context.dialect, CallGraphGenerator::callGraph(_ast)),
		!containsMSize
	}(_ast);
}

void LoadResolver::visit(Expression& _e)
{
	DataFlowAnalyzer::visit(_e);

	if (FunctionCall const* funCall = std::get_if<FunctionCall>(&_e))
		for (auto location: { StoreLoadLocation::Memory, StoreLoadLocation::Storage })
			if (funCall->functionName.name == m_loadFunctionName[static_cast<unsigned>(location)])
			{
				tryResolve(_e, location, funCall->arguments);
				break;
			}
}

void LoadResolver::tryResolve(
	Expression& _e,
	StoreLoadLocation _location,
	vector<Expression> const& _arguments
)
{
	if (_arguments.empty() || !holds_alternative<Identifier>(_arguments.at(0)))
		return;

	YulString key = std::get<Identifier>(_arguments.at(0)).name;
	if (
		_location == StoreLoadLocation::Storage &&
		m_storage.values.count(key)
	)
		_e = Identifier{locationOf(_e), m_storage.values[key]};
	else if (
		m_optimizeMLoad &&
		_location == StoreLoadLocation::Memory &&
		m_memory.values.count(key)
	)
		_e = Identifier{locationOf(_e), m_memory.values[key]};
}
