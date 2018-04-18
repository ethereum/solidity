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
#include <test/libsolidity/FormattedScope.h>
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

struct SyntaxTestError
{
	std::string type;
	std::string message;
	int locationStart;
	int locationEnd;
	bool operator==(SyntaxTestError const& _rhs) const
	{
		return type == _rhs.type &&
			message == _rhs.message &&
			locationStart == _rhs.locationStart &&
			locationEnd == _rhs.locationEnd;
	}
};


class SyntaxTest: AnalysisFramework
{
public:
	SyntaxTest(std::string const& _filename);

	bool run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false);

	std::vector<SyntaxTestError> const& expectations() const { return m_expectations; }
	std::string const& source() const { return m_source; }
	std::vector<SyntaxTestError> const& errorList() const { return m_errorList; }

	static void printErrorList(
		std::ostream& _stream,
		std::vector<SyntaxTestError> const& _errors,
		std::string const& _linePrefix,
		bool const _formatted = false
	);

	static int registerTests(
		boost::unit_test::test_suite& _suite,
		boost::filesystem::path const& _basepath,
		boost::filesystem::path const& _path
	);
	static bool isTestFilename(boost::filesystem::path const& _filename);
	static std::string errorMessage(Exception const& _e);
private:
	static std::string parseSource(std::istream& _stream);
	static std::vector<SyntaxTestError> parseExpectations(std::istream& _stream);

	std::string m_source;
	std::vector<SyntaxTestError> m_expectations;
	std::vector<SyntaxTestError> m_errorList;
};

}
}
}
