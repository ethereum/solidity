// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/TestCase.h>

#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/NameDispenser.h>

#include <libyul/YulString.h>

#include <set>
#include <memory>

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
struct Dialect;
}

namespace solidity::yul::test
{

class YulOptimizerTest: public solidity::frontend::test::EVMVersionRestrictedTestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<YulOptimizerTest>(_config.filename);
	}

	explicit YulOptimizerTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

private:
	bool parse(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted);
	void disambiguate();
	void updateContext();

	static void printErrors(std::ostream& _stream, langutil::ErrorList const& _errors);

	std::string m_optimizerStep;

	Dialect const* m_dialect = nullptr;
	std::set<YulString> m_reservedIdentifiers;
	std::unique_ptr<NameDispenser> m_nameDispenser;
	std::unique_ptr<OptimiserStepContext> m_context;

	std::shared_ptr<Block> m_ast;
	std::shared_ptr<AsmAnalysisInfo> m_analysisInfo;
};

}
