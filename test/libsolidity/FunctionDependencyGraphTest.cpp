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

#include <test/libsolidity/FunctionDependencyGraphTest.h>

#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/analysis/FunctionDependencyAnalysis.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/FunctionCallFinder.h>

#include <fstream>
#include <stdexcept>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

TestCase::TestResult FunctionDependencyGraphTest::run(std::ostream& _stream, std::string const& _linePrefix, bool _formatted)
{
	compiler().reset();
	compiler().setSources(StringMap{{"", m_source}});
	compiler().setViaIR(true);
	compiler().setOptimiserSettings(OptimiserSettings::none());
	if (!compiler().compile(CompilerStack::AnalysisSuccessful))
	{
		_stream << formatErrors(filteredErrors(), _formatted);
		return TestResult::FatalError;
	}

	m_obtainedResult.clear();
	for (auto [top, subs]: compiler().experimentalAnalysis().annotation<experimental::FunctionDependencyAnalysis>().functionCallGraph.edges)
	{
		std::string topName = top->name().empty() ? "fallback" : top->name();
		m_obtainedResult += "(" + topName + ") --> {";
		for (auto sub: subs)
			m_obtainedResult += sub->name() + ",";
		m_obtainedResult += "}\n";
	}

	return checkResult(_stream, _linePrefix, _formatted);
}
