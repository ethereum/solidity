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

#include <libyul/ASTForward.h>

#include <libsolutil/Exceptions.h>
#include <libsolutil/Numeric.h>

#include <cstddef>
#include <variant>
#include <vector>

namespace solidity::yul::tools::interpreter
{

class TraceLimitReached: public util::Exception
{
};

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
	size_t maxTraceSize = 0;

	size_t maxSteps = 0;
	size_t maxExprNesting = 0;
	size_t maxRecursionDepth = 0;
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
	void addTrace(const Args&... args)
	{
		if (config.maxTraceSize == 0) return;
		if (traces.size() > config.maxTraceSize)
			BOOST_THROW_EXCEPTION(TraceLimitReached());
		traces.emplace_back(std::in_place_type<TraceType>, args...);
	}

	void dumpTraces(std::ostream& _out) const;
};

}
