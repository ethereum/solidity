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

#include <test/libsolidity/AnalysisFramework.h>
#include <libsolidity/interface/Exceptions.h>

#include <boost/noncopyable.hpp>
#include <boost/test/unit_test.hpp>

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

struct SyntaxTestExpectation
{
	std::string type;
	std::string message;
};


class SyntaxTest: AnalysisFramework
{
public:
	SyntaxTest(std::string const& _filename);

	bool run(std::ostream& _stream, std::string const& _indent);

	void printExpected(std::ostream& _stream, std::string const& _indent) const;
	void printErrorList(
		std::ostream& _stream,
		ErrorList const& _errors,
		std::string const& _indent
	) const;

	static int registerTests(
		boost::unit_test::test_suite& _suite,
		boost::filesystem::path const& _basepath,
		boost::filesystem::path const& _path
	);
private:
	bool matchesExpectations(ErrorList const& _errors) const;
	static std::string errorMessage(Error const& _e);
	static std::string parseSource(std::istream& _stream);
	static std::vector<SyntaxTestExpectation> parseExpectations(std::istream& _stream);

	std::string m_source;
	std::vector<SyntaxTestExpectation> m_expectations;
	ErrorList m_errorList;
};

}
}
}
