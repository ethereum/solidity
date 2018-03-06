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

#include <test/libsolidity/SyntaxTestParser.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/throw_exception.hpp>
#include <cctype>
#include <fstream>
#include <stdexcept>

using namespace dev;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;

template<typename IteratorType>
void skipWhitespace(IteratorType& it, IteratorType end)
{
	while (it != end && isspace(*it))
		++it;
}

template<typename IteratorType>
void skipSlashes(IteratorType& it, IteratorType end)
{
	while (it != end && *it == '/')
		++it;
}

std::string SyntaxTestParser::parseSource(std::istream& _stream)
{
	std::string source;
	string line;
	string const delimiter("// ----");
	while (getline(_stream, line))
		if (boost::algorithm::starts_with(line, delimiter))
			break;
		else
			source += line + "\n";
	return source;
}

std::vector<SyntaxTestExpectation> SyntaxTestParser::parseExpectations(std::istream& _stream)
{
	std::vector<SyntaxTestExpectation> expectations;
	std::string line;
	while (getline(_stream, line))
	{
		auto it = line.begin();

		skipSlashes(it, line.end());
		skipWhitespace(it, line.end());

		if (it == line.end()) continue;

		auto typeBegin = it;
		while (it != line.end() && *it != ':')
			++it;
		string errorType(typeBegin, it);

		// skip colon
		if (it != line.end()) it++;

		skipWhitespace(it, line.end());

		string errorMessage(it, line.end());
		expectations.emplace_back(SyntaxTestExpectation{move(errorType), move(errorMessage)});
	}
	return expectations;
}

SyntaxTest SyntaxTestParser::parse(string const& _filename)
{
	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("cannot open test contract: \"" + _filename + "\""));
	file.exceptions(ios::badbit);

	SyntaxTest result;
	result.source = parseSource(file);
	result.expectations = parseExpectations(file);
	return result;
}
