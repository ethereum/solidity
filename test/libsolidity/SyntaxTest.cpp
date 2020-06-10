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

#include <test/libsolidity/SyntaxTest.h>
#include <test/Common.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/throw_exception.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace boost::unit_test;
namespace fs = boost::filesystem;

SyntaxTest::SyntaxTest(string const& _filename, langutil::EVMVersion _evmVersion, bool _parserErrorRecovery): CommonSyntaxTest(_filename, _evmVersion)
{
	m_optimiseYul = m_reader.boolSetting("optimize-yul", true);
	m_parserErrorRecovery = _parserErrorRecovery;
}

TestCase::TestResult SyntaxTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	setupCompiler();
	parseAndAnalyze();
	filterObtainedErrors();

	return conclude(_stream, _linePrefix, _formatted);
}

void SyntaxTest::setupCompiler()
{
	string const preamble = "pragma solidity >=0.0;\n// SPDX-License-Identifier: GPL-3.0\n";
	compiler().reset();
	auto sourcesWithPragma = m_sources;
	for (auto& source: sourcesWithPragma)
		source.second = preamble + source.second;
	compiler().setSources(sourcesWithPragma);
	compiler().setEVMVersion(m_evmVersion);
	compiler().setParserErrorRecovery(m_parserErrorRecovery);
	compiler().setOptimiserSettings(
		m_optimiseYul ?
		OptimiserSettings::full() :
		OptimiserSettings::minimal()
	);
}

void SyntaxTest::parseAndAnalyze()
{
	if (compiler().parse() && compiler().analyze())
		try
		{
			if (!compiler().compile())
				BOOST_THROW_EXCEPTION(runtime_error("Compilation failed even though analysis was successful."));
		}
		catch (UnimplementedFeatureError const& _e)
		{
			m_errorList.emplace_back(SyntaxTestError{
				"UnimplementedFeatureError",
				errorMessage(_e),
				"",
				-1,
				-1
			});
		}
}

void SyntaxTest::filterObtainedErrors()
{
	string const preamble = "pragma solidity >=0.0;\n// SPDX-License-Identifier: GPL-3.0\n";
	for (auto const& currentError: filterErrors(compiler().errors(), true))
	{
		int locationStart = -1, locationEnd = -1;
		string sourceName;
		if (auto location = boost::get_error_info<errinfo_sourceLocation>(*currentError))
		{
			// ignore the version & license pragma inserted by the testing tool when calculating locations.
			if (location->start >= static_cast<int>(preamble.size()))
				locationStart = location->start - static_cast<int>(preamble.size());
			if (location->end >= static_cast<int>(preamble.size()))
				locationEnd = location->end - static_cast<int>(preamble.size());
			if (location->source)
				sourceName = location->source->name();
		}
		m_errorList.emplace_back(SyntaxTestError{
			currentError->typeName(),
			errorMessage(*currentError),
			sourceName,
			locationStart,
			locationEnd
		});
	}
}

