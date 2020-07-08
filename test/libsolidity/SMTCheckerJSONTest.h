// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/libsolidity/SyntaxTest.h>

#include <libsolutil/JSON.h>

#include <string>

namespace solidity::frontend::test
{

class SMTCheckerJSONTest: public SyntaxTest
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<SMTCheckerJSONTest>(_config.filename, _config.evmVersion);
	}
	SMTCheckerJSONTest(std::string const& _filename, langutil::EVMVersion _evmVersion);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false) override;

private:
	std::vector<std::string> hashesFromJson(Json::Value const& _jsonObj, std::string const& _auxInput, std::string const& _smtlib);
	Json::Value buildJson(std::string const& _extra);

	Json::Value m_smtResponses;
};

}
