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
	struct ByteFormat
	{
		enum Type {
			UnsignedDec,
			SignedDec
		};
		Type type;
	};

	struct FunctionCallExpectations
	{
		std::string raw;
		bytes rawBytes;
		ByteFormat format;
		bool status = true;
		std::string output;
		std::string comment;
	};

	struct FunctionCallArgs
	{
		std::string raw;
		bytes rawBytes;
		ByteFormat format;
		std::string comment;
	};

	struct FunctionCall
	{
		std::string signature;
		FunctionCallArgs arguments;
		FunctionCallExpectations expectations;
		u256 value;
	};

	static std::string bytesToString(bytes const& _bytes, ByteFormat const& _format);
	static std::pair<bytes, ByteFormat> stringToBytes(std::string _string);

	ExpectationParser(std::istream& _stream): m_scanner(_stream) {}

	std::vector<FunctionCall> parseFunctionCalls();

private:
	class Scanner {
	public:
		Scanner(std::istream& _stream): m_stream(_stream) {}

		char current() const { return *m_char; }
		bool eol() const { return m_char == m_line.end(); }
		std::string::iterator& position() { return m_char; }
		std::string::iterator endPosition() { return m_line.end(); }

		void advance() { ++m_char; }
		bool advanceLine()
		{
			auto& line = getline(m_stream, m_line);
			m_char = m_line.begin();
			return line ? true : false;
		}

	private:
		std::string m_line;
		std::string::iterator m_char;
		std::istream& m_stream;
	};

	std::string parseFunctionCallSignature();
	FunctionCallArgs parseFunctionCallArgument();
	FunctionCallExpectations parseFunctionCallExpectations();
	u256 parseFunctionCallValue();

	void skipWhitespaces();
	void expectCharacter(char const _char);
	bool advanceLine();

	Scanner m_scanner;
};

}
}
}
