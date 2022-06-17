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

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>

#include <liblangutil/EVMVersion.h>
#include <liblangutil/Exceptions.h>

#include <test/libyul/Common.h>
#include <test/libyul/SyntaxTest.h>
#include <test/TestCaseReader.h>

#include <test/libsolidity/util/SoltestErrors.h>

#include <test/Common.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul::test;
using namespace solidity::frontend::test;

void SyntaxTest::parseAndAnalyze()
{
	if (m_sources.sources.size() != 1)
		BOOST_THROW_EXCEPTION(runtime_error{"Expected only one source for yul test."});

	string const& name = m_sources.sources.begin()->first;
	string const& source = m_sources.sources.begin()->second;

	ErrorList errorList{};
	soltestAssert(m_dialect, "");
	// Silently ignoring the results.
	yul::test::parse(source, *m_dialect, errorList);
	for (auto const& error: errorList)
	{
		int locationStart = -1;
		int locationEnd = -1;

		if (SourceLocation const* location = error->sourceLocation())
		{
			locationStart = location->start;
			locationEnd = location->end;
		}

		m_errorList.emplace_back(SyntaxTestError{
			Error::formatErrorType(error->type()),
			error->errorId(),
			errorMessage(*error),
			name,
			locationStart,
			locationEnd
		});
	}

}

SyntaxTest::SyntaxTest(string const& _filename, langutil::EVMVersion _evmVersion):
	CommonSyntaxTest(_filename, _evmVersion)
{
	string dialectName = m_reader.stringSetting("dialect", "evmTyped");
	m_dialect = &dialect(dialectName, solidity::test::CommonOptions::get().evmVersion());
}
