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

using FunctionCall = ExpectationParser::FunctionCall;
using Expectation = ExpectationParser::FunctionCallExpectations;

class SemanticTest: public SolidityExecutionFramework, public TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _options)
	{ return std::unique_ptr<TestCase>(new SemanticTest(_options.filename, _options.ipcPath)); }

	explicit SemanticTest(std::string const& _filename, std::string const& _ipcPath);

	bool run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;
	void printSource(std::ostream &_stream, std::string const& _linePrefix = "", bool const _formatted = false) const override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix = "") const override;

private:
	struct FunctionCallTest
	{
		FunctionCall call;
		bool status;
		bytes rawBytes;
		std::string output;
		bool matchesExpectation() const
		{
			auto expectedByteFormat = ExpectationParser::stringToBytes(call.expectations.raw);
			return status == call.expectations.status && rawBytes == expectedByteFormat.first;
		}
		void reset()
		{
			status = false;
			rawBytes = bytes{};
			output = std::string{};
		}
	};

	void parseExpectations(std::istream& _stream);
	bool deploy(std::string const& _contractName, u256 const& _value, bytes const& _arguments);

	void printFunctionCall(std::ostream& _stream, FunctionCall const& _call, std::string const& _linePrefix = "") const;
	void printFunctionCallTest(
		std::ostream& _stream,
		FunctionCallTest const& test,
		bool _expected,
		std::string const& _linePrefix = "",
		bool const _formatted = false
	) const;

	std::string m_source;
	std::map<std::string, dev::test::Address> m_libraryAddresses;
	std::vector<FunctionCallTest> m_tests;
};

}
}
}
