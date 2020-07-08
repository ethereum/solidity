// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolutil/AnsiColorized.h>
#include <test/TestCase.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

namespace solidity::yul::test
{

class FunctionSideEffects: public solidity::frontend::test::TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{ return std::make_unique<FunctionSideEffects>(_config.filename); }
	explicit FunctionSideEffects(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;
};

}
