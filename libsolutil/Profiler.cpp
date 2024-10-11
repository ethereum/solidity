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
	int64_t durationInMicroseconds = duration_cast<microseconds>(endTime - m_startTime).count();

	auto [metricsIt, inserted] = Profiler::singleton().m_metrics.try_emplace(m_scopeName, Metrics{0, 0});
	metricsIt->second.durationInMicroseconds += durationInMicroseconds;
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

	int64_t totalDurationInMicroseconds = 0;
	size_t totalCallCount = 0;
	for (auto&& [scopeName, scopeMetrics]: sortedMetrics)
	{
		totalDurationInMicroseconds += scopeMetrics.durationInMicroseconds;
		totalCallCount += scopeMetrics.callCount;
	}

	std::cerr << "Performance metrics for profiled scopes" << std::endl;
	std::cerr << "=======================================" << std::endl;
	constexpr double microsecondsInSecond = 1000000;
	for (auto&& [scopeName, scopeMetrics]: sortedMetrics)
	{
		double percentage = 100.0 * static_cast<double>(scopeMetrics.durationInMicroseconds) / static_cast<double>(totalDurationInMicroseconds);
		double durationInSeconds = static_cast<double>(scopeMetrics.durationInMicroseconds) / microsecondsInSecond;
		std::cerr << fmt::format(
			"{:>7.3f}% ({} s, {} calls): {}",
			percentage,
			durationInSeconds,
			scopeMetrics.callCount,
			scopeName
		) << std::endl;
	}
	double totalDurationInSeconds = static_cast<double>(totalDurationInMicroseconds) / microsecondsInSecond;
	std::cerr << "--------------------------------------" << std::endl;
	std::cerr << fmt::format("{:>7}% ({:.3f} s, {} calls)", 100, totalDurationInSeconds, totalCallCount) << std::endl;
}

#endif
