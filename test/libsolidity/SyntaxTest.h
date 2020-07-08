// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/libsolidity/AnalysisFramework.h>
#include <test/TestCase.h>
#include <test/CommonSyntaxTest.h>
#include <liblangutil/Exceptions.h>
#include <libsolutil/AnsiColorized.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

namespace solidity::frontend::test
{

using solidity::test::SyntaxTestError;

class SyntaxTest: public AnalysisFramework, public solidity::test::CommonSyntaxTest
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<SyntaxTest>(_config.filename, _config.evmVersion, false);
	}
	static std::unique_ptr<TestCase> createErrorRecovery(Config const& _config)
	{
		return std::make_unique<SyntaxTest>(_config.filename, _config.evmVersion, true);
	}
	SyntaxTest(std::string const& _filename, langutil::EVMVersion _evmVersion, bool _parserErrorRecovery = false);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false) override;

protected:
	void setupCompiler();
	void parseAndAnalyze() override;
	void filterObtainedErrors();

	bool m_optimiseYul = true;
	bool m_parserErrorRecovery = false;
};

}
