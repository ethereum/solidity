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

#include <test/libsolidity/SyntaxTest.h>

#include <libsmtutil/SolverInterface.h>

#include <string>

namespace solidity::frontend::test
{

class SMTCheckerTest: public SyntaxTest
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<SMTCheckerTest>(_config.filename);
	}
	SMTCheckerTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false) override;

protected:
	/// This is set via option SMTSolvers in the test.
	/// The possible options are `all`, `z3`, `cvc4`, `none`,
	/// where if none is given the default used option is `all`.
	smtutil::SMTSolverChoice m_enabledSolvers;
};

}
