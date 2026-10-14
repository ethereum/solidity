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

#include <liblangutil/EVMVersion.h>

#include <memory>

namespace solidity::yul::test::ssa
{

/// Parses a textual SSA CFG and expects either its reprinted form or the parse error.
/// Fails if reprinting is not stable under another parse.
class ParserTest: public frontend::test::TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config);
	ParserTest(std::string const& _filename, langutil::EVMVersion _evmVersion);
	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false) override;

private:
	langutil::EVMVersion m_evmVersion;
};

}
