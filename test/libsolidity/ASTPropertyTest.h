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

#include <libsolutil/JSON.h>

#include <string>
#include <vector>

namespace solidity::frontend
{
class CompilerStack;
}

namespace solidity::frontend::test
{

using StringPair = std::pair<std::string, std::string>;

class ASTPropertyTest: public TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<ASTPropertyTest>(_config.filename);
	}
	ASTPropertyTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

private:
	struct Test
	{
		std::string property;
		std::string expectedValue;
		std::string obtainedValue;
	};

	void readExpectations();
	std::vector<StringPair> readKeyValuePairs(std::string const& _input);
	void extractTestsFromAST(Json::Value const& _astJson);
	std::string formatExpectations(bool _obtainedResult = true);

	std::vector<std::string> m_testOrder;
	std::map<std::string, Test> m_tests;
};

}
