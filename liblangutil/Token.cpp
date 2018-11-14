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

#include <map>
#include <liblangutil/Token.h>
#include <boost/range/iterator_range.hpp>

using namespace std;

namespace dev
{
namespace solidity
{

void ElementaryTypeNameToken::assertDetails(Token _baseType, unsigned const& _first, unsigned const& _second)
{
	solAssert(TokenTraits::isElementaryTypeName(_baseType), "Expected elementary type name: " + string(TokenTraits::toString(_baseType)));
	if (_baseType == Token::BytesM)
	{
		solAssert(_second == 0, "There should not be a second size argument to type bytesM.");
		solAssert(_first <= 32, "No elementary type bytes" + to_string(_first) + ".");
	}
	else if (_baseType == Token::UIntM || _baseType == Token::IntM)
	{
		solAssert(_second == 0, "There should not be a second size argument to type " + string(TokenTraits::toString(_baseType)) + ".");
		solAssert(
			_first <= 256 && _first % 8 == 0,
			"No elementary type " + string(TokenTraits::toString(_baseType)) + to_string(_first) + "."
		);
	}
	else if (_baseType == Token::UFixedMxN || _baseType == Token::FixedMxN)
	{
		solAssert(
			_first >= 8 && _first <= 256 && _first % 8 == 0 && _second <= 80,
			"No elementary type " + string(TokenTraits::toString(_baseType)) + to_string(_first) + "x" + to_string(_second) + "."
		);
	}
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

#define T(name, string, precedence) precedence,
int precedence(Token tok)
{
	int8_t const static precs[TokenTraits::count()] =
	{
		TOKEN_LIST(T, T)
	};
	return precs[static_cast<size_t>(tok)];
}
#undef T

int parseSize(string::const_iterator _begin, string::const_iterator _end)
{
	try
	{
		unsigned int m = boost::lexical_cast<int>(boost::make_iterator_range(_begin, _end));
		return m;
	}
	catch(boost::bad_lexical_cast const&)
	{
		return -1;
	}
}

static Token keywordByName(string const& _name)
{
	// The following macros are used inside TOKEN_LIST and cause non-keyword tokens to be ignored
	// and keywords to be put inside the keywords variable.
#define KEYWORD(name, string, precedence) {string, Token::name},
#define TOKEN(name, string, precedence)
	static const map<string, Token> keywords({TOKEN_LIST(TOKEN, KEYWORD)});
#undef KEYWORD
#undef TOKEN
	auto it = keywords.find(_name);
	return it == keywords.end() ? Token::Identifier : it->second;
}

tuple<Token, unsigned int, unsigned int> fromIdentifierOrKeyword(string const& _literal)
{
	auto positionM = find_if(_literal.begin(), _literal.end(), ::isdigit);
	if (positionM != _literal.end())
	{
		string baseType(_literal.begin(), positionM);
		auto positionX = find_if_not(positionM, _literal.end(), ::isdigit);
		int m = parseSize(positionM, positionX);
		Token keyword = keywordByName(baseType);
		if (keyword == Token::Bytes)
		{
			if (0 < m && m <= 32 && positionX == _literal.end())
				return make_tuple(Token::BytesM, m, 0);
		}
		else if (keyword == Token::UInt || keyword == Token::Int)
		{
			if (0 < m && m <= 256 && m % 8 == 0 && positionX == _literal.end())
			{
				if (keyword == Token::UInt)
					return make_tuple(Token::UIntM, m, 0);
				else
					return make_tuple(Token::IntM, m, 0);
			}
		}
		else if (keyword == Token::UFixed || keyword == Token::Fixed)
		{
			if (
				positionM < positionX &&
				positionX < _literal.end() &&
				*positionX == 'x' &&
				all_of(positionX + 1, _literal.end(), ::isdigit)
			) {
				int n = parseSize(positionX + 1, _literal.end());
				if (
					8 <= m && m <= 256 && m % 8 == 0 &&
					0 <= n && n <= 80
				) {
					if (keyword == Token::UFixed)
						return make_tuple(Token::UFixedMxN, m, n);
					else
						return make_tuple(Token::FixedMxN, m, n);
				}
			}
		}
		return make_tuple(Token::Identifier, 0, 0);
	}

	return make_tuple(keywordByName(_literal), 0, 0);
}

}
}
}
