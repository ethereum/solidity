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

#include <libyul/interpreter/Results.h>

#include <libyul/ASTForward.h>

#include <libsolutil/Numeric.h>

#include <cstddef>
#include <variant>
#include <vector>
#include <boost/outcome.hpp>

namespace solidity::yul::interpreter
{

struct FunctionCallTrace
{
	FunctionDefinition const& definition;
	std::vector<u256> params;

	FunctionCallTrace(
		FunctionDefinition const& _definition,
		std::vector<u256> const& _params
	):
		definition(_definition),
		params(_params)
	{}
};

struct FunctionReturnTrace
{
	FunctionDefinition const& definition;
	std::vector<u256> returnedValues;

	FunctionReturnTrace(
		FunctionDefinition const& _definition,
		std::vector<u256> const& _returnedValues
	):
		definition(_definition),
		returnedValues(_returnedValues)
	{}
};

using LogTraceEntry = std::variant<FunctionCallTrace, FunctionReturnTrace>;

struct PureInterpreterConfig
{
	// set to 0 to disable tracing
	size_t maxTraceSize{};

	size_t maxSteps{};
	size_t maxExprNesting{};
	size_t maxRecursionDepth{};
};

struct PureInterpreterState
{
	PureInterpreterConfig const config;

	size_t numSteps = 0;
	std::vector<LogTraceEntry> traces = {};

	/// Add a log trace.
	/// Will do nothing if config.maxTraceSize == 0
	///	- the log entry will not be constructed in this case
	template<typename TraceType, typename... Args>
	BOOST_OUTCOME_V2_NAMESPACE::result<void, TraceLimitReached> addTrace(Args const&... _args)
	{
		namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

		if (config.maxTraceSize == 0) return outcome::success();
		if (traces.size() > config.maxTraceSize)
			return TraceLimitReached();
		traces.emplace_back(std::in_place_type<TraceType>, _args...);
		return outcome::success();
	}

	void dumpTraces(std::ostream& _out) const;
};

}
