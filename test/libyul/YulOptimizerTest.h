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

#pragma once

#include <test/TestCase.h>

namespace langutil
{
class Scanner;
class Error;
using ErrorList = std::vector<std::shared_ptr<Error const>>;
}

namespace yul
{
struct AsmAnalysisInfo;
struct Block;
struct Dialect;
}

namespace yul
{
namespace test
{

class YulOptimizerTest: public dev::solidity::test::EVMVersionRestrictedTestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::unique_ptr<TestCase>(new YulOptimizerTest(_config.filename));
	}

	explicit YulOptimizerTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

	void printSource(std::ostream& _stream, std::string const &_linePrefix = "", bool const _formatted = false) const override;
	void printUpdatedSettings(std::ostream &_stream, std::string const &_linePrefix = "", bool const _formatted = false) override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override;

private:
	void printIndented(std::ostream& _stream, std::string const& _output, std::string const& _linePrefix = "") const;
	bool parse(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted);
	void disambiguate();

	static void printErrors(std::ostream& _stream, langutil::ErrorList const& _errors);

	std::string m_source;
	bool m_yul = false;
	std::string m_optimizerStep;
	std::string m_expectation;

	std::shared_ptr<Dialect> m_dialect;
	std::shared_ptr<Block> m_ast;
	std::shared_ptr<AsmAnalysisInfo> m_analysisInfo;
	std::string m_obtainedResult;
};

}
}
