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

#include <test/libsolidity/FormattedScope.h>
#include <test/libsolidity/TestCase.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

namespace dev
{
namespace solidity
{
namespace test
{

class ASTJSONTest: public TestCase
{
public:
	static std::unique_ptr<TestCase> create(std::string const& _filename)
	{ return std::unique_ptr<TestCase>(new ASTJSONTest(_filename)); }
	ASTJSONTest(std::string const& _filename);

	bool run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

	void printSource(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) const override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override;
private:
	std::vector<std::pair<std::string, std::string>> m_sources;
	std::string m_expectation;
	std::string m_expectationLegacy;
	std::string m_astFilename;
	std::string m_legacyAstFilename;
	std::string m_result;
	std::string m_resultLegacy;
};

}
}
}
