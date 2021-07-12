/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <test/TestCase.h>

namespace solidity::yul
{
struct Dialect;

namespace test
{

class StackLayoutGeneratorTest: public solidity::frontend::test::TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<StackLayoutGeneratorTest>(_config.filename);
	}
	explicit StackLayoutGeneratorTest(std::string const& _filename);
	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;
private:
	Dialect const* m_dialect = nullptr;
};
}
}
