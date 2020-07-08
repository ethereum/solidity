// SPDX-License-Identifier: GPL-3.0
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
