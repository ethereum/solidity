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

#include <libdevcore/CommonData.h>
#include <libsolidity/ast/Types.h>

#include <iosfwd>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

namespace dev
{
namespace solidity
{
namespace test
{

class ExpectationParser
{
public:
	struct FunctionCallResult {
		std::string raw;
		bytes output;
		bool status = true;
		std::string comment;
	};

	struct FunctionCallArgs {
		std::string raw;
		bytes input;
		std::string comment;
	};

	struct FunctionCall
	{
		std::string signature;
		FunctionCallArgs arguments;
		FunctionCallResult result;
		u256 costs;
	};

	static std::string bytesToString(bytes const& _bytes);
	static bytes stringToBytes(std::string _string);

	ExpectationParser(std::istream& _stream): m_stream(_stream) {}

	std::vector<FunctionCall> parseFunctionCalls();

private:
	std::string parseFunctionCallSignature();
	FunctionCallArgs parseFunctionCallArgument();
	FunctionCallResult parseFunctionCallResult();
	u256 parseFunctionCallCosts();

	bool advanceLine();
	bool endOfLine();

	std::istream& m_stream;
	std::string m_line;
	std::string::iterator m_char;

};

}
}
}
