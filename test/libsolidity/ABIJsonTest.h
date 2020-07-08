// SPDX-License-Identifier: GPL-3.0
/**
 * Unit tests for the solidity compiler ABI JSON Interface output.
 */

#pragma once

#include <test/TestCase.h>

#include <string>

namespace solidity::frontend::test
{


class ABIJsonTest: public TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{ return std::make_unique<ABIJsonTest>(_config.filename); }
	ABIJsonTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;
};

}
