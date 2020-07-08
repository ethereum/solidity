// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolutil/AnsiColorized.h>
#include <test/TestCase.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

namespace solidity::frontend::test
{

class ASTJSONTest: public TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{ return std::make_unique<ASTJSONTest>(_config.filename); }
	ASTJSONTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

	void printSource(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) const override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override;
private:
	std::vector<std::pair<std::string, std::string>> m_sources;
	std::string m_expectationLegacy;
	std::string m_astFilename;
	std::string m_legacyAstFilename;
	std::string m_result;
	std::string m_resultLegacy;
};

}
