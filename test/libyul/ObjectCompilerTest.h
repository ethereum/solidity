// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/TestCase.h>

namespace solidity::langutil
{
class Scanner;
class Error;
using ErrorList = std::vector<std::shared_ptr<Error const>>;
}

namespace solidity::yul
{
struct AsmAnalysisInfo;
struct Block;
}

namespace solidity::yul::test
{

class ObjectCompilerTest: public solidity::frontend::test::TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<ObjectCompilerTest>(_config.filename);
	}

	explicit ObjectCompilerTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

private:
	bool parse(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted);
	void disambiguate();

	static void printErrors(std::ostream& _stream, langutil::ErrorList const& _errors);

	bool m_optimize = false;
};

}
