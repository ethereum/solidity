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

#include <test/libsolidity/OptimizedIRCachingTest.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/StringUtils.h>

using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

void OptimizedIRCachingTest::setupCompiler(CompilerStack& _compiler)
{
	AnalysisFramework::setupCompiler(_compiler);
	_compiler.setOptimiserSettings(true);
	_compiler.setViaIR(true);
}

TestCase::TestResult OptimizedIRCachingTest::run(std::ostream& _stream, std::string const& _linePrefix, bool _formatted)
{
	soltestAssert(compiler().objectOptimizer().size() == 0);

	if (!runFramework(m_source, PipelineStage::Compilation))
	{
		printPrefixed(_stream, formatErrors(filteredErrors(), _formatted), _linePrefix);
		return TestResult::FatalError;
	}

	m_obtainedResult = "cachedObjects: " + toString(compiler().objectOptimizer().size()) + "\n";
	return checkResult(_stream, _linePrefix, _formatted);
}
