// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/libsolidity/AnalysisFramework.h>
#include <test/TestCase.h>
#include <liblangutil/Exceptions.h>
#include <libsolutil/AnsiColorized.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

namespace solidity::test
{

struct SyntaxTestError
{
	std::string type;
	std::optional<langutil::ErrorId> errorId;
	std::string message;
	std::string sourceName;
	int locationStart = -1;
	int locationEnd = -1;
	bool operator==(SyntaxTestError const& _rhs) const
	{
		return type == _rhs.type &&
			errorId == _rhs.errorId &&
			message == _rhs.message &&
			sourceName == _rhs.sourceName &&
			locationStart == _rhs.locationStart &&
			locationEnd == _rhs.locationEnd;
	}
};


class CommonSyntaxTest: public frontend::test::EVMVersionRestrictedTestCase
{
public:
	CommonSyntaxTest(std::string const& _filename, langutil::EVMVersion _evmVersion);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false) override;

	void printSource(std::ostream& _stream, std::string const &_linePrefix = "", bool _formatted = false) const override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override
	{
		if (!m_errorList.empty())
			printErrorList(_stream, m_errorList, _linePrefix, false);
	}

	static std::string errorMessage(util::Exception const& _e);
protected:
	virtual void parseAndAnalyze() = 0;

	static void printErrorList(
		std::ostream& _stream,
		std::vector<SyntaxTestError> const& _errors,
		std::string const& _linePrefix,
		bool _formatted = false
	);

	TestResult conclude(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false);
	void printExpectationAndError(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false);

	static std::vector<SyntaxTestError> parseExpectations(std::istream& _stream);

	std::map<std::string, std::string> m_sources;
	std::vector<SyntaxTestError> m_expectations;
	std::vector<SyntaxTestError> m_errorList;
	langutil::EVMVersion const m_evmVersion;
};

}
