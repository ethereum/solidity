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

#include <libsolutil/Profiler.h>

#include <fmt/format.h>

#include <algorithm>
#include <iostream>
#include <vector>

using namespace std::chrono;
using namespace solidity;

#ifdef PROFILE_OPTIMIZER_STEPS

util::Profiler::Probe::Probe(std::string _scopeName):
	m_scopeName(std::move(_scopeName)),
	m_startTime(steady_clock::now())
{
}

util::Profiler::Probe::~Probe()
{
	steady_clock::time_point endTime = steady_clock::now();

	auto [metricsIt, inserted] = Profiler::singleton().m_metrics.try_emplace(m_scopeName, Metrics{0us, 0});
	metricsIt->second.durationInMicroseconds += duration_cast<microseconds>(endTime - m_startTime);
	++metricsIt->second.callCount;
}

util::Profiler::~Profiler()
{
	outputPerformanceMetrics();
}

util::Profiler& util::Profiler::singleton()
{
	static Profiler profiler;
	return profiler;
}

void util::Profiler::outputPerformanceMetrics()
{
	std::vector<std::pair<std::string, Metrics>> sortedMetrics(m_metrics.begin(), m_metrics.end());
	std::sort(
		sortedMetrics.begin(),
		sortedMetrics.end(),
		[](std::pair<std::string, Metrics> const& _lhs, std::pair<std::string, Metrics> const& _rhs) -> bool
		{
			return _lhs.second.durationInMicroseconds < _rhs.second.durationInMicroseconds;
		}
	);

	std::chrono::microseconds totalDurationInMicroseconds = 0us;
	size_t totalCallCount = 0;
	for (auto&& [scopeName, scopeMetrics]: sortedMetrics)
	{
		totalDurationInMicroseconds += scopeMetrics.durationInMicroseconds;
		totalCallCount += scopeMetrics.callCount;
	}

	std::cerr << "PERFORMANCE METRICS FOR PROFILED SCOPES\n\n";
	std::cerr << "| Time % | Time       | Calls   | Scope                          |\n";
	std::cerr << "|-------:|-----------:|--------:|--------------------------------|\n";

	double totalDurationInSeconds = duration_cast<duration<double>>(totalDurationInMicroseconds).count();
	for (auto&& [scopeName, scopeMetrics]: sortedMetrics)
	{
		double durationInSeconds = duration_cast<duration<double>>(scopeMetrics.durationInMicroseconds).count();
		double percentage = 100.0 * durationInSeconds / totalDurationInSeconds;
		std::cerr << fmt::format(
			"| {:5.1f}% | {:8.3f} s | {:7} | {:30} |\n",
			percentage,
			durationInSeconds,
			scopeMetrics.callCount,
			scopeName
		);
	}
	std::cerr << fmt::format("| {:5.1f}% | {:8.3f} s | {:7} | {:30} |\n", 100.0, totalDurationInSeconds, totalCallCount, "**TOTAL**");
}

#endif
