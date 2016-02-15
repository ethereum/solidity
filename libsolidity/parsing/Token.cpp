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
// Modifications as part of cpp-ethereum under the following license:
//
// cpp-ethereum is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// cpp-ethereum is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.

#include <map>
#include <libsolidity/parsing/Token.h>

using namespace std;

namespace dev
{
namespace solidity
{

void ElementaryTypeNameToken::parseDetails(Token::Value _baseType, unsigned const& _first, unsigned const& _second)
{
	solAssert(Token::isElementaryTypeName(_baseType), "");
	string baseType = Token::toString(_baseType);
	if (_first == 0 && _second == 0)
	{
		m_name = baseType;
		m_firstNumber = _first;
		m_secondNumber = _second;
	}

	if (baseType == "bytesM")
	{
		solAssert(_second == 0, "There should not be a second size argument to type bytesM.");
		for (unsigned m = 1; m <= 32; m++)
			if (m == _first)
			{
				m_name = baseType.substr(0, baseType.size()-1) + to_string(_first);
				m_firstNumber = _first;
				m_secondNumber = _second;
			}
	}
	else if (baseType == "uintM" || baseType == "intM")
	{
		solAssert(_second == 0, "There should not be a second size argument to type " + baseType + ".");
		for (unsigned m = 8; m <= 256; m+=8)
			if (m == _first)
			{
				m_name = baseType.substr(0, baseType.size()-1) + to_string(_first);
				m_firstNumber = _first;
				m_secondNumber = _second;
			}
	}
	else if (baseType == "ufixedMxN" || baseType == "fixedMxN")
	{
		for (unsigned m = 0; m <= 256; m+=8)
			for (unsigned n = 8; m + n <= 256; n+=8)
				if (m == _first && n == _second)
				{
					m_name = baseType.substr(0, baseType.size()-3) + 
							to_string(_first) + 
							"x" +
							to_string(_second);
					m_firstNumber = _first;
					m_secondNumber = _second;
				}
	}
	
	if (m_name.empty())
		BOOST_THROW_EXCEPTION(Error(Error::Type::TypeError) << 
			errinfo_comment(
				"Cannot create elementary type name token out of type " + 
				baseType +
				" and size(s) " +
				to_string(_first) +
				" and " +
				to_string(_second)
			)
		);	
}

#define T(name, string, precedence) #name,
char const* const Token::m_name[NUM_TOKENS] =
{
	TOKEN_LIST(T, T)
};
#undef T


#define T(name, string, precedence) string,
char const* const Token::m_string[NUM_TOKENS] =
{
	TOKEN_LIST(T, T)
};
#undef T


#define T(name, string, precedence) precedence,
int8_t const Token::m_precedence[NUM_TOKENS] =
{
	TOKEN_LIST(T, T)
};
#undef T


#define KT(a, b, c) 'T',
#define KK(a, b, c) 'K',
char const Token::m_tokenType[] =
{
	TOKEN_LIST(KT, KK)
};
unsigned Token::extractM(string _literal)
{
	try
	{
		unsigned short m = stoi(_literal.substr(_literal.find_first_of("0123456789")));
		return m;
	}
	catch(out_of_range& e)
	{
		return 0;
	}
}
pair<unsigned, unsigned> Token::extractMxN(string _literal)
{
	try
	{
		unsigned short m = stoi(_literal.substr(0, _literal.find_last_of("x") - 1));
		unsigned short n = stoi(_literal.substr(_literal.find_last_of("x") + 1));
		return make_pair(m, n);
	}
	catch (out_of_range& e)
	{
		return make_pair(0, 0);
	}
}
tuple<Token::Value, unsigned short, unsigned short> Token::fromIdentifierOrKeyword(string const& _literal)
{
	if (_literal.find_first_of("0123456789") != string::npos)
	{
		string baseType = _literal.substr(0, _literal.find_first_of("0123456789"));
		if (baseType == "bytes")
		{
			unsigned short m = extractM(_literal);
			if (0 < m && m <= 32)
				return make_tuple(Token::BytesM, m, 0);
			return make_tuple(Token::Identifier, 0, 0);
		}
		else if (baseType == "uint" || baseType == "int")
		{
			unsigned short m = extractM(_literal);			
			if (0 < m && m <= 256 && m % 8 == 0)
			{
				if (baseType == "uint")
					return make_tuple(Token::UIntM, m, 0);
				else
					return make_tuple(Token::IntM, m, 0);
			}
			return make_tuple(Token::Identifier, 0, 0);
		}
		else if (baseType == "ufixed" || baseType == "fixed")
		{
			unsigned short m;
			unsigned short n;
			tie(m, n) = extractMxN(_literal);
			if (0 < n + m && n + m <= 256 && ((n % 8 == 0) && (m % 8 == 0)))
			{
				if (baseType == "ufixed")
					return make_tuple(Token::UFixedMxN, m, n);
				else
					return make_tuple(Token::FixedMxN, m, n);
			}
			return make_tuple(Token::Identifier, 0, 0);
		}
	}
	// The following macros are used inside TOKEN_LIST and cause non-keyword tokens to be ignored
	// and keywords to be put inside the keywords variable.
#define KEYWORD(name, string, precedence) {string, Token::name},
#define TOKEN(name, string, precedence)
	static const map<string, Token::Value> keywords({TOKEN_LIST(TOKEN, KEYWORD)});
#undef KEYWORD
#undef TOKEN
	auto it = keywords.find(_literal);
	return it == keywords.end() ? make_tuple(Token::Identifier, 0, 0) : make_tuple(it->second, 0, 0);
}

#undef KT
#undef KK

}
}
