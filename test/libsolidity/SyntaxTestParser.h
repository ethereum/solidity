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

struct SyntaxTest
{
	std::string source;
	std::vector<SyntaxTestExpectation> expectations;
};

class SyntaxTestParser
{
public:
	SyntaxTestParser() = default;

	SyntaxTest parse(std::string const& _filename);
private:
	std::string parseSource(std::istream& _stream);
	std::vector<SyntaxTestExpectation> parseExpectations(std::istream& _stream);
};

}
}
}
