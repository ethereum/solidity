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
//
#pragma once

#include <libsolutil/Numeric.h>
#include <boost/outcome.hpp>

#include <variant>

namespace solidity::yul::interpreter
{

class ExplicitlyTerminated
{
};

class StepLimitReached
{
};

class RecursionDepthLimitReached
{
};

class ExpressionNestingLimitReached
{
};

class ImpureBuiltinEncountered
{
};

class UnlimitedLiteralEncountered
{
};

class TraceLimitReached
{
};

using ExecutionTerminated = std::variant<
	ExplicitlyTerminated,
	StepLimitReached,
	RecursionDepthLimitReached,
	ExpressionNestingLimitReached,
	ImpureBuiltinEncountered,
	UnlimitedLiteralEncountered,
	TraceLimitReached
>;

enum class ControlFlowState
{
	Default,
	Continue,
	Break,
	Leave
};

struct ExecutionOk
{
	ControlFlowState state{};

	bool operator==(ExecutionOk other) const
	{
		return state == other.state;
	}

	bool operator!=(ExecutionOk other) const
	{
		return state != other.state;
	}
};

struct EvaluationOk
{
	std::vector<u256> values;

	explicit EvaluationOk():
		values()
	{
	}

	explicit EvaluationOk(u256 _value):
		values{_value}
	{
	}

	/// Disable lvalue constructor to encourage rvalue usage.
	EvaluationOk(std::vector<u256> const& _values) = delete;

	explicit EvaluationOk(std::vector<u256>&& _values):
		values(std::move(_values))
	{
	}
};

using ExecutionResult = BOOST_OUTCOME_V2_NAMESPACE::result<ExecutionOk, ExecutionTerminated>;
using EvaluationResult = BOOST_OUTCOME_V2_NAMESPACE::result<EvaluationOk, ExecutionTerminated>;

}
