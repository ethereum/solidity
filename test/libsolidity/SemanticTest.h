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

#include <test/libsolidity/ExpectationParser.h>
#include <test/libsolidity/FormattedScope.h>
#include <test/libsolidity/SolidityExecutionFramework.h>
#include <test/libsolidity/AnalysisFramework.h>
#include <test/TestCase.h>
#include <liblangutil/Exceptions.h>

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

class SemanticTest: public SolidityExecutionFramework, public TestCase
{
public:
	static std::unique_ptr<TestCase> create(std::string const& _filename)
	{ return std::unique_ptr<TestCase>(new SemanticTest(_filename)); }
	SemanticTest(std::string const& _filename);

	virtual bool run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

	virtual void printSource(std::ostream &_stream, std::string const &_linePrefix = "", bool const _formatted = false) const override;
	virtual void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override;

	virtual bool allowsUpdate() const override { return false; }

	static std::string ipcPath;

private:
	void parseExpectations(std::istream& _stream);
	bool deploy(std::string const& _contractName, u256 const& _value, bytes const& _arguments);
	void printCalls(bool _actualResults, std::ostream& _stream, std::string const& _linePrefix, bool const _formatted) const;

	std::string m_source;
	std::vector<ExpectationParser::FunctionCall> m_calls;
	std::map<std::string, dev::test::Address> m_libraryAddresses;
	std::vector<std::pair<bool, bytes>> m_results;
};

}
}
}
