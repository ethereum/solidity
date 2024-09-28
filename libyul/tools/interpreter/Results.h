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

#include <variant>

namespace solidity::yul::tools::interpreter
{

template<typename T>
class ExecutionTerminatedCommon
{
public:
	bool operator==(T) const { return true; }
	bool operator!=(T) const { return false; }
};

class ExplicitlyTerminated : public ExecutionTerminatedCommon<ExplicitlyTerminated>
{
};

class ExplicitlyTerminatedWithReturn : public ExecutionTerminatedCommon<ExplicitlyTerminatedWithReturn>
{
};

class StepLimitReached : public ExecutionTerminatedCommon<StepLimitReached>
{
};

class RecursionDepthLimitReached : public ExecutionTerminatedCommon<RecursionDepthLimitReached>
{
};

class ExpressionNestingLimitReached : public ExecutionTerminatedCommon<ExpressionNestingLimitReached>
{
};

class ImpureBuiltinEncountered : public ExecutionTerminatedCommon<ImpureBuiltinEncountered>
{
};

class UnlimitedLiteralEncountered : public ExecutionTerminatedCommon<UnlimitedLiteralEncountered>
{
};

using ExecutionTerminated = std::variant<
	ExplicitlyTerminated,
	ExplicitlyTerminatedWithReturn,
	StepLimitReached,
	RecursionDepthLimitReached,
	ExpressionNestingLimitReached,
	ImpureBuiltinEncountered,
	UnlimitedLiteralEncountered
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
	ControlFlowState state;

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

	EvaluationOk(u256 _x):
		values{_x}
	{
	}

	/// Disable lvalue constructor to encourage rvalue usage.
	EvaluationOk(std::vector<u256> const& _values) = delete;

	EvaluationOk(std::vector<u256>&& _values):
		values(std::move(_values))
	{
	}
};

using ExecutionResult = std::variant<ExecutionOk, ExecutionTerminated>;
using EvaluationResult = std::variant<EvaluationOk, ExecutionTerminated>;

}
