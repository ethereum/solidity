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

#include <test/libyul/SyntaxTest.h>

#include <test/libyul/Common.h>
#include <test/TestCaseReader.h>

#include <test/libsolidity/util/SoltestErrors.h>

#include <test/Common.h>

#include <libyul/YulStack.h>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::test;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

void SyntaxTest::parseAndAnalyze()
{
	solUnimplementedAssert(m_sources.size() == 1, "Multi-source Yul tests are not supported.");
	auto const& [sourceUnitName, source] = *m_sources.begin();

	YulStack yulStack = parseYul(source);
	if (!yulStack.hasErrors())
	{
		// Assemble the object so that we can test CodeGenerationErrors too.
		yulStack.optimize();
		yulStack.assemble(YulStack::Machine::EVM);
	}
	for (auto const& error: yulStack.errors())
	{
		int locationStart = -1;
		int locationEnd = -1;

		if (SourceLocation const* location = error->sourceLocation())
		{
			locationStart = location->start;
			locationEnd = location->end;
		}

		m_errorList.emplace_back(SyntaxTestError{
			error->type(),
			error->errorId(),
			errorMessage(*error),
			sourceUnitName,
			locationStart,
			locationEnd
		});
	}
}

SyntaxTest::SyntaxTest(std::string const& _filename, langutil::EVMVersion _evmVersion):
	CommonSyntaxTest(_filename, _evmVersion)
{
	std::string dialectName = m_reader.stringSetting("dialect", "evm");
	soltestAssert(dialectName == "evm"); // We only have one dialect now
}
