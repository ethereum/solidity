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

#include <libyul/tools/interpreter/PureInterpreterState.h>
#include <libyul/tools/interpreter/types.h>
#include <libyul/tools/interpreter/Results.h>

namespace solidity::yul
{
struct AsmAnalysisInfo;
class AST;
}

namespace solidity::yul::test
{

class YulPureInterpreterTest: public solidity::frontend::test::EVMVersionRestrictedTestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<YulPureInterpreterTest>(_config.filename);
	}

	explicit YulPureInterpreterTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

private:
	bool parse(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted);
	std::string interpret() const;
	void dumpExecutionData(
		std::ostream& _stream,
		tools::interpreter::ExecutionResult _res,
		tools::interpreter::PureInterpreterState const& _state,
		tools::interpreter::VariableValuesMap const& _outterMostVariables
	) const;
	void dumpExecutionResult(std::ostream& _stream, tools::interpreter::ExecutionResult _res) const;
	void dumpVariables(std::ostream& _stream, tools::interpreter::VariableValuesMap const& _variables) const;
	void dumpValue(std::ostream& _stream, u256 _value) const;

	std::shared_ptr<AST const> m_ast;
	std::shared_ptr<AsmAnalysisInfo const> m_analysisInfo;
	tools::interpreter::PureInterpreterConfig m_config;

	bool m_printHex;
};

}
