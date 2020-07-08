// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/TestCase.h>
#include <libyul/Object.h>

namespace solidity::langutil
{
class Scanner;
class Error;
using ErrorList = std::vector<std::shared_ptr<Error const>>;
}

namespace solidity::yul::test
{

class EwasmTranslationTest: public solidity::frontend::test::EVMVersionRestrictedTestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<EwasmTranslationTest>(_config.filename);
	}

	explicit EwasmTranslationTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

private:
	bool parse(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted);
	std::string interpret();

	static void printErrors(std::ostream& _stream, langutil::ErrorList const& _errors);

	std::shared_ptr<Object> m_object;
};

}
