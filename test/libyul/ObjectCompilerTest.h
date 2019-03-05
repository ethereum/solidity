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
}

namespace yul
{
namespace test
{

class ObjectCompilerTest: public dev::solidity::test::TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::unique_ptr<TestCase>(new ObjectCompilerTest(_config.filename));
	}

	explicit ObjectCompilerTest(std::string const& _filename);

	bool run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

	void printSource(std::ostream& _stream, std::string const &_linePrefix = "", bool const _formatted = false) const override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override;

private:
	void printIndented(std::ostream& _stream, std::string const& _output, std::string const& _linePrefix = "") const;
	bool parse(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted);
	void disambiguate();

	static void printErrors(std::ostream& _stream, langutil::ErrorList const& _errors);

	std::string m_source;
	bool m_optimize = false;
	std::string m_expectation;
	std::string m_obtainedResult;
};

}
}
