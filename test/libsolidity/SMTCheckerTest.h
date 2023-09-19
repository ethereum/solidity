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

#include <libsolidity/formal/ModelChecker.h>

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

	void setupCompiler(CompilerStack& _compiler) override;
	void filterObtainedErrors() override;

	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override;

protected:
	/*
	Options that can be set in the test:
	SMTEngine: `all`, `chc`, `bmc`, `none`, where the default is `all`.
		Set in m_modelCheckerSettings.
	SMTIgnoreCex: `yes`, `no`, where the default is `no`.
		Set in m_ignoreCex.
	SMTIgnoreInv: `yes`, `no`, where the default is `no`.
		Set in m_modelCheckerSettings.
	SMTShowUnproved: `yes`, `no`, where the default is `yes`.
		Set in m_modelCheckerSettings.
	SMTSolvers: `all`, `cvc4`, `z3`, `none`, where the default is `all`.
		Set in m_modelCheckerSettings.
	BMCLoopIterations: number of loop iterations for BMC engine, the default is 1.
		Set in m_modelCheckerSettings.
	*/

	ModelCheckerSettings m_modelCheckerSettings;

	bool m_ignoreCex = false;

	std::vector<SyntaxTestError> m_unfilteredErrorList;
};

}
