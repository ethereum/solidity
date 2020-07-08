// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/libsolidity/AnalysisFramework.h>
#include <test/TestCase.h>
#include <liblangutil/Exceptions.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

namespace solidity::frontend::test
{

class GasTest: AnalysisFramework, public TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{ return std::make_unique<GasTest>(_config.filename); }
	GasTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false) override;

	void printSource(std::ostream &_stream, std::string const &_linePrefix = "", bool _formatted = false) const override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override;

private:
	void parseExpectations(std::istream& _stream);

	bool m_optimise = false;
	bool m_optimiseYul = false;
	size_t m_optimiseRuns = 200;
	std::map<std::string, std::map<std::string, std::string>> m_expectations;
};

}
