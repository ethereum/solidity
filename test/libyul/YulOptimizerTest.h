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

namespace solidity::langutil
{
class Error;
using ErrorList = std::vector<std::shared_ptr<Error const>>;
}

namespace solidity::yul
{
struct AsmAnalysisInfo;
struct Object;
struct Dialect;
}

namespace solidity::yul::test
{

class YulOptimizerTest: public solidity::frontend::test::EVMVersionRestrictedTestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<YulOptimizerTest>(_config.filename);
	}

	explicit YulOptimizerTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;
private:
	std::pair<std::shared_ptr<Object>, std::shared_ptr<AsmAnalysisInfo>> parse(
		std::ostream& _stream, std::string const& _linePrefix, bool const _formatted, std::string const& _source
	);
	static void printErrors(std::ostream& _stream, langutil::ErrorList const& _errors);

	std::string m_optimizerStep;

	Dialect const* m_dialect = nullptr;

	std::shared_ptr<Object> m_object;
	std::shared_ptr<AsmAnalysisInfo> m_analysisInfo;
};

}
