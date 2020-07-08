// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/CommonSyntaxTest.h>
#include <libyul/Dialect.h>

namespace solidity::yul::test
{

using solidity::test::SyntaxTestError;

class SyntaxTest: public solidity::test::CommonSyntaxTest
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<SyntaxTest>(_config.filename, _config.evmVersion);
	}
	static std::unique_ptr<TestCase> createErrorRecovery(Config const& _config)
	{
		return std::make_unique<SyntaxTest>(_config.filename, _config.evmVersion);
	}
	SyntaxTest(std::string const& _filename, langutil::EVMVersion _evmVersion);
	~SyntaxTest() override {}
protected:
	void parseAndAnalyze() override;

private:
	Dialect const* m_dialect = nullptr;
};

}
