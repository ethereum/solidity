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
#include <libyul/backends/evm/EVMMetrics.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/SideEffects.h>
#include <libyul/AST.h>
#include <libyul/Utilities.h>

#include <libevmasm/GasMeter.h>
#include <libsolutil/Keccak256.h>

#include <limits>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::evmasm;
using namespace solidity::yul;

void LoadResolver::run(OptimiserStepContext& _context, Block& _ast)
{
	bool containsMSize = MSizeFinder::containsMSize(_context.dialect, _ast);
	LoadResolver{
		_context.dialect,
		SideEffectsPropagator::sideEffects(_context.dialect, CallGraphGenerator::callGraph(_ast)),
		containsMSize,
		_context.expectedExecutionsPerDeployment
	}(_ast);
}

void LoadResolver::visit(Expression& _e)
{
	DataFlowAnalyzer::visit(_e);

	if (FunctionCall const* funCall = std::get_if<FunctionCall>(&_e))
	{
		if (funCall->functionName.name == m_loadFunctionName[static_cast<unsigned>(StoreLoadLocation::Memory)])
			tryResolve(_e, StoreLoadLocation::Memory, funCall->arguments);
		else if (funCall->functionName.name == m_loadFunctionName[static_cast<unsigned>(StoreLoadLocation::Storage)])
			tryResolve(_e, StoreLoadLocation::Storage, funCall->arguments);
		else if (!m_containsMSize && funCall->functionName.name == m_dialect.hashFunction({}))
		{
			Identifier const* start = std::get_if<Identifier>(&funCall->arguments.at(0));
			Identifier const* length = std::get_if<Identifier>(&funCall->arguments.at(1));
			if (start && length)
				if (auto const& value = keccakValue(start->name, length->name))
					if (inScope(*value))
					{
						_e = Identifier{debugDataOf(_e), *value};
						return;
					}
			tryEvaluateKeccak(_e, funCall->arguments);
		}
	}
}

void LoadResolver::tryResolve(
	Expression& _e,
	StoreLoadLocation _location,
	std::vector<Expression> const& _arguments
)
{
	if (_arguments.empty() || !std::holds_alternative<Identifier>(_arguments.at(0)))
		return;

	YulString key = std::get<Identifier>(_arguments.at(0)).name;
	if (_location == StoreLoadLocation::Storage)
	{
		if (auto value = storageValue(key))
			if (inScope(*value))
				_e = Identifier{debugDataOf(_e), *value};
	}
	else if (!m_containsMSize && _location == StoreLoadLocation::Memory)
		if (auto value = memoryValue(key))
			if (inScope(*value))
				_e = Identifier{debugDataOf(_e), *value};
}

void LoadResolver::tryEvaluateKeccak(
	Expression& _e,
	std::vector<Expression> const& _arguments
)
{
	yulAssert(_arguments.size() == 2, "");
	Identifier const* memoryKey = std::get_if<Identifier>(&_arguments.at(0));
	Identifier const* length = std::get_if<Identifier>(&_arguments.at(1));

	if (!memoryKey || !length)
		return;

	// The costs are only correct for hashes of 32 bytes or 1 word (when rounded up).
	GasMeter gasMeter{
		dynamic_cast<EVMDialect const&>(m_dialect),
		!m_expectedExecutionsPerDeployment,
		m_expectedExecutionsPerDeployment ? *m_expectedExecutionsPerDeployment : 1
	};

	bigint costOfKeccak = gasMeter.costs(_e);
	bigint costOfLiteral = gasMeter.costs(
		Literal{
			{},
			LiteralKind::Number,
			// a dummy 256-bit number to represent the Keccak256 hash.
			YulString{std::numeric_limits<u256>::max().str()},
			{}
		}
	);

	// We skip if there are no net gas savings.
	// Note that for default `m_runs = 200`, the values are
	// `costOfLiteral = 7200` and `costOfKeccak = 9000` for runtime context.
	// For creation context: `costOfLiteral = 531` and `costOfKeccak = 90`.
	if (costOfLiteral > costOfKeccak)
		return;

	std::optional<YulString> value = memoryValue(memoryKey->name);
	if (value && inScope(*value))
	{
		std::optional<u256> memoryContent = valueOfIdentifier(*value);
		std::optional<u256> byteLength = valueOfIdentifier(length->name);
		if (memoryContent && byteLength && *byteLength <= 32)
		{
			bytes contentAsBytes = toBigEndian(*memoryContent);
			contentAsBytes.resize(static_cast<size_t>(*byteLength));
			_e = Literal{
				debugDataOf(_e),
				LiteralKind::Number,
				YulString{u256(keccak256(contentAsBytes)).str()},
				m_dialect.defaultType
			};
		}
	}
}
