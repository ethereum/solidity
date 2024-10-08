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

	auto [durationIt, inserted] = Profiler::singleton().m_durations.try_emplace(m_scopeName, 0);
	durationIt->second += durationInMicroseconds;
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
	std::vector<std::pair<std::string, int64_t>> durations(m_durations.begin(), m_durations.end());
	std::sort(
		durations.begin(),
		durations.end(),
		[](std::pair<std::string, int64_t> const& _lhs, std::pair<std::string, int64_t> const& _rhs) -> bool
		{
			return _lhs.second < _rhs.second;
		}
	);

	int64_t totalDurationInMicroseconds = 0;
	for (auto&& [scopeName, durationInMicroseconds]: durations)
		totalDurationInMicroseconds += durationInMicroseconds;

	std::cerr << "Performance metrics for profiled scopes" << std::endl;
	std::cerr << "=======================================" << std::endl;
	constexpr double microsecondsInSecond = 1000000;
	for (auto&& [scopeName, durationInMicroseconds]: durations)
	{
		double percentage = 100.0 * static_cast<double>(durationInMicroseconds) / static_cast<double>(totalDurationInMicroseconds);
		double durationInSeconds = static_cast<double>(durationInMicroseconds) / microsecondsInSecond;
		std::cerr << fmt::format("{:>7.3f}% ({} s): {}", percentage, durationInSeconds, scopeName) << std::endl;
	}
	double totalDurationInSeconds = static_cast<double>(totalDurationInMicroseconds) / microsecondsInSecond;
	std::cerr << "--------------------------------------" << std::endl;
	std::cerr << fmt::format("{:>7}% ({:.3f} s)", 100, totalDurationInSeconds) << std::endl;
}

#endif
