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

#pragma once

#include <libdevcore/Common.h>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/parsing/UndefMacros.h>

namespace dev
{
namespace solidity
{

// TOKEN_LIST takes a list of 3 macros M, all of which satisfy the
// same signature M(name, string, precedence), where name is the
// symbolic token name, string is the corresponding syntactic symbol
// (or NULL, for literals), and precedence is the precedence (or 0).
// The parameters are invoked for token categories as follows:
//
//   T: Non-keyword tokens
//   K: Keyword tokens

// IGNORE_TOKEN is a convenience macro that can be supplied as
// an argument (at any position) for a TOKEN_LIST call. It does
// nothing with tokens belonging to the respective category.

#define IGNORE_TOKEN(name, string, precedence)

#define TOKEN_LIST(T, K)												\
	/* End of source indicator. */										\
	T(EOS, "EOS", 0)													\
																		\
	/* Punctuators (ECMA-262, section 7.7, page 15). */				\
	T(LParen, "(", 0)                                                   \
	T(RParen, ")", 0)                                                   \
	T(LBrack, "[", 0)                                                   \
	T(RBrack, "]", 0)                                                   \
	T(LBrace, "{", 0)                                                   \
	T(RBrace, "}", 0)                                                   \
	T(Colon, ":", 0)                                                    \
	T(Semicolon, ";", 0)                                                \
	T(Period, ".", 0)                                                   \
	T(Conditional, "?", 3)                                              \
	T(Arrow, "=>", 0)                                                   \
	\
	/* Assignment operators. */										\
	/* IsAssignmentOp() relies on this block of enum values being */	\
	/* contiguous and sorted in the same order!*/						\
	T(Assign, "=", 2)                                                   \
	/* The following have to be in exactly the same order as the simple binary operators*/ \
	T(AssignBitOr, "|=", 2)                                           \
	T(AssignBitXor, "^=", 2)                                          \
	T(AssignBitAnd, "&=", 2)                                          \
	T(AssignShl, "<<=", 2)                                            \
	T(AssignSar, ">>=", 2)                                            \
	T(AssignShr, ">>>=", 2)                                           \
	T(AssignAdd, "+=", 2)                                             \
	T(AssignSub, "-=", 2)                                             \
	T(AssignMul, "*=", 2)                                             \
	T(AssignDiv, "/=", 2)                                             \
	T(AssignMod, "%=", 2)                                             \
	\
	/* Binary operators sorted by precedence. */                       \
	/* IsBinaryOp() relies on this block of enum values */             \
	/* being contiguous and sorted in the same order! */               \
	T(Comma, ",", 1)                                                   \
	T(Or, "||", 4)                                                     \
	T(And, "&&", 5)                                                    \
	T(BitOr, "|", 8)                                                   \
	T(BitXor, "^", 9)                                                  \
	T(BitAnd, "&", 10)                                                 \
	T(SHL, "<<", 11)                                                   \
	T(SAR, ">>", 11)                                                   \
	T(SHR, ">>>", 11)                                                  \
	T(Add, "+", 12)                                                    \
	T(Sub, "-", 12)                                                    \
	T(Mul, "*", 13)                                                    \
	T(Div, "/", 13)                                                    \
	T(Mod, "%", 13)                                                    \
	T(Exp, "**", 14)                                                   \
	\
	/* Compare operators sorted by precedence. */                      \
	/* IsCompareOp() relies on this block of enum values */            \
	/* being contiguous and sorted in the same order! */               \
	T(Equal, "==", 6)                                                  \
	T(NotEqual, "!=", 6)                                               \
	T(LessThan, "<", 7)                                                \
	T(GreaterThan, ">", 7)                                             \
	T(LessThanOrEqual, "<=", 7)                                        \
	T(GreaterThanOrEqual, ">=", 7)                                     \
	\
	/* Unary operators. */                                             \
	/* IsUnaryOp() relies on this block of enum values */              \
	/* being contiguous and sorted in the same order! */               \
	T(Not, "!", 0)                                                     \
	T(BitNot, "~", 0)                                                  \
	T(Inc, "++", 0)                                                    \
	T(Dec, "--", 0)                                                    \
	K(Delete, "delete", 0)                                             \
	\
	/* Keywords */                                                     \
	K(Anonymous, "anonymous", 0)                                       \
	K(As, "as", 0)                                                     \
	K(Assembly, "assembly", 0)                                         \
	K(Break, "break", 0)                                               \
	K(Constant, "constant", 0)                                         \
	K(Continue, "continue", 0)                                         \
	K(Contract, "contract", 0)                                         \
	K(Do, "do", 0)                                                     \
	K(Else, "else", 0)                                                 \
	K(Enum, "enum", 0)                                                 \
	K(Emit, "emit", 0)                                                 \
	K(Event, "event", 0)                                               \
	K(External, "external", 0)                                         \
	K(For, "for", 0)                                                   \
	K(Function, "function", 0)                                         \
	K(Hex, "hex", 0)                                                   \
	K(If, "if", 0)                                                     \
	K(Indexed, "indexed", 0)                                           \
	K(Interface, "interface", 0)                                       \
	K(Internal, "internal", 0)                                         \
	K(Import, "import", 0)                                             \
	K(Is, "is", 0)                                                     \
	K(Library, "library", 0)                                           \
	K(Mapping, "mapping", 0)                                           \
	K(Memory, "memory", 0)                                             \
	K(Modifier, "modifier", 0)                                         \
	K(New, "new", 0)                                                   \
	K(Payable, "payable", 0)                                           \
	K(Public, "public", 0)                                             \
	K(Pragma, "pragma", 0)                                             \
	K(Private, "private", 0)                                           \
	K(Pure, "pure", 0)                                                 \
	K(Return, "return", 0)                                             \
	K(Returns, "returns", 0)                                           \
	K(Storage, "storage", 0)                                           \
	K(CallData, "calldata", 0)                                         \
	K(Struct, "struct", 0)                                             \
	K(Throw, "throw", 0)                                               \
	K(Using, "using", 0)                                               \
	K(Var, "var", 0)                                                   \
	K(View, "view", 0)                                                 \
	K(While, "while", 0)                                               \
	\
	/* Ether subdenominations */                                       \
	K(SubWei, "wei", 0)                                                \
	K(SubSzabo, "szabo", 0)                                            \
	K(SubFinney, "finney", 0)                                          \
	K(SubEther, "ether", 0)                                            \
	K(SubSecond, "seconds", 0)                                         \
	K(SubMinute, "minutes", 0)                                         \
	K(SubHour, "hours", 0)                                             \
	K(SubDay, "days", 0)                                               \
	K(SubWeek, "weeks", 0)                                             \
	K(SubYear, "years", 0)                                             \
	/* type keywords*/                                                 \
	K(Int, "int", 0)                                                   \
	K(UInt, "uint", 0)                                                 \
	K(Bytes, "bytes", 0)                                               \
	K(Byte, "byte", 0)                                                 \
	K(String, "string", 0)                                             \
	K(Address, "address", 0)                                           \
	K(Bool, "bool", 0)                                                 \
	K(Fixed, "fixed", 0)                                               \
	K(UFixed, "ufixed", 0)                                             \
	T(IntM, "intM", 0)                                                 \
	T(UIntM, "uintM", 0)                                               \
	T(BytesM, "bytesM", 0)                                             \
	T(FixedMxN, "fixedMxN", 0)                                         \
	T(UFixedMxN, "ufixedMxN", 0)                                       \
	T(TypesEnd, NULL, 0) /* used as type enum end marker */            \
	\
	/* Literals */                                                     \
	K(TrueLiteral, "true", 0)                                          \
	K(FalseLiteral, "false", 0)                                        \
	T(Number, NULL, 0)                                                 \
	T(StringLiteral, NULL, 0)                                          \
	T(CommentLiteral, NULL, 0)                                         \
	\
	/* Identifiers (not keywords or future reserved words). */         \
	T(Identifier, NULL, 0)                                             \
	\
	/* Keywords reserved for future use. */                            \
	K(Abstract, "abstract", 0)                                         \
	K(After, "after", 0)                                               \
	K(Alias, "alias", 0)                                               \
	K(Apply, "apply", 0)                                               \
	K(Auto, "auto", 0)                                                 \
	K(Case, "case", 0)                                                 \
	K(Catch, "catch", 0)                                               \
	K(CopyOf, "copyof", 0)                                             \
	K(Default, "default", 0)                                           \
	K(Define, "define", 0)                                             \
	K(Final, "final", 0)                                               \
	K(Immutable, "immutable", 0)                                       \
	K(Implements, "implements", 0)                                     \
	K(In, "in", 0)                                                     \
	K(Inline, "inline", 0)                                             \
	K(Let, "let", 0)                                                   \
	K(Macro, "macro", 0)                                               \
	K(Match, "match", 0)                                               \
	K(Mutable, "mutable", 0)                                           \
	K(NullLiteral, "null", 0)                                          \
	K(Of, "of", 0)                                                     \
	K(Override, "override", 0)                                         \
	K(Partial, "partial", 0)                                           \
	K(Promise, "promise", 0)                                           \
	K(Reference, "reference", 0)                                       \
	K(Relocatable, "relocatable", 0)                                   \
	K(Sealed, "sealed", 0)                                             \
	K(Sizeof, "sizeof", 0)                                             \
	K(Static, "static", 0)                                             \
	K(Supports, "supports", 0)                                         \
	K(Switch, "switch", 0)                                             \
	K(Try, "try", 0)                                                   \
	K(Type, "type", 0)                                                 \
	K(Typedef, "typedef", 0)                                           \
	K(TypeOf, "typeof", 0)                                             \
	K(Unchecked, "unchecked", 0)                                       \
	\
	/* Illegal token - not able to scan. */                            \
	T(Illegal, "ILLEGAL", 0)                                           \
	\
	/* Scanner-internal use only. */                                   \
	T(Whitespace, NULL, 0)


class Token
{
public:
	// All token values.
	// attention! msvc issue:
	// http://stackoverflow.com/questions/9567868/compile-errors-after-adding-v8-to-my-project-c2143-c2059
	// @todo: avoid TOKEN_LIST macro
#define T(name, string, precedence) name,
	enum Value
	{
		TOKEN_LIST(T, T)
		NUM_TOKENS
	};
#undef T

	// @returns a string corresponding to the C++ token name
	// (e.g. "LT" for the token LT).
	static char const* name(Value tok)
	{
		solAssert(tok < NUM_TOKENS, "");
		return m_name[tok];
	}

	// Predicates
	static bool isElementaryTypeName(Value tok) { return Int <= tok && tok < TypesEnd; }
	static bool isAssignmentOp(Value tok) { return Assign <= tok && tok <= AssignMod; }
	static bool isBinaryOp(Value op) { return Comma <= op && op <= Exp; }
	static bool isCommutativeOp(Value op) { return op == BitOr || op == BitXor || op == BitAnd ||
				op == Add || op == Mul || op == Equal || op == NotEqual; }
	static bool isArithmeticOp(Value op) { return Add <= op && op <= Exp; }
	static bool isCompareOp(Value op) { return Equal <= op && op <= GreaterThanOrEqual; }

	static Value AssignmentToBinaryOp(Value op)
	{
		solAssert(isAssignmentOp(op) && op != Assign, "");
		return Value(op + (BitOr - AssignBitOr));
	}

	static bool isBitOp(Value op) { return (BitOr <= op && op <= BitAnd) || op == BitNot; }
	static bool isBooleanOp(Value op) { return (Or <= op && op <= And) || op == Not; }
	static bool isUnaryOp(Value op) { return (Not <= op && op <= Delete) || op == Add || op == Sub; }
	static bool isCountOp(Value op) { return op == Inc || op == Dec; }
	static bool isShiftOp(Value op) { return (SHL <= op) && (op <= SHR); }
	static bool isVisibilitySpecifier(Value op) { return isVariableVisibilitySpecifier(op) || op == External; }
	static bool isVariableVisibilitySpecifier(Value op) { return op == Public || op == Private || op == Internal; }
	static bool isLocationSpecifier(Value op) { return op == Memory || op == Storage || op == CallData; }
	static bool isStateMutabilitySpecifier(Value op) { return op == Pure || op == Constant || op == View || op == Payable; }
	static bool isEtherSubdenomination(Value op) { return op == SubWei || op == SubSzabo || op == SubFinney || op == SubEther; }
	static bool isTimeSubdenomination(Value op) { return op == SubSecond || op == SubMinute || op == SubHour || op == SubDay || op == SubWeek || op == SubYear; }
	static bool isReservedKeyword(Value op) { return (Abstract <= op && op <= Unchecked); }

	// @returns a string corresponding to the JS token string
	// (.e., "<" for the token LT) or NULL if the token doesn't
	// have a (unique) string (e.g. an IDENTIFIER).
	static char const* toString(Value tok)
	{
		solAssert(tok < NUM_TOKENS, "");
		return m_string[tok];
	}

	static std::string friendlyName(Value tok)
	{
		char const* ret = toString(tok);
		if (ret == nullptr)
		{
			ret = name(tok);
			solAssert(ret != nullptr, "");
		}
		return std::string(ret);
	}

	// @returns the precedence > 0 for binary and compare
	// operators; returns 0 otherwise.
	static int precedence(Value tok)
	{
		solAssert(tok < NUM_TOKENS, "");
		return m_precedence[tok];
	}

	static std::tuple<Token::Value, unsigned int, unsigned int> fromIdentifierOrKeyword(std::string const& _literal);

private:
	// @returns -1 on error (invalid digit or number too large)
	static int parseSize(std::string::const_iterator _begin, std::string::const_iterator _end);
	// @returns the keyword with name @a _name or Token::Identifier of no such keyword exists.
	static Token::Value keywordByName(std::string const& _name);
	static char const* const m_name[NUM_TOKENS];
	static char const* const m_string[NUM_TOKENS];
	static int8_t const m_precedence[NUM_TOKENS];
	static char const m_tokenType[NUM_TOKENS];
};

class ElementaryTypeNameToken
{
public:
	ElementaryTypeNameToken(Token::Value _token, unsigned const& _firstNumber, unsigned const& _secondNumber)
	{
		assertDetails(_token, _firstNumber, _secondNumber);
	}

	unsigned int firstNumber() const { return m_firstNumber; }
	unsigned int secondNumber() const { return m_secondNumber; }
	Token::Value token() const { return m_token; }
	///if tokValue is set to true, then returns the actual token type name, otherwise, returns full type
	std::string toString(bool const& tokenValue = false) const
	{
		std::string name = Token::toString(m_token);
		if (tokenValue || (firstNumber() == 0 && secondNumber() == 0))
			return name;
		solAssert(name.size() >= 3, "Token name size should be greater than 3. Should not reach here.");
		if (m_token == Token::FixedMxN || m_token == Token::UFixedMxN)
			return name.substr(0, name.size() - 3) + std::to_string(m_firstNumber) + "x" + std::to_string(m_secondNumber);
		else
			return name.substr(0, name.size() - 1) + std::to_string(m_firstNumber);
	}

private:
	Token::Value m_token;
	unsigned int m_firstNumber;
	unsigned int m_secondNumber;
	/// throws if type is not properly sized
	void assertDetails(Token::Value _baseType, unsigned const& _first, unsigned const& _second);
};

}
}
