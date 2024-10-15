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

#include <chrono>
#include <map>
#include <optional>
#include <string>

#ifdef PROFILE_OPTIMIZER_STEPS
#define PROFILER_PROBE(_scopeName, _variable) solidity::util::Profiler::Probe _variable(_scopeName);
#else
#define PROFILER_PROBE(_scopeName, _variable) void(0);
#endif

namespace solidity::util
{

#ifdef PROFILE_OPTIMIZER_STEPS

/// Simpler profiler class that gathers metrics during program execution and prints them out on exit.
///
/// To gather metrics, create a Probe instance and let it live until the end of the scope.
/// The probe will register its creation and destruction time and store the results in the profiler
/// singleton.
///
/// Use the PROFILER_PROBE macro to create probes conditionally, in a way that will not affect performance
/// unless profiling is enabled at compilation time via PROFILE_OPTIMIZER_STEPS CMake option.
///
/// Scopes are identified by the name supplied to the probe. Using the same name multiple times
/// will result in metrics for those scopes being aggregated together as if they were the same scope.
class Profiler
{
public:
	class Probe
	{
	public:
		Probe(std::string _scopeName);
		~Probe();

	private:
		std::string m_scopeName;
		std::chrono::steady_clock::time_point m_startTime;
	};

	static Profiler& singleton();

private:
	~Profiler();

	struct Metrics
	{
		std::chrono::microseconds durationInMicroseconds;
		size_t callCount;
	};

	/// Summarizes gathered metric and prints a report to standard error output.
	void outputPerformanceMetrics();

	std::map<std::string, Metrics> m_metrics;
};

#endif

}
