// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/libsolidity/SyntaxTest.h>

#include <libsmtutil/SolverInterface.h>

#include <string>

namespace solidity::frontend::test
{

class SMTCheckerTest: public SyntaxTest
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<SMTCheckerTest>(_config.filename, _config.evmVersion);
	}
	SMTCheckerTest(std::string const& _filename, langutil::EVMVersion _evmVersion);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false) override;

protected:
	/// This is set via option SMTSolvers in the test.
	/// The possible options are `all`, `z3`, `cvc4`, `none`,
	/// where if none is given the default used option is `all`.
	smtutil::SMTSolverChoice m_enabledSolvers;
};

}
