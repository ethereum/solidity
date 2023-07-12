// Copyright 2006-2012, the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials provided
//      with the distribution.
//    * Neither the name of Google Inc. nor the names of its
//      contributors may be used to endorse or promote products derived
//      from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Modifications as part of solidity under the following license:
//
// solidity is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// solidity is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with solidity.  If not, see <http://www.gnu.org/licenses/>.

#include <liblangutil/Exceptions.h>
#include <liblangutil/Token.h>
#include <libsolutil/StringUtils.h>

#include <map>

namespace solidity::langutil
{

Token TokenTraits::AssignmentToBinaryOp(Token op)
{
	solAssert(isAssignmentOp(op) && op != Token::Assign, "");
	return static_cast<Token>(static_cast<int>(op) + (static_cast<int>(Token::BitOr) - static_cast<int>(Token::AssignBitOr)));
}

std::string ElementaryTypeNameToken::toString(bool const& tokenValue) const
{
	std::string name = TokenTraits::toString(m_token);
	if (tokenValue || (firstNumber() == 0 && secondNumber() == 0))
		return name;
	solAssert(name.size() >= 3, "Token name size should be greater than 3. Should not reach here.");
	if (m_token == Token::FixedMxN || m_token == Token::UFixedMxN)
		return name.substr(0, name.size() - 3) + std::to_string(m_firstNumber) + "x" + std::to_string(m_secondNumber);
	else
		return name.substr(0, name.size() - 1) + std::to_string(m_firstNumber);
}

void ElementaryTypeNameToken::assertDetails(Token _baseType, unsigned const& _first, unsigned const& _second)
{
	solAssert(TokenTraits::isElementaryTypeName(_baseType), "Expected elementary type name: " + std::string(TokenTraits::toString(_baseType)));
	if (_baseType == Token::BytesM)
	{
		solAssert(_second == 0, "There should not be a second size argument to type bytesM.");
		solAssert(_first <= 32, "No elementary type bytes" + std::to_string(_first) + ".");
	}
	else if (_baseType == Token::UIntM || _baseType == Token::IntM)
	{
		solAssert(_second == 0, "There should not be a second size argument to type " + std::string(TokenTraits::toString(_baseType)) + ".");
		solAssert(
			_first <= 256 && _first % 8 == 0,
			"No elementary type " + std::string(TokenTraits::toString(_baseType)) + std::to_string(_first) + "."
		);
	}
	else if (_baseType == Token::UFixedMxN || _baseType == Token::FixedMxN)
	{
		solAssert(
			_first >= 8 && _first <= 256 && _first % 8 == 0 && _second <= 80,
			"No elementary type " + std::string(TokenTraits::toString(_baseType)) + std::to_string(_first) + "x" + std::to_string(_second) + "."
		);
	}
	else
		solAssert(_first == 0 && _second == 0, "Unexpected size arguments");

	m_token = _baseType;
	m_firstNumber = _first;
	m_secondNumber = _second;
}

namespace TokenTraits
{
char const* toString(Token tok)
{
	switch (tok)
	{
#define T(name, string, precedence) case Token::name: return string;
		TOKEN_LIST(T, T)
#undef T
		default: // Token::NUM_TOKENS:
			return "";
	}
}

char const* name(Token tok)
{
#define T(name, string, precedence) #name,
	static char const* const names[TokenTraits::count()] = { TOKEN_LIST(T, T) };
#undef T

	solAssert(static_cast<size_t>(tok) < TokenTraits::count(), "");
	return names[static_cast<size_t>(tok)];
}

std::string friendlyName(Token tok)
{
	char const* ret = toString(tok);
	if (ret)
		return std::string(ret);

	ret = name(tok);
	solAssert(ret != nullptr, "");
	return std::string(ret);
}


static Token keywordByName(std::string const& _name)
{
	// The following macros are used inside TOKEN_LIST and cause non-keyword tokens to be ignored
	// and keywords to be put inside the keywords variable.
#define KEYWORD(name, string, precedence) {string, Token::name},
#define TOKEN(name, string, precedence)
	static std::map<std::string, Token> const keywords({TOKEN_LIST(TOKEN, KEYWORD)});
#undef KEYWORD
#undef TOKEN
	auto it = keywords.find(_name);
	return it == keywords.end() ? Token::Identifier : it->second;
}

bool isYulKeyword(std::string const& _literal)
{
	return _literal == "leave" || isYulKeyword(keywordByName(_literal));
}

std::tuple<Token, unsigned int, unsigned int> fromIdentifierOrKeyword(std::string const& _literal)
{
	// Used for `bytesM`, `uintM`, `intM`, `fixedMxN`, `ufixedMxN`.
	// M/N must be shortest representation. M can never be 0. N can be zero.
	auto parseSize = [](std::string::const_iterator _begin, std::string::const_iterator _end) -> int
	{
		// No number.
		if (distance(_begin, _end) == 0)
			return -1;

		// Disallow leading zero.
		if (distance(_begin, _end) > 1 && *_begin == '0')
			return -1;

		int ret = 0;
		for (auto it = _begin; it != _end; it++)
		{
			if (*it < '0' || *it > '9')
				return -1;
			//  Overflow check. The largest acceptable value is 256 in the callers.
			if (ret >= 256)
				return -1;
			ret *= 10;
			ret += *it - '0';
		}
		return ret;
	};

	auto positionM = find_if(_literal.begin(), _literal.end(), util::isDigit);
	if (positionM != _literal.end())
	{
		std::string baseType(_literal.begin(), positionM);
		auto positionX = find_if_not(positionM, _literal.end(), util::isDigit);
		int m = parseSize(positionM, positionX);
		Token keyword = keywordByName(baseType);
		if (keyword == Token::Bytes)
		{
			if (0 < m && m <= 32 && positionX == _literal.end())
				return std::make_tuple(Token::BytesM, m, 0);
		}
		else if (keyword == Token::UInt || keyword == Token::Int)
		{
			if (0 < m && m <= 256 && m % 8 == 0 && positionX == _literal.end())
			{
				if (keyword == Token::UInt)
					return std::make_tuple(Token::UIntM, m, 0);
				else
					return std::make_tuple(Token::IntM, m, 0);
			}
		}
		else if (keyword == Token::UFixed || keyword == Token::Fixed)
		{
			if (
				positionM < positionX &&
				positionX < _literal.end() &&
				*positionX == 'x' &&
				all_of(positionX + 1, _literal.end(), util::isDigit)
			) {
				int n = parseSize(positionX + 1, _literal.end());
				if (
					8 <= m && m <= 256 && m % 8 == 0 &&
					0 <= n && n <= 80
				) {
					if (keyword == Token::UFixed)
						return std::make_tuple(Token::UFixedMxN, m, n);
					else
						return std::make_tuple(Token::FixedMxN, m, n);
				}
			}
		}
		return std::make_tuple(Token::Identifier, 0, 0);
	}

	return std::make_tuple(keywordByName(_literal), 0, 0);
}

}
}
