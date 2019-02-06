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

#include <boost/filesystem.hpp>

#include <iosfwd>
#include <memory>
#include <string>

namespace dev
{
namespace solidity
{
namespace test
{

/** Common superclass of SyntaxTest and SemanticsTest. */
class TestCase
{
public:
	struct Config
	{
		std::string filename;
		std::string ipcPath;
	};

	using TestCaseCreator = std::unique_ptr<TestCase>(*)(Config const&);

	virtual ~TestCase() = default;

	/// Runs the test case.
	/// Outputs error messages to @arg _stream. Each line of output is prefixed with @arg _linePrefix.
	/// Optionally, color-coding can be enabled (if @arg _formatted is set to true).
	/// @returns true, if the test case succeeds, false otherwise
	virtual bool run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) = 0;

	/// Outputs the test contract to @arg _stream.
	/// Each line of output is prefixed with @arg _linePrefix.
	/// If @arg _formatted is true, color-coding may be used to indicate
	/// error locations in the contract, if applicable.
	virtual void printSource(std::ostream &_stream, std::string const &_linePrefix = "", bool const _formatted = false) const = 0;
	/// Outputs test expectations to @arg _stream that match the actual results of the test.
	/// Each line of output is prefixed with @arg _linePrefix.
	virtual void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const = 0;

	static bool isTestFilename(boost::filesystem::path const& _filename);

protected:
	static std::string parseSource(std::istream& _file);
	static void expect(std::string::iterator& _it, std::string::iterator _end, std::string::value_type _c);

	template<typename IteratorType>
	static void skipWhitespace(IteratorType& _it, IteratorType _end)
	{
		while (_it != _end && isspace(*_it))
			++_it;
	}

	template<typename IteratorType>
	static void skipSlashes(IteratorType& _it, IteratorType _end)
	{
		while (_it != _end && *_it == '/')
			++_it;
	}

};

}
}
}
