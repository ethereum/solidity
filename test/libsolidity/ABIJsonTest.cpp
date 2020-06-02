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
/**
 * Unit tests for the solidity compiler ABI JSON Interface output.
 */

#include <test/libsolidity/ABIJsonTest.h>

#include <test/Common.h>

#include <libsolidity/interface/CompilerStack.h>
#include <libsolutil/JSON.h>
#include <libsolutil/AnsiColorized.h>

#include <fstream>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

ABIJsonTest::ABIJsonTest(string const& _filename):
	TestCase(_filename)
{
	m_source = m_reader.source();
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult ABIJsonTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	CompilerStack compiler;

	compiler.setSources({{
		"",
		"pragma solidity >=0.0;\n// SPDX-License-Identifier: GPL-3.0\n" + m_source
	}});
	compiler.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
	compiler.setOptimiserSettings(solidity::test::CommonOptions::get().optimize);
	if (!compiler.parseAndAnalyze())
		BOOST_THROW_EXCEPTION(runtime_error("Parsing contract failed"));

	m_obtainedResult.clear();
	bool first = true;
	for (string const& contractName: compiler.contractNames())
	{
		if (!first)
			m_obtainedResult += "\n\n";
		m_obtainedResult += "    " + contractName + "\n";
		m_obtainedResult += jsonPrettyPrint(compiler.contractABI(contractName)) + "\n";
		first = false;
	}

	return checkResult(_stream, _linePrefix, _formatted);
}
